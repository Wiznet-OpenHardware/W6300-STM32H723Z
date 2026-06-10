import threading
import socket
import time
import queue
import tkinter as tk
from tkinter import ttk
from tkinter.scrolledtext import ScrolledText
from datetime import datetime
from pathlib import Path

# 테스트 파라미터
DURATION_SEC = 1.0  # 패킷 통신 테스트 시간 절반으로 축소
PAYLOAD = b''.join(bytes(str(i % 10), 'utf-8') for i in range(1024))
INITIAL_DELAY = 2.0  # START 버튼 후 대기 시간
SOCKET_COUNT = 3

# 성능 임계값 설정
QSPI_MIN_MBPS = 2.0    # QSPI 모드 최소 성능 임계값


class ModernLoopbackUI:
    # 모던한 컬러 팔레트
    COLOR_BG = "#1a1a1a"          # 다크 배경
    COLOR_CARD = "#2d2d2d"        # 카드 배경
    COLOR_PRIMARY = "#0078d4"     # 파란색 (Microsoft 스타일)
    COLOR_SUCCESS = "#16a085"     # 초록색
    COLOR_ERROR = "#e74c3c"       # 빨간색
    COLOR_WARNING = "#f39c12"     # 주황색
    COLOR_IDLE = "#7f8c8d"        # 회색
    COLOR_TEXT = "#ffffff"        # 텍스트
    COLOR_TEXT_SECONDARY = "#bdc3c7"  # 보조 텍스트

    def __init__(self, root: tk.Tk):
        self.root = root
        self.root.title("WIZnet ioModule Multi-SPI Loopback Tester")
        self.root.configure(bg=self.COLOR_BG)
        
        # 창 크기 설정 (더 컴팩트하게)
        self.root.geometry("1100x750")
        self.root.minsize(900, 600)

        self.stop_event = threading.Event()
        self.tester_thread = None
        self.log_q = queue.Queue()
        self.test_results = [
            {
                "qspi": {"success": False, "loops": 0, "accuracy": 0.0, "mbps": 0.0},
            }
            for _ in range(SOCKET_COUNT)
        ]
        self.socket_logs = [[] for _ in range(SOCKET_COUNT)]
        self.error_logs = []  # 통합 에러 로그
        self.packet_analysis = [[] for _ in range(SOCKET_COUNT)]  # 패킷 분석 정보

        self.ip_vars = []
        self.port_vars = []
        self.enable_vars = []
        self.status_indicators = []
        self.ping_indicators = []  # 핑 상태 LED
        self.ping_states = [{"reachable": False, "last_ping": 0}] * SOCKET_COUNT  # 핑 상태 저장
        self.result_tree = None
        
        # 핑 모니터링용 변수
        self.ping_thread = None
        self.ping_stop_event = threading.Event()

        self._setup_styles()
        self._build_widgets()
        self._start_ping_monitor()  # 핑 모니터링 시작
        self._update_log()

    def _setup_styles(self):
        """모던한 스타일 설정"""
        style = ttk.Style()
        style.theme_use('clam')
        
        # 프레임 스타일
        style.configure("Card.TFrame", 
                       background=self.COLOR_CARD, 
                       relief="flat", 
                       borderwidth=1)
        
        # 버튼 스타일
        style.configure("Primary.TButton",
                       background=self.COLOR_PRIMARY,
                       foreground="white",
                       borderwidth=0,
                       focuscolor="none")
        style.map("Primary.TButton",
                 background=[('active', '#106ebe')])
        
        style.configure("Stop.TButton",
                       background=self.COLOR_ERROR,
                       foreground="white",
                       borderwidth=0,
                       focuscolor="none")
        style.map("Stop.TButton",
                 background=[('active', '#c0392b')])
        
        # Treeview 헤더 스타일 커스터마이징
        style.configure("Custom.Treeview.Heading",
                       background=self.COLOR_CARD,
                       foreground=self.COLOR_TEXT,
                       font=("Segoe UI", 10, "bold"),
                       borderwidth=1,
                       relief="raised")
        
        # Treeview 본체 스타일
        style.configure("Custom.Treeview",
                       background=self.COLOR_CARD,
                       foreground=self.COLOR_TEXT,
                       fieldbackground=self.COLOR_CARD,
                       borderwidth=0)
        
        # 선택 시 색상
        style.map("Custom.Treeview",
                 background=[('selected', self.COLOR_PRIMARY)],
                 foreground=[('selected', 'white')])

    def _build_widgets(self):
        # 메인 컨테이너 (패딩 축소)
        main_frame = tk.Frame(self.root, bg=self.COLOR_BG)
        main_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=8)

        # 헤더 (크기 축소)
        header_frame = tk.Frame(main_frame, bg=self.COLOR_BG)
        header_frame.pack(fill=tk.X, pady=(0, 5))
        
        title_label = tk.Label(header_frame, 
                              text="WIZnet ioModule Multi-SPI Loopback Tester",
                              font=("Segoe UI", 13, "bold"),
                              fg=self.COLOR_TEXT,
                              bg=self.COLOR_BG)
        title_label.pack(side=tk.LEFT)

        # 상단 영역: 설정, 컨트롤, 결과를 가로로 배치
        top_container = tk.Frame(main_frame, bg=self.COLOR_BG)
        top_container.pack(fill=tk.X, pady=(0, 8))

        # 왼쪽: 소켓 설정 (축소)
        config_frame = ttk.Frame(top_container, style="Card.TFrame", padding=8)
        config_frame.pack(side=tk.LEFT, fill=tk.Y, padx=(0, 5))

        config_title = tk.Label(config_frame, 
                               text="Socket Config",
                               font=("Segoe UI", 10, "bold"),
                               fg=self.COLOR_TEXT,
                               bg=self.COLOR_CARD)
        config_title.grid(row=0, column=0, columnspan=6, sticky=tk.W, pady=(0, 5))

        # 헤더 라벨
        headers = ["Socket", "IP Address", "Port", "Enable", "Ping", "Status"]
        for col, header in enumerate(headers):
            if col == 5:  # Status 컬럼
                continue
            label = tk.Label(config_frame, 
                           text=header,
                           font=("Segoe UI", 10, "bold"),
                           fg=self.COLOR_TEXT_SECONDARY,
                           bg=self.COLOR_CARD)
            label.grid(row=1, column=col, padx=8, pady=5, sticky=tk.W)

        # 입력 위젯들을 저장할 리스트
        self.input_widgets = []

        # 소켓 설정 입력
        socket_ips = ["192.168.11.10", "192.168.11.11", "192.168.11.12"]  # 소켓 4번째(192.168.11.13) 제거
        
        for i in range(SOCKET_COUNT):
            row = i + 2
            row_widgets = []
            
            # 소켓 번호
            socket_label = tk.Label(config_frame,
                                   text=f"Socket {i+1}",
                                   font=("Segoe UI", 10),
                                   fg=self.COLOR_TEXT,
                                   bg=self.COLOR_CARD)
            socket_label.grid(row=row, column=0, padx=8, pady=5)

            # IP 입력
            ip_var = tk.StringVar(value=socket_ips[i])
            ip_var.trace_add('write', lambda *args, idx=i: self._on_ip_change(idx))  # IP 변경 감지
            ip_entry = ttk.Entry(config_frame, width=15, textvariable=ip_var, font=("Segoe UI", 9))
            ip_entry.grid(row=row, column=1, padx=8, pady=5)
            self.ip_vars.append(ip_var)
            row_widgets.append(ip_entry)

            # 포트 입력
            port_var = tk.IntVar(value=5000 + i)
            port_entry = ttk.Entry(config_frame, width=8, textvariable=port_var, font=("Segoe UI", 9))
            port_entry.grid(row=row, column=2, padx=8, pady=5)
            self.port_vars.append(port_var)
            row_widgets.append(port_entry)

            # 활성화 체크박스 (커스텀 스타일)
            enable_var = tk.BooleanVar(value=True)
            enable_frame = tk.Frame(config_frame, bg=self.COLOR_CARD)
            enable_frame.grid(row=row, column=3, padx=8, pady=5)
            
            # 커스텀 체크박스 버튼 (크기 축소)
            enable_btn = tk.Button(enable_frame, 
                                  text="✓", 
                                  font=("Segoe UI", 10, "bold"),
                                  width=2, height=1,
                                  bg=self.COLOR_SUCCESS,
                                  fg="white",
                                  relief=tk.FLAT,
                                  command=lambda idx=i: self._toggle_enable(idx))
            enable_btn.pack()
            
            self.enable_vars.append(enable_var)
            row_widgets.append(enable_btn)

            # 핑 상태 LED (새로 추가)
            ping_frame = tk.Frame(config_frame, bg=self.COLOR_CARD)
            ping_frame.grid(row=row, column=4, padx=8, pady=5)
            
            ping_canvas = tk.Canvas(ping_frame, width=20, height=20, 
                                   highlightthickness=0, bg=self.COLOR_CARD)
            ping_canvas.pack()
            ping_canvas.create_oval(2, 2, 18, 18, fill=self.COLOR_IDLE, 
                                   outline=self.COLOR_TEXT_SECONDARY, width=1, tags="ping_led")
            
            ping_label = tk.Label(ping_frame, text="PING", 
                                font=("Segoe UI", 7, "bold"), 
                                fg=self.COLOR_TEXT_SECONDARY,
                                bg=self.COLOR_CARD)
            ping_label.pack()
            
            self.ping_indicators.append(ping_canvas)

            # 상태 표시기 (컬럼 위치 조정)
            status_frame = tk.Frame(config_frame, bg=self.COLOR_CARD)
            status_frame.grid(row=row, column=5, padx=8, pady=5, sticky=tk.W)
            
            indicators = {}
            status_labels = {}
            indicator_info = [("QSPI", 0), ("Result", 70)]
            
            for label_text, x_pos in indicator_info:
                # 인디케이터 컨테이너 (크기 축소)
                ind_frame = tk.Frame(status_frame, bg=self.COLOR_CARD)
                ind_frame.place(x=x_pos, y=0, width=65, height=30)
                
                # LED 인디케이터 (크기 축소)
                canvas = tk.Canvas(ind_frame, width=12, height=12, 
                                 highlightthickness=0, bg=self.COLOR_CARD)
                canvas.pack(pady=(1, 0))
                canvas.create_oval(1, 1, 11, 11, fill=self.COLOR_IDLE, 
                                 outline=self.COLOR_TEXT_SECONDARY, width=1, tags="led")
                
                # 라벨 (크기 축소)
                label = tk.Label(ind_frame, text=label_text, 
                               font=("Segoe UI", 7, "bold"), 
                               fg=self.COLOR_TEXT_SECONDARY,
                               bg=self.COLOR_CARD)
                label.pack()
                
                # 상태 텍스트 (크기 축소)
                status_text = tk.Label(ind_frame, text="IDLE", 
                                     font=("Segoe UI", 6), 
                                     fg=self.COLOR_TEXT_SECONDARY,
                                     bg=self.COLOR_CARD)
                status_text.pack()
                
                indicators[label_text.lower()] = canvas
                status_labels[label_text.lower()] = status_text
            
            self.status_indicators.append(indicators)
            self.input_widgets.append(row_widgets)
            
            # 상태 라벨도 저장
            if not hasattr(self, 'status_labels'):
                self.status_labels = []
            self.status_labels.append(status_labels)

        # 오른쪽: 컨트롤 및 결과 영역
        right_frame = ttk.Frame(top_container, style="Card.TFrame", padding=8)
        right_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(5, 0))

        # 컨트롤 영역
        control_frame = tk.Frame(right_frame, bg=self.COLOR_CARD)
        control_frame.pack(fill=tk.X, pady=(0, 10))

        control_title = tk.Label(control_frame, 
                                text="Test Control",
                                font=("Segoe UI", 10, "bold"),
                                fg=self.COLOR_TEXT,
                                bg=self.COLOR_CARD)
        control_title.pack(anchor=tk.W, pady=(0, 5))

        # 버튼과 상태 LED를 가로로 배치
        button_row_frame = tk.Frame(control_frame, bg=self.COLOR_CARD)
        button_row_frame.pack(fill=tk.X)

        # 버튼 영역
        button_area = tk.Frame(button_row_frame, bg=self.COLOR_CARD)
        button_area.pack(side=tk.LEFT, fill=tk.Y)

        self.btn_connect = ttk.Button(button_area, text="START TEST", 
                                     style="Primary.TButton",
                                     command=self.on_connect)
        self.btn_connect.pack(side=tk.LEFT, padx=(0, 10), ipadx=12, ipady=6)

        self.btn_stop = ttk.Button(button_area, text="STOP TEST", 
                                  style="Stop.TButton",
                                  command=self.on_stop, 
                                  state=tk.DISABLED)
        self.btn_stop.pack(side=tk.LEFT, ipadx=12, ipady=6)
        
        # 상태 LED 영역 (버튼 옆)
        status_led_frame = tk.Frame(button_row_frame, bg=self.COLOR_CARD)
        status_led_frame.pack(side=tk.LEFT, padx=(20, 0), fill=tk.Y)
        
        # 전체 상태 LED
        led_container = tk.Frame(status_led_frame, bg=self.COLOR_CARD)
        led_container.pack(side=tk.LEFT)
        
        self.overall_status_canvas = tk.Canvas(led_container, width=35, height=35, 
                                             highlightthickness=0, bg=self.COLOR_CARD)
        self.overall_status_canvas.pack()
        self.overall_status_canvas.create_oval(3, 3, 32, 32, fill=self.COLOR_IDLE, 
                                             outline=self.COLOR_TEXT_SECONDARY, width=2, tags="status_led")
        
        # 상태 정보 (LED 옆)
        status_info_frame = tk.Frame(status_led_frame, bg=self.COLOR_CARD)
        status_info_frame.pack(side=tk.LEFT, padx=(10, 0), fill=tk.Y)
        
        self.overall_status_label = tk.Label(status_info_frame, text="READY", 
                                           font=("Segoe UI", 9, "bold"), 
                                           fg=self.COLOR_TEXT_SECONDARY,
                                           bg=self.COLOR_CARD)
        self.overall_status_label.pack(anchor=tk.W)
        
        # 에러 소켓 정보 라벨
        self.error_sockets_label = tk.Label(status_info_frame, text="", 
                                          font=("Segoe UI", 8), 
                                          fg=self.COLOR_ERROR,
                                          bg=self.COLOR_CARD,
                                          wraplength=150)
        self.error_sockets_label.pack(anchor=tk.W, pady=(2, 0))

        # 결과 테이블 영역
        results_frame = tk.Frame(right_frame, bg=self.COLOR_CARD)
        results_frame.pack(fill=tk.BOTH, expand=True)

        results_title = tk.Label(results_frame,
                                text="Test Results",
                                font=("Segoe UI", 10, "bold"),
                                fg=self.COLOR_TEXT,
                                bg=self.COLOR_CARD)
        results_title.pack(anchor=tk.W, pady=(0, 5))

        # 트리뷰로 결과 테이블 생성 (헤더 텍스트 단순화)
        columns = ("Socket", "QSPI Speed", "QSPI Accuracy", "Overall Status")

        self.result_tree = ttk.Treeview(results_frame, columns=columns, show="headings", height=4, style="Custom.Treeview")
        
        # 컬럼 설정 (단순화된 헤더)
        col_widths = [100, 150, 150, 200]
        simple_headers = ["Socket", "QSPI Speed", "QSPI Acc.", "Status"]
        
        for i, (col, width, header) in enumerate(zip(columns, col_widths, simple_headers)):
            self.result_tree.heading(col, text=header)
            self.result_tree.column(col, width=width, anchor=tk.CENTER)

        # SPI/QSPI 시각적 구분을 위한 스타일 태그 설정
        self._setup_tree_tags()

        # Overall Status 컬럼에 LED 추가를 위한 준비
        self.overall_status_leds = []

        # 초기 데이터 입력
        for i in range(SOCKET_COUNT):
            self.result_tree.insert("", tk.END, iid=str(i), values=(
                f"📡 Socket {i+1}", " 0.00 Mbps", "⏸️ 0.0%", "⏸️ Ready"
            ), tags=(f"socket_{i}_ready",))
        self.result_tree.pack(fill=tk.BOTH, expand=True)

        # 로그 섹션 (하단, 크기 축소)
        log_frame = ttk.Frame(main_frame, style="Card.TFrame", padding=8)
        log_frame.pack(fill=tk.BOTH, expand=True, pady=(0, 0))

        log_title = tk.Label(log_frame,
                            text="Test Log",
                            font=("Segoe UI", 10, "bold"),
                            fg=self.COLOR_TEXT,
                            bg=self.COLOR_CARD)
        log_title.pack(anchor=tk.W, pady=(0, 5))

        self.log_box = ScrolledText(log_frame, height=12, state=tk.DISABLED, 
                                   wrap=tk.WORD, font=("Consolas", 8),
                                   bg="#1e1e1e", fg="#ffffff",
                                   insertbackground="#ffffff")
        self.log_box.pack(fill=tk.BOTH, expand=True)

    def _toggle_enable(self, socket_idx):
        """Enable 버튼 토글"""
        current_state = self.enable_vars[socket_idx].get()
        new_state = not current_state
        self.enable_vars[socket_idx].set(new_state)
        
        # 버튼 외관 업데이트
        btn = self.input_widgets[socket_idx][2]  # enable 버튼
        if new_state:
            btn.config(text="✓", bg=self.COLOR_SUCCESS, fg="white")
            # 활성화 시 핑 테스트 시작
            self._on_ip_change(socket_idx)
        else:
            btn.config(text="✗", bg=self.COLOR_IDLE, fg="white")
            # 비활성화 시 핑 LED를 회색으로
            self._update_ping_led(socket_idx, "disabled")

    def _set_indicator(self, socket_idx: int, indicator_type: str, status: str):
        """상태 인디케이터 업데이트"""
        color_map = {
            "idle": self.COLOR_IDLE,
            "connecting": self.COLOR_PRIMARY,
            "testing": self.COLOR_WARNING,
            "success": self.COLOR_SUCCESS,
            "error": self.COLOR_ERROR,
        }
        
        status_text_map = {
            "idle": "IDLE",
            "connecting": "CONNECTING",
            "testing": "TESTING",
            "success": "SUCCESS",
            "error": "ERROR",
        }
        
        canvas = self.status_indicators[socket_idx][indicator_type]
        status_label = self.status_labels[socket_idx][indicator_type]
        
        color = color_map.get(status, self.COLOR_IDLE)
        text = status_text_map.get(status, "IDLE")
        
        canvas.itemconfig("led", fill=color)
        status_label.config(text=text, fg=color)

    def _log(self, msg: str, socket_idx: int | None = None, is_error: bool = False):
        ts = datetime.now().strftime("%H:%M:%S.%f")[:-3]
        line = f"{ts} {msg}\n"
        self.log_q.put(line)
        
        if socket_idx is not None:
            self.socket_logs[socket_idx].append(line)
            if is_error:
                # 에러 정보를 더 상세하게 기록
                error_info = {
                    'timestamp': ts,
                    'socket': socket_idx + 1,
                    'message': msg,
                    'full_line': line
                }
                self.error_logs.append(error_info)

    def _update_log(self):
        try:
            while True:
                line = self.log_q.get_nowait()
                self.log_box.config(state=tk.NORMAL)
                self.log_box.insert(tk.END, line)
                self.log_box.see(tk.END)
                self.log_box.config(state=tk.DISABLED)
        except queue.Empty:
            pass
        self.root.after(100, self._update_log)

    def _start_ping_monitor(self):
        """핑 모니터링 시작"""
        self.ping_stop_event.clear()
        self.ping_thread = threading.Thread(target=self._ping_monitor, daemon=True)
        self.ping_thread.start()

    def _stop_ping_monitor(self):
        """핑 모니터링 중지"""
        self.ping_stop_event.set()
        if self.ping_thread and self.ping_thread.is_alive():
            self.ping_thread.join(timeout=1.0)

    def _ping_monitor(self):
        """핑 모니터링 루프 (100ms마다 실행)"""
        print(f"Ping monitor started with ICMP ping. Socket count: {SOCKET_COUNT}")
        
        while not self.ping_stop_event.is_set():
            for i in range(SOCKET_COUNT):
                if self.ping_stop_event.is_set():
                    break
                    
                if not self.enable_vars[i].get():
                    # 비활성화된 소켓은 회색으로 표시
                    self._update_ping_led(i, "disabled")
                    continue
                
                ip = self.ip_vars[i].get().strip()
                if not ip:
                    self._update_ping_led(i, "disabled")
                    continue
                
                # 핑 테스트 수행
                ping_success = self._test_ping(ip)
                self.ping_states[i]["reachable"] = ping_success
                self.ping_states[i]["last_ping"] = time.time()
                
                # LED 업데이트 (디버깅 출력 제거)
                if ping_success:
                    self._update_ping_led(i, "success")
                else:
                    self._update_ping_led(i, "error")
            
            # 100ms 대기
            for _ in range(10):  # 10 x 10ms = 100ms
                if self.ping_stop_event.is_set():
                    break
                time.sleep(0.01)

    def _test_ping(self, ip: str) -> bool:
        """실제 ICMP 핑 테스트"""
        import subprocess
        import platform
        
        try:
            # Windows에서는 ping -n 1, Linux/Mac에서는 ping -c 1
            if platform.system().lower() == "windows":
                # Windows ping: ping -n 1 -w 500 (500ms 타임아웃)
                result = subprocess.run(
                    ["ping", "-n", "1", "-w", "500", ip],
                    capture_output=True,
                    timeout=1.0,  # 전체 프로세스 타임아웃
                    text=True
                )
            else:
                # Linux/Mac ping: ping -c 1 -W 500
                result = subprocess.run(
                    ["ping", "-c", "1", "-W", "500", ip],
                    capture_output=True,
                    timeout=1.0,
                    text=True
                )
            
            # ping 명령어의 반환값이 0이면 성공
            return result.returncode == 0
            
        except subprocess.TimeoutExpired:
            return False
        except Exception as e:
            # 예외 발생 시 폴백으로 소켓 연결 테스트
            try:
                sock = socket.create_connection((ip, 5000), timeout=0.3)
                sock.close()
                return True
            except:
                return False

    def _update_ping_led(self, socket_idx: int, status: str):
        """핑 LED 상태 업데이트"""
        if not hasattr(self, 'ping_indicators') or socket_idx >= len(self.ping_indicators) or socket_idx < 0:
            return
            
        color_map = {
            "success": self.COLOR_SUCCESS,    # 연결 가능 - 초록색
            "error": self.COLOR_ERROR,        # 연결 불가 - 빨간색
            "disabled": self.COLOR_IDLE,      # 비활성화 - 회색
        }
        
        color = color_map.get(status, self.COLOR_IDLE)
        canvas = self.ping_indicators[socket_idx]
        
        # 메인 스레드에서 UI 업데이트
        def update_ui():
            try:
                if canvas.winfo_exists():  # 위젯이 존재하는지 확인
                    canvas.itemconfig("ping_led", fill=color)
            except Exception as e:
                print(f"Ping LED update error for socket {socket_idx}: {e}")
        
        # 메인 스레드에서 실행되도록 예약
        try:
            self.root.after(0, update_ui)
        except:
            pass  # root가 없는 경우 무시

    def _on_ip_change(self, socket_idx: int):
        """IP 주소 변경 시 핑 상태 즉시 업데이트"""
        if not hasattr(self, 'ping_indicators') or socket_idx >= len(self.ping_indicators):
            return
            
        # IP 변경 후 잠깐 대기하고 핑 테스트
        def delayed_ping_test():
            time.sleep(0.1)  # 입력 완료 대기
            if not self.enable_vars[socket_idx].get():
                self._update_ping_led(socket_idx, "disabled")
                return
                
            ip = self.ip_vars[socket_idx].get().strip()
            if not ip:
                self._update_ping_led(socket_idx, "disabled")
                return
            
            # 즉시 핑 테스트
            ping_success = self._test_ping(ip)
            self.ping_states[socket_idx]["reachable"] = ping_success
            self.ping_states[socket_idx]["last_ping"] = time.time()
            
            if ping_success:
                self._update_ping_led(socket_idx, "success")
            else:
                self._update_ping_led(socket_idx, "error")
        
        # 백그라운드에서 핑 테스트 실행
        threading.Thread(target=delayed_ping_test, daemon=True).start()

    def on_connect(self):
        self._toggle_inputs(False)
        self.stop_event.clear()
        self.error_logs.clear()  # 에러 로그 초기화
        
        # 패킷 분석 데이터 초기화
        for i in range(SOCKET_COUNT):
            self.packet_analysis[i].clear()
        
        # 테스트 결과 초기화
        for i in range(SOCKET_COUNT):
            self.test_results[i] = {
                "qspi": {"success": False, "loops": 0, "accuracy": 0.0, "mbps": 0.0},
            }
        
        # 전체 상태 LED를 테스트 중으로 설정
        self._set_overall_status_led("testing")
        
        # 모든 인디케이터 초기화 및 Enable 버튼 상태 설정
        for i in range(SOCKET_COUNT):
            for indicator_type in ["qspi", "result"]:
                self._set_indicator(i, indicator_type, "idle")
            
            # Enable 버튼 초기 상태 설정
            btn = self.input_widgets[i][2]
            if self.enable_vars[i].get():
                btn.config(text="✓", bg=self.COLOR_SUCCESS, fg="white")
            else:
                btn.config(text="✗", bg=self.COLOR_IDLE, fg="white")
            
            # 결과 테이블 초기화
            self.result_tree.item(str(i), values=(
                f"📡 Socket {i+1}", "🟢 0.00 Mbps", "⏸️ 0.0%", "⏸️ Ready"
            ), tags=(f"socket_{i}_ready",))
        
        # 로그 초기화 (선택사항 - 필요하면 주석 해제)
        # self.log_box.config(state=tk.NORMAL)
        # self.log_box.delete(1.0, tk.END)
        # self.log_box.config(state=tk.DISABLED)
        
        self.tester_thread = threading.Thread(target=self._tester, daemon=True)
        self.tester_thread.start()

    def on_stop(self):
        self._log("[USER] Stop requested - Force terminating test")
        self.stop_event.set()
        
        # 강제 종료 시 모든 상태를 초기화
        self._reset_all_states()
        
        # 전체 상태 LED를 READY로 설정 (에러가 아닌 초기화 상태)
        self._set_overall_status_led("ready")
        
        # UI 입력 활성화
        self._toggle_inputs(True)

    def _toggle_inputs(self, enabled: bool):
        """입력 위젯들과 버튼 상태 토글"""
        self.btn_connect.config(state=tk.NORMAL if enabled else tk.DISABLED)
        self.btn_stop.config(state=tk.DISABLED if enabled else tk.NORMAL)
        
        # 모든 입력 위젯들 비활성화/활성화
        for row_widgets in self.input_widgets:
            for widget in row_widgets:
                if isinstance(widget, ttk.Entry):
                    widget.config(state=tk.NORMAL if enabled else tk.DISABLED)
                elif isinstance(widget, tk.Button):  # Enable 버튼
                    widget.config(state=tk.NORMAL if enabled else tk.DISABLED)

    def _connect_with_retry(self, ip: str, port: int, label: str, socket_idx: int):
        if self.stop_event.is_set():
            return None
        self._log(f"[{label}] Connecting to {ip}:{port}", socket_idx)
        try:
            sock = socket.create_connection((ip, port), timeout=2)
            sock.settimeout(1.0)
            self._log(f"[{label}] Connected", socket_idx)
            return sock
        except Exception as e:
            self._log(f"[{label}] Connect fail: {e}", socket_idx, True)
        self._log(f"[{label}] Connection failed", socket_idx, True)
        return None

    def _test_spi_mode(self, sock: socket.socket, label: str, socket_idx: int):
        """QSPI 모드 테스트 - 성능 기반 재시도 제거"""
        self._log(f"[{label}] Starting QSPI performance test (target: >{QSPI_MIN_MBPS:.1f} Mbps)", socket_idx)
        
        loops = 0
        ok = True
        packet_errors = []
        t0 = time.time()
        
        try:
            while (time.time() - t0) < DURATION_SEC and not self.stop_event.is_set():
                # 데이터 전송
                try:
                    sock.sendall(PAYLOAD)
                except Exception as send_e:
                    packet_errors.append({
                        'loop': loops + 1,
                        'type': 'SEND_ERROR',
                        'error': str(send_e),
                        'timestamp': time.perf_counter() - t0
                    })
                    raise send_e
                
                # 데이터 수신
                data = b""
                try:
                    while len(data) < len(PAYLOAD):
                        part = sock.recv(len(PAYLOAD) - len(data))
                        if not part:
                            packet_errors.append({
                                'loop': loops + 1,
                                'type': 'CONNECTION_CLOSED',
                                'error': 'Peer closed connection',
                                'received_bytes': len(data),
                                'expected_bytes': len(PAYLOAD),
                                'timestamp': time.perf_counter() - t0
                            })
                            raise ConnectionError("Peer closed")
                        data += part
                except Exception as recv_e:
                    packet_errors.append({
                        'loop': loops + 1,
                        'type': 'RECEIVE_ERROR',
                        'error': str(recv_e),
                        'received_bytes': len(data),
                        'expected_bytes': len(PAYLOAD),
                        'timestamp': time.perf_counter() - t0
                    })
                    raise recv_e
                
                # 데이터 검증
                if data != PAYLOAD:
                    mismatch_count = sum(1 for i, (a, b) in enumerate(zip(data, PAYLOAD)) if a != b)
                    packet_errors.append({
                        'loop': loops + 1,
                        'type': 'DATA_MISMATCH',
                        'error': 'Payload verification failed',
                        'mismatch_bytes': mismatch_count,
                        'total_bytes': len(PAYLOAD),
                        'mismatch_percentage': (mismatch_count / len(PAYLOAD)) * 100,
                        'timestamp': time.perf_counter() - t0
                    })
                    raise ValueError("Payload mismatch")
                
                loops += 1
                
                # 주기적 성능 로그 (100번마다)
                if loops % 100 == 0:
                    elapsed = time.time() - t0
                    current_mbps = (loops * len(PAYLOAD) * 2 * 8) / 1_000_000 / elapsed
                    self._log(f"[{label}] Current performance: {current_mbps:.2f} Mbps ({loops} loops)", socket_idx)
                    
        except Exception as e:
            ok = False
            self._log(f"[{label}] ERROR {type(e).__name__} – {e}", socket_idx, True)
        
        # 패킷 분석 정보 저장
        if packet_errors:
            self.packet_analysis[socket_idx].extend(packet_errors)
        
        # 최종 성능 계산
        duration = max(time.time() - t0, 1e-9)
        mbps = (loops * len(PAYLOAD) * 2 * 8) / 1_000_000 / duration
        accuracy = (loops / (loops + len(packet_errors)) * 100.0) if (loops + len(packet_errors)) > 0 else 0.0
        
        # Accuracy가 100% 미만이면 에러로 처리
        if accuracy < 100.0:
            ok = False
            self._log(f"[{label}] ACCURACY ERROR: {accuracy:.1f}% (Expected: 100.0%)", socket_idx, True)
        
        # 성능 체크
        if mbps >= QSPI_MIN_MBPS and ok:
            self._log(f"[{label}] SUCCESS: Performance target achieved ({mbps:.2f} >= {QSPI_MIN_MBPS:.1f} Mbps)", socket_idx)
        else:
            if mbps < QSPI_MIN_MBPS:
                self._log(f"[{label}] FAILED: Performance below target ({mbps:.2f} < {QSPI_MIN_MBPS:.1f} Mbps)", socket_idx, True)
                ok = False
        
        self._log(f"[{label}] FINAL RESULT: loops={loops}, errors={len(packet_errors)}, accuracy={accuracy:.1f}%, speed={mbps:.2f} Mbps", socket_idx)
        
        return ok, loops, accuracy, mbps

    def _update_result_table(self, socket_idx: int):
        """결과 테이블 업데이트 - QSPI만 표시"""
        q = self.test_results[socket_idx]["qspi"]
        
        # QSPI 성능에 따른 색상 구분
        def format_speed_with_color(mbps):
            if mbps >= QSPI_MIN_MBPS:
                color_code = "🟢"  # 임계값 이상: 초록
            elif mbps >= QSPI_MIN_MBPS * 0.8:  # 80% 이상
                color_code = "🟡"  # 중간 성능: 노랑
            else:
                color_code = "🔴"  # 낮은 성능: 빨강
            return f"{color_code} {mbps:.2f} Mbps"
        
        def format_accuracy_with_color(accuracy):
            if accuracy >= 95.0:
                return f"✅ {accuracy:.1f}%"
            elif accuracy >= 80.0:
                return f"⚠️ {accuracy:.1f}%"
            else:
                return f"❌ {accuracy:.1f}%"
        
        # Overall Status 결정
        qspi_success = q["success"]
        
        if qspi_success:
            status_display = "🎯 Complete"
            tag_suffix = "success"
        elif q["loops"] == 0:
            status_display = "⏸️ Ready"
            tag_suffix = "ready"
        else:
            status_display = "❌ Error"
            tag_suffix = "error"
        
        # 값 업데이트 (시각적 아이콘 포함)
        values = (
            f"📡 Socket {socket_idx+1}",
            format_speed_with_color(q['mbps']),
            format_accuracy_with_color(q['accuracy']),
            status_display
        )
        
        # 적절한 태그 적용
        tag_name = f"socket_{socket_idx}_{tag_suffix}"
        self.result_tree.item(str(socket_idx), values=values, tags=(tag_name,))


    def _update_overall_status(self, socket_idx: int, status: str):
        """Overall Status만 별도로 업데이트"""
        current_values = list(self.result_tree.item(str(socket_idx))['values'])
        
        status_map = {
            "ready": "⏸️ Ready",
            "testing": "🔄 Testing", 
            "complete": "🎯 Complete",
            "error": "❌ Error"
        }
        
        current_values[3] = status_map.get(status, "⏸️ Ready")  # status 컬럼 인덱스 조정
        
        # 적절한 태그 적용
        tag_name = f"socket_{socket_idx}_{status}"
        self.result_tree.item(str(socket_idx), values=current_values, tags=(tag_name,))

    def _tester(self):
        # START 버튼 후 초기 대기
        self._log(f"[SYSTEM] Waiting {INITIAL_DELAY}s for RP2350/WIZnet initialization...")
        for delay_step in range(int(INITIAL_DELAY * 10)):
            if self.stop_event.is_set():
                self._log("[STOP] Test terminated during initialization delay")
                return
            time.sleep(0.1)
        
        # 소켓별로 순차적으로 QSPI 테스트 진행
        completed_sockets = []
        error_sockets = []
        
        for i in range(SOCKET_COUNT):
            if self.stop_event.is_set():
                self._log("[STOP] Test terminated before processing sockets")
                return
                
            if not self.enable_vars[i].get():
                self._log(f"[Socket {i+1}] Disabled - skipping test", i)
                continue

            ip = self.ip_vars[i].get()
            port = self.port_vars[i].get()
            
            self._log(f"[Socket {i+1}] Starting QSPI test")
            
            # QSPI 루프백 테스트
            self._set_indicator(i, "result", "idle")
            self._update_overall_status(i, "testing")

            label = f"Socket {i+1} - QSPI"
            self._set_indicator(i, "qspi", "connecting")
            
            sock = self._connect_with_retry(ip, port, label, i)
            if not sock:
                self.test_results[i]["qspi"]["success"] = False
                self._set_indicator(i, "qspi", "error")
                self._set_indicator(i, "result", "error")
                self._update_overall_status(i, "error")
                error_sockets.append(i + 1)
            else:
                self._set_indicator(i, "qspi", "testing")
                success, loops, acc, mbps = self._test_spi_mode(sock, label, i)
                sock.close()
                self.test_results[i]["qspi"] = {
                    "success": success,
                    "loops": loops,
                    "accuracy": acc,
                    "mbps": mbps,
                }
                self._set_indicator(i, "qspi", "success" if success else "error")
                
                if success:
                    self._set_indicator(i, "result", "success")
                    self._update_overall_status(i, "complete")
                    completed_sockets.append(i + 1)
                    self._log(f"[Socket {i+1}] QSPI TEST COMPLETE - Success", i)
                else:
                    self._set_indicator(i, "result", "error")
                    self._update_overall_status(i, "error")
                    error_sockets.append(i + 1)
                    self._log(f"[Socket {i+1}] QSPI TEST FAILED", i, True)

            self._update_result_table(i)
            self._log(
                f"[SUMMARY] Socket {i+1}: "
                f"QSPI {self.test_results[i]['qspi']['mbps']:.2f} Mbps",
                i,
            )

        # 전체 테스트 결과 평가
        self._log(f"[SYSTEM] All socket tests completed. Success: {completed_sockets}, Errors: {error_sockets}")
        
        # 전체 상태 LED 설정
        if error_sockets:
            self._set_overall_status_led("error", error_sockets)
        else:
            self._set_overall_status_led("success")

        # 강제 종료가 아닌 경우에만 에러 로그 작성
        if not self.stop_event.is_set():
            self._write_unified_error_log()
        else:
            self._log("[STOP] Test manually terminated - skipping error log generation")
        
        self._toggle_inputs(True)
        self.stop_event.set()

    def _write_unified_error_log(self):
        """통합 에러 로그 파일 생성 - 패킷 분석 정보 포함"""
        has_errors = bool(self.error_logs) or any(self.packet_analysis)
        
        if not has_errors:
            self._log("[FILE] No errors occurred - all tests completed successfully")
            return
            
        ts = datetime.now().strftime("%Y%m%d-%H%M%S")
        fname = Path.cwd() / f"loopback_test_errors_{ts}.txt"
        
        try:
            with fname.open("w", encoding="utf-8") as f:
                f.write("=== WIZnet ioModule Multi-SPI Loopback Test Error Report ===\n")
                f.write(f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
                f.write(f"Total Connection Errors: {len(self.error_logs)}\n")
                total_packet_errors = sum(len(analysis) for analysis in self.packet_analysis)
                f.write(f"Total Packet Errors: {total_packet_errors}\n")
                f.write("=" * 60 + "\n\n")
                
                # 소켓별로 에러 그룹화
                socket_errors = {}
                for error in self.error_logs:
                    socket_num = error['socket']
                    if socket_num not in socket_errors:
                        socket_errors[socket_num] = {'spi': [], 'qspi': [], 'other': []}
                    
                    msg = error['message']
                    if 'Normal SPI' in msg:
                        socket_errors[socket_num]['spi'].append(error)
                    elif 'Quad QSPI' in msg:
                        socket_errors[socket_num]['qspi'].append(error)
                    else:
                        socket_errors[socket_num]['other'].append(error)
                
                # 패킷 분석 에러도 추가
                for socket_idx, packet_errors in enumerate(self.packet_analysis):
                    if packet_errors:
                        socket_num = socket_idx + 1
                        if socket_num not in socket_errors:
                            socket_errors[socket_num] = {'spi': [], 'qspi': [], 'other': [], 'packet_analysis': []}
                        else:
                            socket_errors[socket_num]['packet_analysis'] = []
                        socket_errors[socket_num]['packet_analysis'] = packet_errors
                
                # 소켓별 에러 출력
                for socket_num in sorted(socket_errors.keys()):
                    f.write(f"SOCKET {socket_num} ERRORS:\n")
                    f.write("-" * 30 + "\n")
                    
                    for mode, errors in socket_errors[socket_num].items():
                        if not errors:
                            continue
                            
                        if mode == 'packet_analysis':
                            f.write(f"\nPACKET ANALYSIS ERRORS:\n")
                            
                            # 에러 타입별 통계
                            error_types = {}
                            for error in errors:
                                error_type = error['type']
                                if error_type not in error_types:
                                    error_types[error_type] = []
                                error_types[error_type].append(error)
                            
                            for error_type, type_errors in error_types.items():
                                f.write(f"  {error_type} ({len(type_errors)} occurrences):\n")
                                
                                if error_type == 'DATA_MISMATCH':
                                    total_mismatches = sum(e['mismatch_bytes'] for e in type_errors)
                                    avg_mismatch_pct = sum(e['mismatch_percentage'] for e in type_errors) / len(type_errors)
                                    f.write(f"    Total mismatch bytes: {total_mismatches}\n")
                                    f.write(f"    Average mismatch percentage: {avg_mismatch_pct:.2f}%\n")
                                    
                                    # 상세 정보 (처음 5개만)
                                    for i, error in enumerate(type_errors[:5]):
                                        f.write(f"    Loop {error['loop']}: {error['mismatch_bytes']}/{error['total_bytes']} bytes ({error['mismatch_percentage']:.1f}%)\n")
                                    if len(type_errors) > 5:
                                        f.write(f"    ... and {len(type_errors) - 5} more\n")
                                
                                elif error_type in ['SEND_ERROR', 'RECEIVE_ERROR', 'CONNECTION_CLOSED']:
                                    # 처음 3개 에러만 상세 표시
                                    for i, error in enumerate(type_errors[:3]):
                                        f.write(f"    Loop {error['loop']} at {error['timestamp']:.3f}s: {error['error']}\n")
                                    if len(type_errors) > 3:
                                        f.write(f"    ... and {len(type_errors) - 3} more similar errors\n")
                        else:
                            f.write(f"\n{mode.upper()} Mode Errors:\n")
                            for error in errors:
                                f.write(f"  [{error['timestamp']}] {error['message']}\n")
                    f.write("\n")
                
                # 전체 에러 로그 (시간순)
                if self.error_logs:
                    f.write("\nCHRONOLOGICAL CONNECTION ERROR LOG:\n")
                    f.write("=" * 30 + "\n")
                    for error in self.error_logs:
                        f.write(f"[Socket {error['socket']}] {error['full_line']}")
            
            self._log(f"[FILE] Unified error log saved: {fname}")
        except Exception as e:
            self._log(f"[FILE] Failed to write error log: {fname} – {e}")

    def _setup_tree_tags(self):
        """트리뷰의 SPI/QSPI 시각적 구분을 위한 태그 스타일 설정"""
        # 기본 소켓 행 스타일
        for i in range(SOCKET_COUNT):
            # 소켓별 기본 스타일
            self.result_tree.tag_configure(f"socket_{i}", 
                                         background=self.COLOR_CARD,
                                         foreground=self.COLOR_TEXT)
            
            # 상태별 스타일
            self.result_tree.tag_configure(f"socket_{i}_ready", 
                                         background=self.COLOR_CARD,
                                         foreground=self.COLOR_TEXT_SECONDARY)
            
            self.result_tree.tag_configure(f"socket_{i}_testing", 
                                         background="#2a4d6b",  # 어두운 파란색 배경
                                         foreground="#ffffff")
            
            self.result_tree.tag_configure(f"socket_{i}_success", 
                                         background="#1a4d3a",  # 어두운 초록색 배경
                                         foreground="#ffffff")
            
            self.result_tree.tag_configure(f"socket_{i}_partial", 
                                         background="#4d3a1a",  # 어두운 주황색 배경
                                         foreground="#ffffff")
            
            self.result_tree.tag_configure(f"socket_{i}_error", 
                                         background="#4d1a1a",  # 어두운 빨간색 배경
                                         foreground="#ffffff")

    def _set_overall_status_led(self, status: str, error_sockets: list = None):
        """전체 테스트 상태 LED 업데이트"""
        color_map = {
            "ready": self.COLOR_IDLE,
            "testing": self.COLOR_PRIMARY,
            "success": self.COLOR_SUCCESS,
            "error": self.COLOR_ERROR,
        }
        
        status_text_map = {
            "ready": "READY",
            "testing": "TESTING",
            "success": "SUCCESS",
            "error": "ERROR",
        }
        
        color = color_map.get(status, self.COLOR_IDLE)
        text = status_text_map.get(status, "READY")
        
        self.overall_status_canvas.itemconfig("status_led", fill=color)
        self.overall_status_label.config(text=text, fg=color)
        
        # 에러 소켓 정보 표시
        if status == "error" and error_sockets:
            error_text = f"Error Sockets:\n{', '.join(map(str, error_sockets))}"
            self.error_sockets_label.config(text=error_text)
        else:
            self.error_sockets_label.config(text="")

    def _reset_all_states(self):
        """강제 종료 시 모든 상태를 초기 상태로 리셋"""
        # 테스트 결과 초기화
        for i in range(SOCKET_COUNT):
            self.test_results[i] = {
                "qspi": {"success": False, "loops": 0, "accuracy": 0.0, "mbps": 0.0},
            }
        
        # 모든 인디케이터를 IDLE로 설정
        for i in range(SOCKET_COUNT):
            for indicator_type in ["qspi", "result"]:
                self._set_indicator(i, indicator_type, "idle")
            
            # 결과 테이블 초기화
            self.result_tree.item(str(i), values=(
                f"📡 Socket {i+1}", "🟢 0.00 Mbps", "⏸️ 0.0%", "⏸️ Ready"
            ), tags=(f"socket_{i}_ready",))
        
        # 에러 로그 및 패킷 분석 초기화
        self.error_logs.clear()
        for i in range(SOCKET_COUNT):
            self.packet_analysis[i].clear()

if __name__ == "__main__":
    tk_root = tk.Tk()
    ui = ModernLoopbackUI(tk_root)
    
    # 프로그램 종료 시 핑 모니터링 정리
    def on_closing():
        ui._stop_ping_monitor()
        tk_root.destroy()
    
    tk_root.protocol("WM_DELETE_WINDOW", on_closing)
    tk_root.mainloop()