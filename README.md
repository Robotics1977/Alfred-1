# Alfred 1

Alfred 1 is an autonomous mobile robot built using a Raspberry Pi 5, ROS 2, custom motor control electronics, and multiple sensors including LiDAR, thermal imaging, IMU, and computer vision.

This project is part of the AstroGear Labs robotics initiative.

---

## 📦 Current Hardware

### **Compute + OS**
- Raspberry Pi 5 (8GB)
- 512GB SSD (Ubuntu 24.04.3)
- ROS 2 Jazzy

### **Sensors**
- **RPLIDAR A1M8** (360° 2D LiDAR, ~12m range, 10 Hz scan rate)
- **EMEET C960** (1080p USB camera, 30 FPS, calibrated)
- **MPU-6050** (6-axis IMU — accelerometer + gyroscope)
- **AMG8833** (8×8 thermal camera — hotspot/person detection)
- **3× HC-SR04 ultrasonic sensors** (left, right, rear)

### **Motor System**
- **Two 12V JGA25-371 DC motors with encoders**
  - 126 RPM @ 12V
  - 12 counts per revolution encoder
- **TB6612FNG dual-motor driver**

### **Power**
- 12V switching supply (bench testing) / 12V LiFePO4 battery (planned)
- 12V → 5V buck converter (Pi power via USB-C)
- Separate 12V motor bus for motors

### **Mechanical**
- Chassis redesign in progress (original base too narrow for battery + casters too close together for stability)
- Three custom PCBs fabricated via JLCPCB: motor connector board, Nano/TB6612FNG controller board, power distribution board (on/off switch + 12V in/out)
- Custom I2C breakout board for AMG8833 + MPU-6050 (includes SDA/SCL/SDO breakout)

---

## 🧠 Arduino Nano (Fixed Pin Layout)

This layout will **never be changed**, as required for Alfred 1.

| Pin | Device |
| --- | ------------------------------- |
| D2  | HC-SR04 Rear Trigger |
| D3  | HC-SR04 Rear Echo |
| D4  | HC-SR04 Left Trigger |
| D5  | HC-SR04 Left Echo |
| D6  | HC-SR04 Right Trigger |
| D7  | HC-SR04 Right Echo |
| D8  | Motor A (Left) IN1 |
| D9  | Motor A (Left) IN2 |
| D10 | Motor A (Left) PWM |
| D11 | Motor B (Right) IN1 |
| D12 | Motor B (Right) IN2 |
| D13 | Motor B (Right) PWM |
| A0  | Motor A Encoder Channel A |
| A1  | Motor A Encoder Channel B |
| A2  | Motor B Encoder Channel A |
| A3  | Motor B Encoder Channel B |
| A4  | TB6612FNG STBY |

Nano connects to the Pi 5 over USB serial at 115200 baud, fixed at `/dev/alfred_nano` via udev rule.

---

## 🌡️ I2C Bus (Raspberry Pi 5 — Pins 1/3/5/6)

Both sensors share the Pi's I2C1 bus (SDA/Pin 3, SCL/Pin 5). Addresses are kept distinct via the AD0/SDO pins:

| Device | AD0 / SDO | I2C Address |
| --- | --- | --- |
| AMG8833 | Floating (default) | `0x69` |
| MPU-6050 | Grounded (default) | `0x68` |

---

## 🔌 USB Device Map (udev rules)

| Device | Symlink | Underlying Port |
| --- | --- | --- |
| Arduino Nano | `/dev/alfred_nano` | CH340 (1a86:7523) |
| RPLIDAR A1M8 | `/dev/alfred_lidar` | CP210x (10c4:ea60) |
| EMEET C960 | `/dev/alfred_camera` | UVC (328f:006d) |

Rules live in `/etc/udev/rules.d/99-alfred.rules`.

---

## 🗂️ Repository Structure

```plaintext
Alfred1/
│
├── firmware/         # Arduino Nano sketch
├── ros2_ws/          # ROS 2 packages (alfred_bridge, alfred_i2c, alfred_bringup)
├── hardware/         # Wiring diagrams, PCB designs, electronics documentation
├── media/            # Photos, reference images, diagrams
├── tests/            # Standalone sensor test scripts (pre-ROS hardware checks)
└── README.md
```

---

## 🔗 Folder Links

- [Hardware Documentation](hardware/)
- [Media](media/)
- [Tests](tests/)

