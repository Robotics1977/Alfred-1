# Alfred 1

Alfred 1 is an autonomous mobile robot built using a Raspberry Pi 5, ROS 2, custom motor control electronics, and multiple sensors including LIDAR and computer vision.  
This project is part of the AstroGear Labs robotics initiative.

---

## 📦 Current Hardware

### **Compute + OS**
- Raspberry Pi 5 (8GB)
- 512GB SSD (Ubuntu 24.04.3)
- ROS 2 (Jazzy)

### **Sensors**
- **RPLIDAR A1M8** (360° 2D LiDAR)
- **C960 camera** (1080p USB camera)
- **MPU-6050 6-axis IMU**
- **3× HC-SR04 ultrasonic sensors**

### **Motor System**
- **Two 12V JGA25-371 DC motors with encoders**  
  - 126 RPM @ 12V  
  - 12 counts per revolution encoder  
- **TB6612FNG dual-motor driver**

### **Power**
- **12.8V 12Ah LiFePO4 battery**
- **12V → 5V 10A buck converter**
- Separate 12V motor bus for motors

### **Mechanical**
- Caster wheels
- Modular top plate (planned)
- Protective bumper (planned)

---

## 🧠 Arduino Nano (Fixed Pin Layout)

This layout will **never be changed**, as required for Alfred 1.

| Pin | Device |
| --- | ------------------------------- |
| D2  | HC-SR04 #1 Trigger |
| D3  | HC-SR04 #1 Echo |
| D4  | HC-SR04 #2 Trigger |
| D5  | HC-SR04 #2 Echo |
| D6  | HC-SR04 #3 Trigger |
| D7  | HC-SR04 #3 Echo |
| D8  | Motor A IN1 |
| D9  | Motor A IN2 |
| D10 | Motor A PWM |
| D11 | Motor B IN1 |
| D12 | Motor B IN2 |
| D13 | Motor B PWM |
| A0  | Motor A Encoder A |
| A1  | Motor A Encoder B |
| A2  | Motor B Encoder A |
| A3  | Motor B Encoder B |
| A4  | TB6612FNG STBY |

---

## 🗂️ Repository Structure

```plaintext
Alfred1/
│
├──firmware/         # MCU / Pi firmware, configs, ROS nodes, etc.
├──hardware/         # Wiring diagrams, electronics documentation
├── media/            # Photos, reference images, diagrams
├── tests/            # Sensor + motor test code
└── README.md
```



---

## 🔗 Folder Links
- [Hardware Documentation](hardware/)
- [Media](media/)
- [Tests](tests/)

---

## 🧩 Software Architecture (Planned)

### **ROS 2 Nodes**
- `motor_controller` — Wheel speed control + encoder feedback  
- `lidar_node` — Laser scan publisher  
- `camera_node` — USB camera feed  
- `navigation_node` — Mapping + path planning  
- `telemetry_node` — Diagnostics + system monitoring  

### **Arduino Nano Responsibilities**
- PWM motor control  
- Ultrasonic ranging  
- Encoder pulse counting  
- I2C IMU readings  
- Serial communications to Pi  

---

## 📅 Roadmap

### **🔨 Hardware**
- [ ] Platform weight/load testing  
- [ ] Permanent chassis mounting  
- [ ] Battery + converter wiring  
- [ ] Cable management and routing  

### **🧠 Electronics**
- [ ] Integrate IMU (MPU-6050) into ROS  
- [ ] Add ultrasonic sensor readings to ROS  
- [ ] Nano ↔ Pi serial communication  
- [ ] Encoder-based odometry  

### **🧭 Software**
- [ ] ROS 2 motor control node  
- [ ] SLAM using RPLIDAR  
- [ ] Basic autonomous navigation  
- [ ] Obstacle avoidance  
- [ ] Teleop via keyboard/controller  

### **🎥 Media**
- [ ] Add build photos  
- [ ] Add wiring diagrams  
- [ ] Add early movement test videos  

---

## 📝 Credits / Sources

- Amazon product images used only for documentation reference  
- Images and screenshots stored in `/media/`  
- Created by **AstroGear Labs** (https://astrogearlabs.com)
