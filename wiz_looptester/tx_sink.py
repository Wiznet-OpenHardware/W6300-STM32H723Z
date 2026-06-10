"""
TX 싱크 (진단 전용) — 검사 없이 받고 버리기만 하는 최소 수신 서버.

목적: 장비 TX 속도가 "받는 프로그램"에 묶이는지 가린다.
      패턴검사·GUI·스레드·로직이 전혀 없는 순수 recv 루프.

사용:
    python tx_sink.py            # 0.0.0.0:5000
    python tx_sink.py 5000       # 포트 지정

장비를 QSPI(또는 BUS) TX로 접속시키고 → **장비 시리얼에 찍히는 TX Mbps** 를 본다.
  - 80~90 나옴  → PC(파이썬 GUI 툴)의 수신 처리가 병목. 펌웨어는 정상.
  - 여전히 ~49 → 펌웨어 송신 루프 문제 (iperf 방식으로 교체).

아래 PC측 Mbps는 참고용 교차검증 (검사 없는 순수 recv 속도).
"""

import sys
import time
import socket

PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 5000


def main():
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind(("0.0.0.0", PORT))
    srv.listen(1)
    print(f"[SINK] listening on 0.0.0.0:{PORT}  (recv & discard, 검사 없음)")

    try:
        while True:
            conn, addr = srv.accept()
            try:
                conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
                conn.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 1 << 20)
            except OSError:
                pass
            print(f"[SINK] connected: {addr[0]}:{addr[1]}")

            total = 0
            t0 = None
            while True:
                data = conn.recv(65536)     # 그냥 받고 버림
                if not data:
                    break
                if t0 is None:
                    t0 = time.perf_counter()
                total += len(data)

            dt = (time.perf_counter() - t0) if t0 else 0.0
            mbps = (total * 8 / dt / 1_000_000) if dt > 0 else 0.0
            print(f"[SINK] closed: {total} bytes / {dt:.3f}s / PC측 {mbps:.2f} Mbps (참고용)")
            conn.close()
    except KeyboardInterrupt:
        print("\n[SINK] bye")
    finally:
        srv.close()


if __name__ == "__main__":
    main()