---

## 🧩 Software Architecture

### **ROS 2 Packages (built and verified)**

- **`alfred_bridge`** — Serial bridge to the Arduino Nano
  - Subscribes: `/cmd_vel`
  - Publishes: `/ultrasonic/{left,right,rear}/range`, `/encoders/counts`, `/alfred/nano_response`
- **`alfred_i2c`** — AMG8833 + MPU-6050 sensor node
  - Publishes: `/alfred/imu`, `/alfred/imu/temperature`, `/thermal/grid`, `/thermal/stats`, `/thermal/hotspot`
- **`alfred_bringup`** — Single launch file (`alfred.launch.py`) that starts the bridge, I2C node, camera, and LiDAR together
- **`rplidar_ros`** — Built from source (`ros2` branch) due to a segfault in the current apt package on Pi 5 ARM64; publishes `/scan`
- **`usb_cam`** — Camera driver; publishes `/image_raw` (calibrated, see `~/.ros/camera_info/default_cam.yaml`)

### **Arduino Nano Responsibilities**

- PWM motor control (`SET <left> <right>` serial command)
- Ultrasonic ranging (3× HC-SR04, 10 Hz)
- Quadrature encoder pulse counting (20 Hz report rate)
- Serial communication to Pi at 115200 baud

### **Topic Map (Current)**

| Topic | Type | Rate | Source |
| --- | --- | --- | --- |
| `/cmd_vel` | `Twist` | on demand | → `alfred_bridge` |
| `/ultrasonic/{left,right,rear}/range` | `Range` | 10 Hz | `alfred_bridge` |
| `/encoders/counts` | `Int64MultiArray` | 20 Hz | `alfred_bridge` |
| `/alfred/nano_response` | `String` | as needed | `alfred_bridge` |
| `/alfred/imu` | `Imu` | 50 Hz | `alfred_i2c` |
| `/alfred/imu/temperature` | `Temperature` | 50 Hz | `alfred_i2c` |
| `/thermal/grid` | `Float32MultiArray` | 10 Hz | `alfred_i2c` |
| `/thermal/stats` | `Float32MultiArray` | 10 Hz | `alfred_i2c` |
| `/thermal/hotspot` | `Float32MultiArray` | 10 Hz | `alfred_i2c` |
| `/scan` | `LaserScan` | 10 Hz | `rplidar_ros` |
| `/image_raw` | `Image` | 30 Hz | `usb_cam` |

### **Single-Command Launch**

```bash
ros2 launch alfred_bringup alfred.launch.py
```

---

## ✅ Verified Working

- [x] Nano firmware uploaded and running (motors, encoders, 3× ultrasonics)
- [x] `alfred_bridge` node — sonar, encoders, motor commands confirmed over serial
- [x] Motors confirmed driving via keyboard teleop (forward/reverse/turn)
- [x] `alfred_i2c` node — AMG8833 + MPU-6050 both publishing simultaneously
- [x] RPLIDAR A1M8 publishing `/scan` (built from source)
- [x] EMEET C960 camera publishing `/image_raw` at 1080p/30fps
- [x] Camera calibrated via `camera_calibration` (chessboard, 0.015m squares)
- [x] All three USB devices have permanent udev symlinks
- [x] `alfred_bringup` launches all four nodes with one command

---

## 📅 Roadmap

### **🔨 Hardware**
- [ ] Redesign and fabricate new chassis base (wider for battery, wider caster stance for stability)
- [ ] Mount all sensors and boards to final chassis
- [ ] Measure final wheel separation (`WHEEL_BASE`) and sensor mounting positions
- [ ] Battery + converter wiring (currently bench-powered)
- [ ] Cable management and routing (shorter USB cables for Nano/LiDAR)

### **🧭 Software**
- [ ] Robot description (URDF) — blocked on final chassis measurements
- [ ] Encoder-based odometry node
- [ ] SLAM using RPLIDAR
- [ ] Nav2 — autonomous navigation
- [ ] Update `WHEEL_BASE` constant in `alfred_bridge` once chassis is final

### **🎥 Media**
- [ ] Add build photos
- [ ] Add wiring diagrams
- [ ] Add early movement test videos

---

## 📝 Credits / Sources

- Amazon product images used only for documentation reference
- Images and screenshots stored in `/media/`
- Created by **AstroGear Labs** (https://astrogearlabs.com)
