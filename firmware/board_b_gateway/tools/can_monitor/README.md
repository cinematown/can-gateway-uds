# CAN Traffic Monitor (CANable v2.0)

PC에서 CAN 버스를 모니터링하거나 UDS 요청을 강제로 보낼 때 사용.

## 하드웨어
- CANable v2.0 (slcan 펌웨어)
- 120Ω 종단저항 (필요 시 추가)

## Linux/Ubuntu

### 설정 (1회)
```bash
sudo apt install can-utils
sudo modprobe slcan

sudo slcand -o -c -s6 /dev/ttyACM0 can0   # s6 = 500kbps
sudo ip link set up can0
```

### 모니터링
```bash
candump can0                              # 전체 트래픽
candump can0 | grep '280\|1A0'            # RPM/Speed 만
```

### UDS 요청 송신 (CLI 없이 테스트)
```bash
# read_did F40C (현재 RPM)
cansend can0 7DF#0322F40C00000000
```

## Windows
- Zadig 로 libusb-win32 드라이버 설치
- `slcand` 대체로 PCAN-View, Kvaser CanKing 등 GUI 툴 사용
- 또는 WSL2 + USB passthrough

## 주의
- **ELM327 어댑터는 쓰지 마세요.** UDS raw frame 전송 불가.
- 버스에 이미 보드B가 있으면 CANable 은 종단저항 없이 연결.
- 보드B 없이 CANable 단독 테스트 시 120Ω 추가.
