#!/usr/bin/env python3
import board
import busio
import adafruit_amg88xx
import time

i2c = busio.I2C(board.SCL, board.SDA)
sensor = adafruit_amg88xx.AMG88XX(i2c)

print("AMG8833 connected! Reading thermal data...\n")

try:
    while True:
        pixels = sensor.pixels
        print("Thermal Grid (°C):")
        for row in pixels:
            print("  " + "  ".join(f"{val:5.1f}" for val in row))
        flat = [val for row in pixels for val in row]
        print(f"\n  Min: {min(flat):.1f}°C  Max: {max(flat):.1f}°C  Avg: {sum(flat)/len(flat):.1f}°C\n")
        time.sleep(0.5)
except KeyboardInterrupt:
    print("Done.")
