"""
WIZ LoopTester - QSPI / BUS 단방향 대역폭 + 패턴 무결성 테스터
=============================================================

WIZnet ioModule (1차 대상: WIZ630MJ) 생산품 불량 판정용.

동작 개요 (자세한 규약은 PROTOCOL.md)
------------------------------------
- PC가 **TCP 서버**로 동작 (Bind IP / Port 화면에서 수정)
- 장비(WIZ630MJ)가 클라이언트로 접속 → 한 연결 = 한 측정
- 접속 직후 장비가 헤더 1줄 전송: [QSPI][TX] / [QSPI][RX] / [BUS][TX] / [BUS][RX]
- **TX**: 장비 → PC 단방향 송신, PC가 받기만 하며 Mbps + 패턴 무결성 측정
- **RX**: PC → 장비 단방향 송신, 장비가 받아서 [RESULT]로 회신 → PC가 Mbps 계산
- 합격 = (Mbps ≥ Min Mbps) ∧ (패턴 오류 0)

에코/왕복 없음 (RTT 병목 제거). 무결성은 약속된 숫자 패턴(byte i == '0'+(i%10))으로 검사.
"""

import os
import re
import sys
import csv
import time
import queue
import socket
import threading
import tkinter as tk
from tkinter import ttk
from tkinter.scrolledtext import ScrolledText
from datetime import datetime
from pathlib import Path

# ---------------------------------------------------------------------------
# 설정 기본값
# ---------------------------------------------------------------------------
DEFAULT_BIND_IP = "192.168.11.42"   # 장비가 접속하는 PC IP (전체 수신은 0.0.0.0)
DEFAULT_PORT = 5000
DEFAULT_MIN_MBPS = 2.0       # 인터페이스별 최소 대역폭 임계값
DEFAULT_RX_DURATION = 1.0    # RX 측정 시 PC가 송신하는 시간 (초). 불량판정엔 1초면 충분

INTERFACES = ["qspi", "bus"]
DIRECTIONS = ["tx", "rx"]
IFACE_ACCENT = {"qspi": "#3498db", "bus": "#e67e22"}

# 한 인터페이스의 측정 완료 = TX + RX 둘 다.  (모드당 단방향이면 여기만 바꾸면 됨)
QSPI_SET = {("qspi", "tx"), ("qspi", "rx")}
BUS_SET = {("bus", "tx"), ("bus", "rx")}

