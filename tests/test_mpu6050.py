#!/usr/bin/env python3
import board
import busio
import adafruit_mpu6050
import time

i2c = busio.I2C(board.SCL, board.SDA)
mpu = adafruit_mpu6050.MPU6050(i2c)

print("MPU-6050 connected!\n")

try:
    while True:
        ax, ay, az = mpu.acceleration
        gx, gy, gz = mpu.gyro
        temp = mpu.temperature
        print(f"Accel  X:{ax:7.3f}  Y:{ay:7.3f}  Z:{az:7.3f} m/s²")
        print(f"Gyro   X:{gx:7.3f}  Y:{gy:7.3f}  Z:{gz:7.3f} rad/s")
        print(f"Temp   {temp:.1f}°C  ({(temp * 9/5) + 32:.1f}°F)\n")
        time.sleep(0.5)
except KeyboardInterrupt:
    print("Done.")
