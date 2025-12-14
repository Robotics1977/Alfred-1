# Alfred 1 — Hardware Components List

This document lists all the major hardware components used in the Alfred 1 robot prototype, including specifications, source links, images, and notes.

| Component | Specs / Description | Quantity | Image | Source | Notes |
|-----------|------------------|----------|-------|--------|-------|
| Raspberry Pi 5 | Ubuntu + ROS 2, SSD | 1 | ![Raspberry Pi](../media/raspberry_pi.png) | [Official Pi](https://www.raspberrypi.com/) | Main computing unit |
| RPLIDAR A1M8 | 360° scanning, 12m range | 1 | ![LIDAR](../media/lidar.png) | [Amazon Listing](https://www.amazon.com/example-link) | Obstacle detection / mapping |
| TB6612FNG | Dual motor driver, 12V | 1 | ![Motor Driver](../media/tb6612.png) | [Amazon Listing](https://www.amazon.com/example-link) | Controls 2 DC motors |
| 12V DC Motors w/ Encoders | 65mm, 500RPM | 2 | ![Motors](../media/motors.png) | [Amazon Listing](https://www.amazon.com/example-link) | Drive wheels |
| Caster Wheels | 1" swivel | 2 | ![Caster](../media/caster.png) | [Tractor Supply| Front stabilizing wheels |
| LiFePO4 Battery | 12.8V 12Ah | 1 | ![Battery](../media/battery.png) | [Amazon Listing](https://www.amazon.com/example-link) | Power source |
| 12V → 5V Converter | 20A | 1 | ![Converter](../media/converter.png) | [Amazon Listing](https://www.amazon.com/example-link) | Powers Pi and electronics |
| MPU 6050 | 1 | ![MPU6050](../media/mpu6050.png) | [Amazon Listing](https://www.amazon.com/example-link) | GPS |

**Notes:**
- All images are for reference purposes. Original sources are listed under the “Source” column.
- Dimensions and wiring diagrams are in `/media/`.
- This list will be updated as the project progresses.