# 약속된 패턴 (PROTOCOL.md §1): byte[i] = '0' + (i % 10)
PATTERN_UNIT = b"0123456789"
RECV_CHUNK = 65536
_PAT_BIG = PATTERN_UNIT * (RECV_CHUNK // 10 + 2)     # 슬라이스용 (>= RECV_CHUNK+10)
PATTERN_BLOCK = PATTERN_UNIT * 6553                  # 65530B, 10의 배수 → 이어붙여도 정렬 유지


def app_dir() -> Path:
    """실행 파일(또는 스크립트)이 있는 폴더. PyInstaller onefile(.exe)에서도
    임시 추출 폴더가 아니라 실제 EXE 위치를 반환 → 결과 CSV가 EXE 옆에 저장됨.
    """
    if getattr(sys, "frozen", False):
        return Path(sys.executable).resolve().parent
    return Path(__file__).resolve().parent


def count_mismatches(data: bytes, offset: int) -> int:
    """data가 약속 패턴과 다른 바이트 수. offset = 스트림상의 전역 시작 위치.

    두 경로 모두 C 속도(파이썬 per-byte 루프 없음):
    - 일치: bytes == 비교(memcmp)로 즉시 0.
    - 불일치: XOR 후 0이 아닌 바이트 수 = len - (XOR결과의 0바이트 개수, bytes.count(0)).
    이렇게 해야 손상/정렬오류 스트림에서도 recv가 느려지지 않아 TX가 안 막힘.
    """
    phase = offset % 10
    exp = _PAT_BIG[phase:phase + len(data)]
    if data == exp:
        return 0
    x = int.from_bytes(data, "big") ^ int.from_bytes(exp, "big")
    return len(data) - x.to_bytes(len(data), "big").count(0)


def parse_header(line: str):
    """'[QSPI][TX]' -> ('qspi', 'tx').  방향 생략 시 ('qspi', None)."""
    iface, direction = None, None
    for tag in re.findall(r"\[([^\]]+)\]", line):
        t = tag.strip().lower()
        if t in INTERFACES:
            iface = t
        elif t in DIRECTIONS:
            direction = t
    return iface, direction


def parse_result(line: str) -> dict:
    """'[RESULT]bytes=123,ms=1000,err=0' -> {'bytes':123,'ms':1000,'err':0}."""
    return {k.lower(): int(v) for k, v in re.findall(r"(\w+)\s*=\s*(-?\d+)", line)}


class WizLoopTester:
    COLOR_BG = "#1a1a1a"
    COLOR_CARD = "#2d2d2d"
    COLOR_PRIMARY = "#0078d4"
    COLOR_SUCCESS = "#16a085"
    COLOR_ERROR = "#e74c3c"
    COLOR_WARNING = "#f39c12"
    COLOR_IDLE = "#7f8c8d"
    COLOR_TEXT = "#ffffff"
    COLOR_TEXT_SECONDARY = "#bdc3c7"

    def __init__(self, root: tk.Tk):
        self.root = root
        self.root.title("WIZ LoopTester  -  QSPI / BUS 단방향 대역폭 테스트")
        self.root.configure(bg=self.COLOR_BG)
        self.root.geometry("1000x740")
        self.root.minsize(860, 620)

        # 서버 상태
        self.server_sock = None
        self.client_sock = None
        self.server_thread = None
        self.stop_event = threading.Event()
        self.running = False

        # START 시 스냅샷되는 설정 (워커 스레드에서 안전하게 읽기 위함)
        self.cfg_min_mbps = DEFAULT_MIN_MBPS
        self.cfg_rx_duration = DEFAULT_RX_DURATION

        # UI 비동기 갱신 큐
        self.log_q = queue.Queue()
        self.ui_q = queue.Queue()

        # 결과 셀 위젯: cells[iface] = {"tx": label, "rx": label, "note": label}
        self.cells = {}
        # 세션 판정: (iface, direction) -> ok.  4개 모이면 전체 PASS/FAIL
        self.session = {}
        self.popup = None             # 작업자 안내 팝업 (Toplevel) 또는 None
        self.cycle_complete = False   # 4개 측정 완료 → 다음 연결은 새 DUT로 간주

        # 결과 CSV (DUT 한 대 완료마다 1행 append). 엑셀 한글 위해 UTF-8 BOM.
        self.csv_path = app_dir() / "test_results.csv"
        self.round_no = 0
        self._init_round_no()

        self._setup_styles()
        self._build_widgets()
        self._pump_queues()

    # ------------------------------------------------------------------ 스타일
    def _setup_styles(self):
        style = ttk.Style()
        style.theme_use("clam")
        style.configure("Card.TFrame", background=self.COLOR_CARD, relief="flat", borderwidth=1)
        style.configure("Primary.TButton", background=self.COLOR_PRIMARY,
                        foreground="white", borderwidth=0, focuscolor="none")
        style.map("Primary.TButton", background=[("active", "#106ebe")])
        style.configure("Stop.TButton", background=self.COLOR_ERROR,
                        foreground="white", borderwidth=0, focuscolor="none")
        style.map("Stop.TButton", background=[("active", "#c0392b")])

    # --------------------------------------------------------------- 위젯 구성
    def _build_widgets(self):
        main = tk.Frame(self.root, bg=self.COLOR_BG)
        main.pack(fill=tk.BOTH, expand=True, padx=12, pady=10)

        tk.Label(main, text="WIZ LoopTester", font=("Segoe UI", 15, "bold"),
                 fg=self.COLOR_TEXT, bg=self.COLOR_BG).pack(anchor=tk.W)
        tk.Label(main, text="QSPI / BUS 단방향 대역폭 + 패턴 무결성  ·  TCP Server",
                 font=("Segoe UI", 9), fg=self.COLOR_TEXT_SECONDARY,
                 bg=self.COLOR_BG).pack(anchor=tk.W, pady=(0, 10))

        # --- 서버 설정 ---
        sc = ttk.Frame(main, style="Card.TFrame", padding=10)
        sc.pack(fill=tk.X, pady=(0, 8))
        tk.Label(sc, text="Server Config", font=("Segoe UI", 10, "bold"),
                 fg=self.COLOR_TEXT, bg=self.COLOR_CARD).grid(row=0, column=0, columnspan=8, sticky=tk.W, pady=(0, 6))

        tk.Label(sc, text="Bind IP", font=("Segoe UI", 9), fg=self.COLOR_TEXT_SECONDARY,
                 bg=self.COLOR_CARD).grid(row=1, column=0, padx=(0, 6), sticky=tk.W)
        self.ip_var = tk.StringVar(value=DEFAULT_BIND_IP)
        self.ip_entry = ttk.Entry(sc, width=16, textvariable=self.ip_var)
        self.ip_entry.grid(row=1, column=1, padx=(0, 14))

        tk.Label(sc, text="Port", font=("Segoe UI", 9), fg=self.COLOR_TEXT_SECONDARY,
                 bg=self.COLOR_CARD).grid(row=1, column=2, padx=(0, 6), sticky=tk.W)
        self.port_var = tk.IntVar(value=DEFAULT_PORT)
        self.port_entry = ttk.Entry(sc, width=8, textvariable=self.port_var)
        self.port_entry.grid(row=1, column=3, padx=(0, 14))

        self.btn_start = ttk.Button(sc, text="START SERVER", style="Primary.TButton", command=self.on_start)
        self.btn_start.grid(row=1, column=4, padx=(0, 8), ipadx=8, ipady=2)
        self.btn_stop = ttk.Button(sc, text="STOP", style="Stop.TButton", command=self.on_stop, state=tk.DISABLED)
        self.btn_stop.grid(row=1, column=5, ipadx=8, ipady=2)

        conn = tk.Frame(sc, bg=self.COLOR_CARD)
        conn.grid(row=1, column=6, padx=(20, 0))
        self.conn_canvas = tk.Canvas(conn, width=18, height=18, highlightthickness=0, bg=self.COLOR_CARD)
        self.conn_canvas.pack(side=tk.LEFT)
        self.conn_canvas.create_oval(2, 2, 16, 16, fill=self.COLOR_IDLE,
                                     outline=self.COLOR_TEXT_SECONDARY, width=1, tags="led")
        self.conn_label = tk.Label(conn, text="STOPPED", font=("Segoe UI", 9, "bold"),
                                   fg=self.COLOR_TEXT_SECONDARY, bg=self.COLOR_CARD)
        self.conn_label.pack(side=tk.LEFT, padx=(6, 0))

        # --- 테스트 설정 ---
        tc = ttk.Frame(main, style="Card.TFrame", padding=10)
        tc.pack(fill=tk.X, pady=(0, 8))
        tk.Label(tc, text="Test Config", font=("Segoe UI", 10, "bold"),
                 fg=self.COLOR_TEXT, bg=self.COLOR_CARD).grid(row=0, column=0, columnspan=6, sticky=tk.W, pady=(0, 6))

        tk.Label(tc, text="Min Mbps (임계)", font=("Segoe UI", 9), fg=self.COLOR_TEXT_SECONDARY,
                 bg=self.COLOR_CARD).grid(row=1, column=0, padx=(0, 6), sticky=tk.W)
        self.min_var = tk.DoubleVar(value=DEFAULT_MIN_MBPS)
        self.min_entry = ttk.Entry(tc, width=8, textvariable=self.min_var)
        self.min_entry.grid(row=1, column=1, padx=(0, 18))

        tk.Label(tc, text="RX Duration(s)", font=("Segoe UI", 9), fg=self.COLOR_TEXT_SECONDARY,
                 bg=self.COLOR_CARD).grid(row=1, column=2, padx=(0, 6), sticky=tk.W)
        self.rxdur_var = tk.DoubleVar(value=DEFAULT_RX_DURATION)
        self.rxdur_entry = ttk.Entry(tc, width=8, textvariable=self.rxdur_var)
        self.rxdur_entry.grid(row=1, column=3, padx=(0, 18))

        tk.Label(tc, text="(측정은 장비 접속 시 자동: 헤더 [QSPI]/[BUS] + [TX]/[RX])",
                 font=("Segoe UI", 8), fg=self.COLOR_TEXT_SECONDARY,
                 bg=self.COLOR_CARD).grid(row=1, column=4, padx=(0, 14), sticky=tk.W)

        self.btn_reset = ttk.Button(tc, text="결과 초기화 (NEW DUT)", command=self.on_reset)
        self.btn_reset.grid(row=1, column=5, ipadx=4, ipady=2)

        self.btn_open_csv = ttk.Button(tc, text="결과 CSV 열기", command=self.on_open_csv)
        self.btn_open_csv.grid(row=1, column=6, padx=(8, 0), ipadx=4, ipady=2)

        tk.Label(tc, text="참고: 장비 이더넷 100M 캡 — QSPI는 ~90Mbps에서 네트워크에 막힐 수 있고, "
                          "BUS는 호스트 인터페이스가 느려 더 낮게 나옵니다.",
                 font=("Segoe UI", 8), fg=self.COLOR_TEXT_SECONDARY,
                 bg=self.COLOR_CARD).grid(row=2, column=0, columnspan=7, sticky=tk.W, pady=(6, 0))

        # --- 통계 (CSV 집계: Today / Total) ---
        stc = ttk.Frame(main, style="Card.TFrame", padding=10)
        stc.pack(fill=tk.X, pady=(0, 8))
        tk.Label(stc, text="Statistics", font=("Segoe UI", 10, "bold"),
                 fg=self.COLOR_TEXT, bg=self.COLOR_CARD).grid(row=0, column=0, columnspan=6, sticky=tk.W, pady=(0, 6))

        self.stats_scope = tk.StringVar(value="today")
        for col, (val, txt) in enumerate((("today", "Today"), ("total", "Total"))):
            tk.Radiobutton(stc, text=txt, value=val, variable=self.stats_scope,
                           command=self._update_stats, font=("Segoe UI", 9, "bold"),
                           fg=self.COLOR_TEXT, bg=self.COLOR_CARD, selectcolor=self.COLOR_PRIMARY,
                           activebackground=self.COLOR_CARD, activeforeground=self.COLOR_TEXT,
                           indicatoron=True).grid(row=1, column=col, padx=(0, 10), sticky=tk.W)

        self.work_label = tk.Label(stc, text="작업량: 0 개", font=("Segoe UI", 14, "bold"),
                                   fg=self.COLOR_TEXT, bg=self.COLOR_CARD)
        self.work_label.grid(row=1, column=2, padx=(24, 24), sticky=tk.W)
        self.defect_label = tk.Label(stc, text="불량률: 0.00%", font=("Segoe UI", 14, "bold"),
                                     fg=self.COLOR_TEXT_SECONDARY, bg=self.COLOR_CARD)
        self.defect_label.grid(row=1, column=3, sticky=tk.W)

        # --- 인터페이스별 결과 (TX / RX) ---
        rc = ttk.Frame(main, style="Card.TFrame", padding=10)
        rc.pack(fill=tk.X, pady=(0, 8))
        tk.Label(rc, text="Interface Result", font=("Segoe UI", 10, "bold"),
                 fg=self.COLOR_TEXT, bg=self.COLOR_CARD).grid(row=0, column=0, columnspan=3, sticky=tk.W, pady=(0, 8))
        self.overall_label = tk.Label(rc, text="전체: 0/4", font=("Segoe UI", 13, "bold"),
                                      fg=self.COLOR_TEXT_SECONDARY, bg=self.COLOR_CARD)
        self.overall_label.grid(row=0, column=3, sticky=tk.E, pady=(0, 8))

        # 헤더 행
        heads = ["", "TX (장비→PC)", "RX (PC→장비)", "무결성"]
        for col, h in enumerate(heads):
            tk.Label(rc, text=h, font=("Segoe UI", 9, "bold"), fg=self.COLOR_TEXT_SECONDARY,
                     bg=self.COLOR_CARD).grid(row=1, column=col, padx=10, pady=(0, 4), sticky=tk.W)

        for i, name in enumerate(INTERFACES):
            row = i + 2
            tk.Label(rc, text=f"  {name.upper()}  ", font=("Segoe UI", 11, "bold"),
                     fg="white", bg=IFACE_ACCENT.get(name, self.COLOR_PRIMARY)).grid(
                         row=row, column=0, padx=10, pady=4, sticky=tk.W)
            tx = tk.Label(rc, text="대기", font=("Consolas", 11, "bold"), width=14, anchor=tk.W,
                          fg=self.COLOR_TEXT_SECONDARY, bg=self.COLOR_CARD)
            tx.grid(row=row, column=1, padx=10, sticky=tk.W)
            rx = tk.Label(rc, text="대기", font=("Consolas", 11, "bold"), width=14, anchor=tk.W,
                          fg=self.COLOR_TEXT_SECONDARY, bg=self.COLOR_CARD)
            rx.grid(row=row, column=2, padx=10, sticky=tk.W)
            note = tk.Label(rc, text="-", font=("Segoe UI", 9), anchor=tk.W,
                            fg=self.COLOR_TEXT_SECONDARY, bg=self.COLOR_CARD)
            note.grid(row=row, column=3, padx=10, sticky=tk.W)
            self.cells[name] = {"tx": tx, "rx": rx, "note": note}

        # --- 로그 ---
        lc = ttk.Frame(main, style="Card.TFrame", padding=10)
        lc.pack(fill=tk.BOTH, expand=True)
        tk.Label(lc, text="Log", font=("Segoe UI", 10, "bold"),
                 fg=self.COLOR_TEXT, bg=self.COLOR_CARD).pack(anchor=tk.W, pady=(0, 6))
        self.log_box = ScrolledText(lc, height=14, state=tk.DISABLED, wrap=tk.WORD,
                                    font=("Consolas", 9), bg="#1e1e1e", fg="#ffffff",
                                    insertbackground="#ffffff")
        self.log_box.pack(fill=tk.BOTH, expand=True)

        self._update_stats()   # 시작 시 기존 CSV 집계 표시

    # --------------------------------------------------------------- 로깅/큐
    def _log(self, msg: str):
        ts = datetime.now().strftime("%H:%M:%S.%f")[:-3]
        self.log_q.put(f"{ts}  {msg}\n")

    def _pump_queues(self):
        try:
            try:
                while True:
                    line = self.log_q.get_nowait()
                    self.log_box.config(state=tk.NORMAL)
                    self.log_box.insert(tk.END, line)
                    self.log_box.see(tk.END)
                    self.log_box.config(state=tk.DISABLED)
            except queue.Empty:
                pass
            while True:
                try:
                    action, data = self.ui_q.get_nowait()
                except queue.Empty:
                    break
                try:
                    action(data)
                except Exception as e:
                    self._log(f"[UI][ERROR] {type(e).__name__}: {e}")
        finally:
            self.root.after(80, self._pump_queues)

    def _ui(self, action, data=None):
        self.ui_q.put((action, data))

    # --------------------------------------------------------------- 표시 헬퍼
    def _set_conn(self, state: str):
        cmap = {"stopped": self.COLOR_IDLE, "listening": self.COLOR_PRIMARY,
                "connected": self.COLOR_SUCCESS, "error": self.COLOR_ERROR}
        tmap = {"stopped": "STOPPED", "listening": "LISTENING",
                "connected": "CONNECTED", "error": "ERROR"}
        color = cmap.get(state, self.COLOR_IDLE)
        self.conn_canvas.itemconfig("led", fill=color)
        self.conn_label.config(text=tmap.get(state, state.upper()), fg=color)

    def _set_cell(self, iface: str, direction: str, text: str, color: str):
        self.cells[iface][direction].config(text=text, fg=color)

    def _set_note(self, iface: str, text: str, color: str):
        self.cells[iface]["note"].config(text=text, fg=color)

    def _reset_results(self):
        for name in INTERFACES:
            self._set_cell(name, "tx", "대기", self.COLOR_TEXT_SECONDARY)
            self._set_cell(name, "rx", "대기", self.COLOR_TEXT_SECONDARY)
            self._set_note(name, "-", self.COLOR_TEXT_SECONDARY)

    def _update_overall(self):
        """세션 4개(QSPI/BUS × TX/RX) 결과로 전체 PASS/FAIL 표시."""
        n = len(self.session)
        passed = sum(1 for v in self.session.values() if v["ok"])
        if n == 0:
            self.overall_label.config(text="전체: 0/4", fg=self.COLOR_TEXT_SECONDARY)
        elif n < 4:
            self.overall_label.config(text=f"전체: {n}/4 진행", fg=self.COLOR_WARNING)
        elif passed == 4:
            self.overall_label.config(text="전체: PASS (4/4)", fg=self.COLOR_SUCCESS)
        else:
            self.overall_label.config(text=f"전체: FAIL ({passed}/4)", fg=self.COLOR_ERROR)

    # --------------------------------------------------------------- 작업 안내 팝업
    def _evaluate_phase(self):
        """한 연결 측정이 끝날 때마다 호출 — 단계 전환/완료 시 작업자 팝업 표시.

        QSPI(TX+RX) 완료 → 'BUS로 전환' 팝업.  BUS까지 완료 → '교체' 팝업(정상/불량).
        팝업은 다음 연결이 들어오면 _serve에서 자동 제거된다.
        """
        done = set(self.session.keys())
        qspi_done = QSPI_SET <= done
        bus_started = bool(done & BUS_SET)
        bus_done = BUS_SET <= done

        if qspi_done and bus_done:
            verdict = all(v["ok"] for v in self.session.values())
            self.cycle_complete = True
            self._save_result_row(verdict)
            self._log(f"[SESSION] 전체 측정 완료 → {'정상' if verdict else '불량'}")
            self._ui(lambda _, v=verdict: self._show_complete_popup(v))
        elif qspi_done and not bus_started:
            self._log("[SESSION] QSPI 완료 → BUS 전환 안내 팝업")
            self._ui(lambda _: self._show_switch_popup())

    def _show_switch_popup(self):
        self._show_popup(
            "QSPI TEST 완료!\n\n모듈을 BUS 모드로\n퓨즈를 연결해 주세요\n\n"
            "(BUS로 연결되면 자동으로 사라집니다)",
            self.COLOR_PRIMARY)

    def _show_complete_popup(self, verdict: bool):
        result = "정상" if verdict else "불량"
        color = self.COLOR_SUCCESS if verdict else self.COLOR_ERROR
        self._show_popup(
            f"TEST 완료!\n\n결과 = {result}!\n\n모듈을 교체해 주세요\n\n"
            "(모듈이 연결되면 자동으로 사라집니다)",
            color)

    def _show_popup(self, text: str, color: str):
        self._dismiss_popup()
        top = tk.Toplevel(self.root)
        top.title("WIZ LoopTester")
        top.configure(bg=color)
        top.resizable(False, False)
        try:
            top.attributes("-topmost", True)
        except tk.TclError:
            pass
        w, h = 560, 320
        self.root.update_idletasks()
        x = max(self.root.winfo_rootx() + (self.root.winfo_width() - w) // 2, 0)
        y = max(self.root.winfo_rooty() + (self.root.winfo_height() - h) // 2, 0)
        top.geometry(f"{w}x{h}+{x}+{y}")
        tk.Label(top, text=text, font=("Segoe UI", 17, "bold"), fg="white", bg=color,
                 justify=tk.CENTER, wraplength=w - 60).pack(expand=True, fill=tk.BOTH, padx=24, pady=24)
        self.popup = top

    def _dismiss_popup(self):
        if self.popup is not None:
            try:
                self.popup.destroy()
            except tk.TclError:
                pass
            self.popup = None

    # --------------------------------------------------------------- 결과 CSV
    def _init_round_no(self):
        """기존 CSV가 있으면 마지막 회차를 이어받는다 (재시작해도 번호 연속)."""
        if not self.csv_path.exists():
            return
        try:
            with self.csv_path.open("r", encoding="utf-8-sig", newline="") as f:
                rows = [r for r in csv.reader(f) if r]
            data = rows[1:] if rows else []   # 헤더 제외
            if data:
                try:
                    self.round_no = int(data[-1][1])
                except (ValueError, IndexError):
                    self.round_no = len(data)
        except OSError:
            pass

    def _save_result_row(self, verdict: bool):
        """DUT 한 대 완료 시 결과 1행을 CSV에 append (시간·회차·4대역폭·최종결과)."""
        def mbps_of(key):
            rec = self.session.get(key)
            return f"{rec['mbps']:.2f}" if rec else ""

        self.round_no += 1
        ts = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        row = [ts, self.round_no,
               mbps_of(("qspi", "tx")), mbps_of(("qspi", "rx")),
               mbps_of(("bus", "tx")), mbps_of(("bus", "rx")),
               "정상" if verdict else "불량"]
        try:
            new_file = not self.csv_path.exists()
            with self.csv_path.open("a", encoding="utf-8-sig", newline="") as f:
                w = csv.writer(f)
                if new_file:
                    w.writerow(["시간", "회차", "QSPI_TX(Mbps)", "QSPI_RX(Mbps)",
                                "BUS_TX(Mbps)", "BUS_RX(Mbps)", "최종결과"])
                w.writerow(row)
            self._log(f"[FILE] 결과 저장 (회차 {self.round_no}): {self.csv_path.name}")
        except OSError as e:
            # 엑셀에서 파일을 열어두면 잠겨서 실패할 수 있음 → 다음 행은 정상 기록됨
            self._log(f"[FILE][ERROR] 저장 실패(파일이 열려있나요?): {e}")
        self._ui(lambda _: self._update_stats())

    def _update_stats(self):
        """결과 CSV를 읽어 작업량/불량률 표시 (Today=오늘 날짜 / Total=전체)."""
        scope = self.stats_scope.get()
        today = datetime.now().strftime("%Y-%m-%d")
        total, fail = 0, 0
        try:
            if self.csv_path.exists():
                with self.csv_path.open("r", encoding="utf-8-sig", newline="") as f:
                    rows = list(csv.reader(f))
                for r in rows[1:]:                 # 헤더 제외
                    if len(r) < 7:
                        continue
                    if scope == "today" and not r[0].startswith(today):
                        continue
                    total += 1
                    if r[6].strip() == "불량":
                        fail += 1
        except OSError as e:
            self._log(f"[FILE][ERROR] 통계 읽기 실패: {e}")
            return

        rate = (fail / total * 100) if total else 0.0
        self.work_label.config(text=f"작업량: {total} 개")
        self.defect_label.config(
            text=f"불량률: {rate:.2f}%  (불량 {fail})",
            fg=self.COLOR_ERROR if fail else self.COLOR_SUCCESS)

    # --------------------------------------------------------------- 버튼 핸들러
    def on_start(self):
        try:
            ip = self.ip_var.get().strip()
            port = int(self.port_var.get())
            self.cfg_min_mbps = float(self.min_var.get())
            self.cfg_rx_duration = max(0.2, float(self.rxdur_var.get()))
        except (tk.TclError, ValueError):
            self._log("[ERROR] 설정값(Port/Min Mbps/Duration)이 올바르지 않습니다.")
            return

        self.stop_event.clear()
        self.running = True
        self.cycle_complete = False
        self._dismiss_popup()
        self._toggle_inputs(running=True)
        self.session = {}
        self._reset_results()
        self._update_overall()

        self.server_thread = threading.Thread(target=self._serve, args=(ip, port), daemon=True)
        self.server_thread.start()

    def on_reset(self):
        """다음 DUT 측정을 위해 결과/판정 초기화 (서버는 그대로)."""
        self.session = {}
        self.cycle_complete = False
        self._dismiss_popup()
        self._reset_results()
        self._update_overall()
        self._log("[USER] 결과 초기화 (NEW DUT)")

    def on_open_csv(self):
        """결과 CSV 파일을 기본 프로그램(엑셀)으로 연다."""
        try:
            if self.csv_path.exists():
                os.startfile(str(self.csv_path))   # Windows 전용
            else:
                self._log("[FILE] 아직 저장된 결과가 없습니다.")
        except Exception as e:
            self._log(f"[FILE][ERROR] CSV 열기 실패: {e}")

    def on_stop(self):
        self._log("[USER] 서버 중지 요청")
        self.running = False
        self.stop_event.set()
        self._close_sockets()
        self._dismiss_popup()
        self._ui(lambda _: self._set_conn("stopped"))
        self._ui(lambda _: self._toggle_inputs(running=False))

    def _toggle_inputs(self, running: bool):
        self.btn_start.config(state=tk.DISABLED if running else tk.NORMAL)
        self.btn_stop.config(state=tk.NORMAL if running else tk.DISABLED)
        st = tk.DISABLED if running else tk.NORMAL
        for w in (self.ip_entry, self.port_entry, self.min_entry, self.rxdur_entry):
            w.config(state=st)

    # --------------------------------------------------------------- 서버 로직
    def _serve(self, ip: str, port: int):
        try:
            self.server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.server_sock.bind((ip, port))
            self.server_sock.listen(1)
            self.server_sock.settimeout(1.0)
            self._log(f"[SERVER] Listening on {ip}:{port}  (Min={self.cfg_min_mbps:.1f}Mbps, "
                      f"RX={self.cfg_rx_duration:.1f}s)")
            self._ui(lambda _: self._set_conn("listening"))
        except OSError as e:
            self._log(f"[ERROR] 서버 시작 실패: {e}")
            self._ui(lambda _: self._set_conn("error"))
            self._ui(lambda _: self._toggle_inputs(running=False))
            self.running = False
            return

        while not self.stop_event.is_set():
            try:
                client, addr = self.server_sock.accept()
            except socket.timeout:
                continue
            except OSError:
                break

            self.client_sock = client
            # 새 연결 → 안내 팝업 제거. 직전 사이클이 완료였으면 새 DUT로 초기화.
            self._ui(lambda _: self._dismiss_popup())
            if self.cycle_complete:
                self.session = {}
                self.cycle_complete = False
                self._log("[SESSION] 새 모듈 측정 시작")
                self._ui(lambda _: (self._reset_results(), self._update_overall()))
            self._log(f"[SERVER] 장비 연결됨: {addr[0]}:{addr[1]}")
            self._ui(lambda _: self._set_conn("connected"))
            try:
                self._handle_connection(client)
            except Exception as e:
                self._log(f"[ERROR] 연결 처리 예외: {type(e).__name__} - {e}")
            self._close_client()
            self._evaluate_phase()
            if not self.stop_event.is_set():
                self._ui(lambda _: self._set_conn("listening"))

        self._log("[SERVER] 수락 루프 종료")

    def _handle_connection(self, client: socket.socket):
        # 1) 헤더 한 줄 읽기
        for opt in ((socket.IPPROTO_TCP, socket.TCP_NODELAY, 1),):
            try:
                client.setsockopt(*opt)
            except OSError:
                pass
        try:
            client.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 1 << 20)
        except OSError:
            pass

        header, leftover = self._read_line(client, timeout=5.0)
        if header is None:
            self._log("[WARN] 헤더 수신 실패 (장비가 즉시 끊음)")
            return

        iface, direction = parse_header(header)
        if iface not in INTERFACES:
            self._log(f"[WARN] 알 수 없는 헤더: {header!r} — 연결 종료")
            return
        direction = direction or "tx"
        self._log(f"[IDENT] {iface.upper()} / {direction.upper()}  (header={header!r})")
        self._ui(lambda _: self._set_cell(iface, direction, "측정 중...", self.COLOR_PRIMARY))
        self._ui(lambda _: self._set_note(iface, "측정 중...", self.COLOR_WARNING))

        # 2) 방향별 측정
        if direction == "tx":
            self._measure_tx(client, iface, leftover)
        else:
            self._measure_rx(client, iface)

    def _read_line(self, client: socket.socket, timeout: float):
        """헤더/결과 한 줄 읽기. (line, leftover_bytes) 반환. 실패 시 (None, b'')."""
        client.settimeout(timeout)
        buf = b""
        while b"\n" not in buf:
            try:
                chunk = client.recv(256)
            except socket.timeout:
                return None, b""
            except OSError:
                return None, b""
            if not chunk:
                return (None, b"") if not buf else (buf.decode("utf-8", "replace").strip(), b"")
            buf += chunk
            if len(buf) > 4096:
                break
        line, _, rest = buf.partition(b"\n")
        return line.decode("utf-8", "replace").strip(), rest

    # --------------------------------------------------------------- TX 측정
    def _measure_tx(self, client: socket.socket, iface: str, leftover: bytes):
        """장비 → PC 단방향 수신 측정. (종료 = 장비 close/EOF. 4초 무수신이면 중단)"""
        client.settimeout(4.0)
        total = len(leftover)
        errors = count_mismatches(leftover, 0) if leftover else 0
        offset = len(leftover)
        t0 = time.perf_counter() if leftover else None

        while not self.stop_event.is_set():
            try:
                data = client.recv(RECV_CHUNK)
            except socket.timeout:
                self._log("[TX][WARN] 4초 무수신 — 장비가 스트림 후 소켓을 닫았는지 확인(EOF 필요)")
                break
            except OSError:
                break
            if not data:
                break  # 장비가 close → 정상 종료
            if t0 is None:
                t0 = time.perf_counter()
            errors += count_mismatches(data, offset)
            offset += len(data)
            total += len(data)

        dt = max(time.perf_counter() - (t0 or time.perf_counter()), 1e-9)
        mbps = total * 8 / dt / 1_000_000
        self._report(iface, "tx", total, dt, mbps, errors)

    # --------------------------------------------------------------- RX 측정
    def _measure_rx(self, client: socket.socket, iface: str):
        """PC → 장비 단방향 송신. 장비가 [RESULT]로 회신."""
        duration = self.cfg_rx_duration
        client.settimeout(5.0)
        sent = 0
        t0 = time.perf_counter()
        try:
            while (time.perf_counter() - t0) < duration and not self.stop_event.is_set():
                client.sendall(PATTERN_BLOCK)
                sent += len(PATTERN_BLOCK)
        except socket.timeout:
            self._log("[RX][WARN] 송신 타임아웃 — 장비가 받지 못하는 중")
        except OSError as e:
            self._log(f"[RX][ERROR] 송신 중단: {e}")
        send_dt = max(time.perf_counter() - t0, 1e-9)

        # 데이터 끝 신호 (EOF)
        try:
            client.shutdown(socket.SHUT_WR)
        except OSError:
            pass

        # 장비 결과 회신 수신 ([RESULT]). 늦으면 2초 후 PC 추정으로 폴백
        line, _ = self._read_line(client, timeout=2.0)
        dev = parse_result(line) if line else {}
        if line:
            self._log(f"[RX] 장비 회신: {line!r}")

        if dev.get("ms", 0) > 0 and "bytes" in dev:
            total = dev["bytes"]
            dt = dev["ms"] / 1000.0
            errors = dev.get("err", -1)
            mbps = total * 8 / dt / 1_000_000
            src = "장비측정"
        else:
            total = sent
            dt = send_dt
            errors = -1  # 미확인
            mbps = total * 8 / dt / 1_000_000
            src = "PC추정"
        self._report(iface, "rx", total, dt, mbps, errors, src)

    # --------------------------------------------------------------- 결과 반영
    def _report(self, iface, direction, total, dt, mbps, errors, src=None):
        min_mbps = self.cfg_min_mbps
        integrity_ok = (errors == 0)
        integrity_unknown = (errors < 0)
        speed_ok = (total > 0 and mbps >= min_mbps)
        # 합격 = 속도 임계 이상 AND 무결성 확인됨(err==0). 미확인은 불합격 처리.
        ok = speed_ok and integrity_ok

        srctxt = f" [{src}]" if src else ""
        self._log(f"[{iface.upper()}][{direction.upper()}]{srctxt} "
                  f"{total}B / {dt:.3f}s / {mbps:.2f} Mbps / "
                  f"오류 {'미확인' if integrity_unknown else errors}  -> {'정상' if ok else '불량'}")

        if total == 0:
            speed_txt = "수신 0"
        else:
            speed_txt = f"{mbps:.2f} Mbps"
        if integrity_unknown:
            color = self.COLOR_WARNING
            note_txt, note_color = "무결성 미확인", self.COLOR_WARNING
        elif integrity_ok:
            color = self.COLOR_SUCCESS if speed_ok else self.COLOR_ERROR
            note_txt, note_color = "패턴 OK", self.COLOR_SUCCESS
        else:
            color = self.COLOR_ERROR
            note_txt, note_color = f"손상 {errors}B", self.COLOR_ERROR

        self.session[(iface, direction)] = {"ok": ok, "mbps": mbps, "err": errors}
        self._ui(lambda _: self._set_cell(iface, direction, speed_txt, color))
        self._ui(lambda _: self._set_note(iface, note_txt, note_color))
        self._ui(lambda _: self._update_overall())

    # --------------------------------------------------------------- 소켓 정리
    def _close_client(self):
        if self.client_sock:
            try:
                self.client_sock.close()
            except OSError:
                pass
        self.client_sock = None

    def _close_sockets(self):
        self._close_client()
        if self.server_sock:
            try:
                self.server_sock.close()
            except OSError:
                pass
        self.server_sock = None


if __name__ == "__main__":
    root = tk.Tk()
    app = WizLoopTester(root)

    def on_closing():
        app.running = False
        app.stop_event.set()
        app._close_sockets()
        root.destroy()

    root.protocol("WM_DELETE_WINDOW", on_closing)
    root.mainloop()
