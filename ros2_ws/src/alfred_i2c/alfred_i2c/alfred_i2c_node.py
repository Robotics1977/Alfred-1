import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Imu, Temperature
from std_msgs.msg import Float32MultiArray
import board
import busio
import adafruit_mpu6050
import adafruit_amg88xx


PUBLISH_RATE_IMU     = 50.0   # Hz
PUBLISH_RATE_THERMAL = 10.0   # Hz


class AlfredI2CNode(Node):
    def __init__(self):
        super().__init__('alfred_i2c')

        # ------------------------------------------------------------------ #
        #  I2C bus                                                            #
        # ------------------------------------------------------------------ #
        self.i2c = busio.I2C(board.SCL, board.SDA)

        # ------------------------------------------------------------------ #
        #  MPU-6050 setup                                                     #
        # ------------------------------------------------------------------ #
        try:
            self.mpu = adafruit_mpu6050.MPU6050(self.i2c)
            self.get_logger().info('MPU-6050 connected at 0x68')
        except Exception as e:
            self.get_logger().error(f'MPU-6050 init failed: {e}')
            raise

        # ------------------------------------------------------------------ #
        #  AMG8833 setup                                                      #
        # ------------------------------------------------------------------ #
        try:
            self.amg = adafruit_amg88xx.AMG88XX(self.i2c)
            self.get_logger().info('AMG8833 connected at 0x69')
        except Exception as e:
            self.get_logger().error(f'AMG8833 init failed: {e}')
            raise

        # ------------------------------------------------------------------ #
        #  Publishers — MPU-6050                                              #
        # ------------------------------------------------------------------ #
        self.imu_pub = self.create_publisher(
            Imu, '/alfred/imu', 10
        )

        self.imu_temp_pub = self.create_publisher(
            Temperature, '/alfred/imu/temperature', 10
        )

        # ------------------------------------------------------------------ #
        #  Publishers — AMG8833                                               #
        # ------------------------------------------------------------------ #
        self.thermal_pub = self.create_publisher(
            Float32MultiArray, '/thermal/grid', 10
        )

        self.hotspot_pub = self.create_publisher(
            Float32MultiArray, '/thermal/hotspot', 10
        )

        self.thermal_stats_pub = self.create_publisher(
            Float32MultiArray, '/thermal/stats', 10
        )

        # ------------------------------------------------------------------ #
        #  Timers                                                             #
        # ------------------------------------------------------------------ #
        self.imu_timer = self.create_timer(
            1.0 / PUBLISH_RATE_IMU, self.publish_imu
        )
        self.thermal_timer = self.create_timer(
            1.0 / PUBLISH_RATE_THERMAL, self.publish_thermal
        )

        self.get_logger().info('Alfred I2C node ready.')
        self.get_logger().info(f'IMU publishing at {PUBLISH_RATE_IMU} Hz')
        self.get_logger().info(f'Thermal publishing at {PUBLISH_RATE_THERMAL} Hz')

    # ====================================================================== #
    #  MPU-6050 PUBLISHER                                                     #
    # ====================================================================== #

    def publish_imu(self):
        try:
            ax, ay, az = self.mpu.acceleration
            gx, gy, gz = self.mpu.gyro
            temp       = self.mpu.temperature
        except Exception as e:
            self.get_logger().warn(f'IMU read error: {e}')
            return

        now = self.get_clock().now().to_msg()

        # --- Imu message ---
        imu_msg = Imu()
        imu_msg.header.stamp    = now
        imu_msg.header.frame_id = 'imu_link'

        imu_msg.linear_acceleration.x = ax
        imu_msg.linear_acceleration.y = ay
        imu_msg.linear_acceleration.z = az

        imu_msg.angular_velocity.x = gx
        imu_msg.angular_velocity.y = gy
        imu_msg.angular_velocity.z = gz

        # Orientation unknown without fusion — mark covariance as -1
        imu_msg.orientation_covariance[0] = -1.0

        self.imu_pub.publish(imu_msg)

        # --- Temperature message ---
        temp_msg = Temperature()
        temp_msg.header.stamp    = now
        temp_msg.header.frame_id = 'imu_link'
        temp_msg.temperature     = temp
        temp_msg.variance        = 0.0

        self.imu_temp_pub.publish(temp_msg)

        self.get_logger().debug(
            f'IMU accel=({ax:.2f},{ay:.2f},{az:.2f}) '
            f'gyro=({gx:.2f},{gy:.2f},{gz:.2f})'
        )

    # ====================================================================== #
    #  AMG8833 PUBLISHER                                                      #
    # ====================================================================== #

    def publish_thermal(self):
        try:
            pixels = self.amg.pixels
        except Exception as e:
            self.get_logger().warn(f'Thermal read error: {e}')
            return

        flat = [val for row in pixels for val in row]

        # --- Raw grid ---
        grid_msg = Float32MultiArray()
        grid_msg.data = flat
        self.thermal_pub.publish(grid_msg)

        # --- Stats: [min, max, avg] ---
        t_min = min(flat)
        t_max = max(flat)
        t_avg = sum(flat) / len(flat)

        stats_msg = Float32MultiArray()
        stats_msg.data = [t_min, t_max, t_avg]
        self.thermal_stats_pub.publish(stats_msg)

        # --- Hotspot: [row, col, temperature] ---
        max_idx = flat.index(t_max)
        hot_row = max_idx // 8
        hot_col = max_idx  % 8

        hotspot_msg = Float32MultiArray()
        hotspot_msg.data = [float(hot_row), float(hot_col), t_max]
        self.hotspot_pub.publish(hotspot_msg)

        self.get_logger().debug(
            f'Thermal min={t_min:.1f} max={t_max:.1f} '
            f'hotspot=({hot_row},{hot_col}) {t_max:.1f}°C'
        )

    # ====================================================================== #
    #  CLEANUP                                                                #
    # ====================================================================== #

    def destroy_node(self):
        self.i2c.deinit()
        super().destroy_node()


def main(args=None):
    rclpy.init(args=args)
    try:
        node = AlfredI2CNode()
    except Exception:
        rclpy.shutdown()
        return

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
