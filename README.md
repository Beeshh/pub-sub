# SOME/IP Pub/Sub Multi-ECU Communication System
### v2.0 — Real-Time Event-Driven Architecture
### Built on Raspberry Pi using vSomeIP (C++) and someipy (Python)

**Project Members:**
- Bashar Hejazi
- Jorge Castellanos
- Mohamed Eltahan

---

## From v1.0 to v2.0

In **v1.0** of this project ([someip-rpc-project](https://github.com/Beeshh/someip-rpc-project)), we implemented a **Remote Procedure Call (RPC)** communication system where:
- The Climate ECU explicitly asked the Window ECU for the current window position
- The Window ECU responded once and the communication ended
- Each exchange required a request before any data was sent

**v2.0** evolves the architecture into a **Publish/Subscribe (Pub/Sub)** event-driven system where:
- The Speed ECU continuously broadcasts the vehicle speed in real time
- The Window ECU continuously publishes the window position based on speed logic
- The Climate ECU subscribes and receives updates automatically — no requests needed
- All three ECUs run simultaneously and independently, reacting to events as they happen

This shift mirrors how real automotive systems work — ECUs don't poll each other constantly. They publish data and interested parties subscribe to it, making the system far more efficient for real-time communication.

---

## What's New in v2.0

| Feature | v1.0 (RPC) | v2.0 (Pub/Sub) |
|---|---|---|
| Communication style | Request → Response | Continuous event stream |
| Data flow | On-demand | Real-time, automatic |
| Speed awareness | None | Speed drives window logic |
| Window modes | Fixed at 30% | 4 dynamic modes (100/60/30/10%) |
| Climate ECU output | Single value | Speed range + window level |
| Updates | One shot | Continuous, every 2 seconds |
| Real-time feel | No | Yes |

---

## System Architecture

```
┌──────────────────────────────────────────────────────────┐
│                      Raspberry Pi                        │
│                                                          │
│   ┌─────────────────────────────────────────────────┐   │
│   │           someipy Daemon (someipyd)              │   │
│   │     Manages Python SOME/IP SD & routing         │   │
│   └─────────────────────────────────────────────────┘   │
│                         │                                │
│   ┌─────────────────────▼──────────────────────┐        │
│   │           Speed ECU (Python/someipy)        │        │
│   │   Publishes speed: 0→120→0 km/h cycling    │        │
│   │   Every 2 seconds via SOME/IP event         │        │
│   └────────────────────────────────────────────┘        │
│                                                          │
│   ┌────────────────────────────────────────────┐        │
│   │          Window ECU (C++/vSomeIP)           │        │
│   │   Simulates speed, calculates window mode  │        │
│   │   Publishes window position every 2 seconds│        │
│   └───────────────────┬────────────────────────┘        │
│                       │ SOME/IP Event                    │
│   ┌───────────────────▼────────────────────────┐        │
│   │          Climate ECU (C++/vSomeIP)          │        │
│   │   Subscribes to window events               │        │
│   │   Prints window level + speed range mode   │        │
│   └────────────────────────────────────────────┘        │
└──────────────────────────────────────────────────────────┘
```

---

## Speed-Based Window Control Logic

The core intelligence of v2.0 is the **automatic window adjustment based on vehicle speed**. At higher speeds, windows close for aerodynamics and cabin noise reduction — exactly as real vehicles behave.

```
Speed 0–30 km/h    →  Window 100%  (fully open)
Speed 31–60 km/h   →  Window 60%   (partially open)
Speed 61–90 km/h   →  Window 30%   (mostly closed)
Speed 91–120 km/h  →  Window 10%   (nearly closed)
```

The Speed ECU cycles continuously:
```
0 → 10 → 20 → ... → 120 → 110 → ... → 0 → 10 → ...
```

This produces a continuous stream of window position changes that the Climate ECU receives and displays in real time.

---

## Window Modes

v2.0 introduces **4 distinct window modes** displayed by the Climate ECU:

| Mode | Window | Speed Range | Description |
|---|---|---|---|
| LOW | 100% | 0–30 km/h | City driving, windows fully open |
| MEDIUM | 60% | 31–60 km/h | Suburban driving, partially open |
| HIGH | 30% | 61–90 km/h | Highway driving, mostly closed |
| FAST | 10% | 91–120 km/h | High speed, nearly closed |

**Climate ECU output example:**
```
Climate ECU: Window=100 % | Vehicle Speed=0-30 km/h   (LOW)
Climate ECU: Window=60 %  | Vehicle Speed=31-60 km/h  (MEDIUM)
Climate ECU: Window=30 %  | Vehicle Speed=61-90 km/h  (HIGH)
Climate ECU: Window=10 %  | Vehicle Speed=91-120 km/h (FAST)
```

---

## SOME/IP Service Identifiers

**Window ECU (C++ / vSomeIP)**
| Identifier | Value |
|---|---|
| Service ID | 0x1111 |
| Instance ID | 0x2222 |
| Event ID | 0x8001 |
| Event Group | 0x0001 |

**Speed ECU (Python / someipy)**
| Identifier | Value |
|---|---|
| Service ID | 0x2222 |
| Instance ID | 0x3333 |
| Event ID | 0x8002 |
| Event Group | 0x0002 |

---

## Project Structure

```
pub-sub/
├── window_ecu.hpp          # Window ECU publisher (C++)
├── window_ecu_main.cpp     # Window ECU entry point
├── climate_ecu.hpp         # Climate ECU subscriber (C++)
├── climate_ecu_main.cpp    # Climate ECU entry point
├── CMakeLists.txt          # CMake build configuration
├── helloworld-local.json   # vSomeIP network configuration
├── speed_ecu_pubsub.py     # Speed ECU publisher (Python/someipy)
├── someipyd.json           # someipy daemon configuration
└── README.md
```

---

## Hardware & Environment

- **Hardware:** Raspberry Pi
- **OS:** Raspberry Pi OS (Debian Linux)
- **Network:** Ethernet (`eth0`), IP: `100.88.162.130` (dynamic)
- **Remote Access:** SSH from Windows PC
- **C++ Middleware:** vSomeIP 3.7.2
- **Python Middleware:** someipy 2.1.2
- **Build System:** CMake 3.31.6
- **Python:** 3.13

---

## Dependencies

### C++ Side
```bash
git clone https://github.com/COVESA/vsomeip.git
cd vsomeip && mkdir build && cd build
cmake .. && make
sudo make install && sudo ldconfig
```

### Python Side
```bash
pip install someipy --break-system-packages
```

---

## Building the C++ ECUs

```bash
git clone https://github.com/Beeshh/pub-sub.git
cd pub-sub
mkdir build && cd build
cmake ..
make
```

---

## Running the Full System (4 Terminals)

Start in this exact order:

**Terminal 1 — someipy Daemon (always first)**
```bash
someipyd --config ~/pub-sub/someipyd.json
```
Wait for:
```
someipyd [INFO]: Unix domain socket server started at /tmp/someipyd.sock
```

**Terminal 2 — Speed ECU (Python publisher)**
```bash
python3 ~/pub-sub/speed_ecu_pubsub.py
```
```
Speed ECU started - broadcasting vehicle speed...
Speed ECU: Broadcasting speed = 0 km/h
Speed ECU: Broadcasting speed = 10 km/h
...
```

**Terminal 3 — Window ECU (C++ publisher)**
```bash
cd ~/pub-sub/build
VSOMEIP_CONFIGURATION=../helloworld-local.json \
VSOMEIP_APPLICATION_NAME=window_ecu_service \
./window_ecu
```
```
Window ECU: Publishing window position = 100 %
Window ECU: Publishing window position = 60 %
...
```

**Terminal 4 — Climate ECU (C++ subscriber)**
```bash
cd ~/pub-sub/build
VSOMEIP_CONFIGURATION=../helloworld-local.json \
VSOMEIP_APPLICATION_NAME=climate_ecu_client \
./climate_ecu
```
```
Climate ECU: Window=100 % | Vehicle Speed=0-30 km/h   (LOW)
Climate ECU: Window=60 %  | Vehicle Speed=31-60 km/h  (MEDIUM)
Climate ECU: Window=30 %  | Vehicle Speed=61-90 km/h  (HIGH)
Climate ECU: Window=10 %  | Vehicle Speed=91-120 km/h (FAST)
```

---

## Why Pub/Sub is Better for Real-Time

In **v1.0 (RPC)**, communication looked like this:
```
Climate ECU: "What is the window position?"  → Window ECU: "30%"  [done]
```
One question, one answer, connection closed. Not suitable for continuously changing data.

In **v2.0 (Pub/Sub)**, communication looks like this:
```
Window ECU: "Window=100%" → Climate ECU receives automatically
Window ECU: "Window=60%"  → Climate ECU receives automatically
Window ECU: "Window=30%"  → Climate ECU receives automatically
Window ECU: "Window=10%"  → Climate ECU receives automatically
```
No requests needed. Data flows continuously. Any number of subscribers can receive the same event simultaneously. This is how real automotive systems handle sensor data, speed signals, and state changes.

---

## Cross-Middleware Integration

A key achievement of this project is running **two different SOME/IP middleware stacks simultaneously**:

- **vSomeIP (C++)** handles the Window ECU and Climate ECU
- **someipy (Python)** handles the Speed ECU with its own daemon

Both stacks operate independently on the same Raspberry Pi, each offering and consuming SOME/IP services without interfering with each other. This demonstrates how real automotive networks handle multi-vendor ECU integration.

---

## Relevance to Automotive Software

This project demonstrates concepts directly used in:

- **AUTOSAR Adaptive** — service-oriented middleware with Pub/Sub event groups
- **SOME/IP event notifications** — standard in modern vehicle E/E architectures
- **Zonal ECU architectures** — multiple ECUs reacting to shared vehicle state
- **Real-time vehicle data distribution** — speed, position, sensor data
- **Multi-vendor middleware integration** — different stacks coexisting on the same network

---

## Future Improvements

- Direct cross-middleware subscription (Python Speed ECU → C++ Window ECU via bridge)
- More ECUs subscribing to the same speed event simultaneously
- Dynamic speed input from a real sensor or keyboard
- Window control commands (manual override via RPC on top of Pub/Sub)
- Combining v1.0 RPC and v2.0 Pub/Sub in a unified ECU network
