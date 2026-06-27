import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from sensor_msgs.msg import Range
from std_msgs.msg import String, Int64MultiArray
import serial
import threading


# --- Serial Configuration ---
SERIAL_PORT = '/dev/alfred_nano'
BAUD_RATE   = 115200

# --- Alfred Physical Parameters ---
WHEEL_BASE  = 0.20   # Meters, center of left wheel to center of right wheel

# --- Motor Configuration ---
MAX_SPEED   = 255
DEADBAND    = 10

# --- Safety ---
CMD_TIMEOUT = 1.0    # Seconds before auto-stop if no /cmd_vel received

# --- Sonar Configuration ---
SONAR_CONFIG = {
    'rear':  ('ultrasonic_rear_link',  0.26, 0.02, 4.0),
    'left':  ('ultrasonic_left_link',  0.26, 0.02, 4.0),
    'right': ('ultrasonic_right_link', 0.26, 0.02, 4.0),
}


class AlfredBridgeNode(Node):
    def __init__(self):
        super().__init__('alfred_bridge')

        # ------------------------------------------------------------------ #
        #  Publishers                                                          #
        # ------------------------------------------------------------------ #

        self.sonar_pubs = {
            name: self.create_publisher(Range, f'/ultrasonic/{name}/range', 10)
            for name in SONAR_CONFIG
        }

        self.encoder_pub = self.create_publisher(
            Int64MultiArray, '/encoders/counts', 10
        )

        self.response_pub = self.create_publisher(
            String, '/alfred/nano_response', 10
        )

        # ------------------------------------------------------------------ #
        #  Subscribers                                                         #
        # ------------------------------------------------------------------ #

        self.cmd_sub = self.create_subscription(
            Twist, '/cmd_vel', self.cmd_vel_callback, 10
        )

        # ------------------------------------------------------------------ #
        #  Serial connection                                                   #
        # ------------------------------------------------------------------ #
        try:
            self.ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
            self.serial_lock = threading.Lock()
            self.get_logger().info(f'Connected to Alfred Nano on {SERIAL_PORT}')
        except serial.SerialException as e:
            self.get_logger().error(f'Failed to open serial port: {e}')
            raise

        # ------------------------------------------------------------------ #
        #  Safety timeout timer                                                #
        # ------------------------------------------------------------------ #
        self.last_cmd_time = self.get_clock().now()
        self.timeout_timer = self.create_timer(0.1, self.check_timeout)

        # ------------------------------------------------------------------ #
        #  Background serial read thread                                       #
        # ------------------------------------------------------------------ #
        self.running = True
        self.read_thread = threading.Thread(target=self.read_loop, daemon=True)
        self.read_thread.start()

        self.get_logger().info('Alfred bridge node ready.')
        self.get_logger().info('Subscribed to: /cmd_vel')
        self.get_logger().info('Publishing:    /ultrasonic/{left,right,rear}/range')
        self.get_logger().info('Publishing:    /encoders/counts')
        self.get_logger().info('Publishing:    /alfred/nano_response')

    # ====================================================================== #
    #  SERIAL READ LOOP (background thread)                                   #
    # ====================================================================== #

    def read_loop(self):
        while self.running:
            try:
                with self.serial_lock:
                    raw = self.ser.readline()

                line = raw.decode('utf-8', errors='replace').strip()
                if not line:
                    continue

                if line.startswith('SONAR'):
                    self.handle_sonar(line)
                elif line.startswith('ENC'):
                    self.handle_encoder(line)
                else:
                    self.handle_response(line)

            except Exception as e:
                self.get_logger().warn(f'Serial read error: {e}')

    # ====================================================================== #
    #  INCOMING DATA HANDLERS                                                 #
    # ====================================================================== #

    def handle_sonar(self, line: str):
        parts = line.split()
        if len(parts) != 4:
            self.get_logger().warn(f'Unexpected SONAR format: {line}')
            return

        try:
            readings_cm = {
                'rear':  int(parts[1]),
                'left':  int(parts[2]),
                'right': int(parts[3]),
            }
        except ValueError:
            self.get_logger().warn(f'Could not parse SONAR values: {line}')
            return

        now = self.get_clock().now().to_msg()

        for name, cm in readings_cm.items():
            frame_id, fov, min_r, max_r = SONAR_CONFIG[name]

            msg = Range()
            msg.header.stamp    = now
            msg.header.frame_id = frame_id
            msg.radiation_type  = Range.ULTRASOUND
            msg.field_of_view   = fov
            msg.min_range       = min_r
            msg.max_range       = max_r
            msg.range = max_r if cm <= 0 else cm / 100.0

            self.sonar_pubs[name].publish(msg)

        self.get_logger().debug(
            f'Sonar rear={readings_cm["rear"]}cm '
            f'left={readings_cm["left"]}cm '
            f'right={readings_cm["right"]}cm'
        )

    def handle_encoder(self, line: str):
        parts = line.split()
        if len(parts) != 3:
            self.get_logger().warn(f'Unexpected ENC format: {line}')
            return

        try:
            left_count  = int(parts[1])
            right_count = int(parts[2])
        except ValueError:
            self.get_logger().warn(f'Could not parse ENC values: {line}')
            return

        msg = Int64MultiArray()
        msg.data = [left_count, right_count]
        self.encoder_pub.publish(msg)

        self.get_logger().debug(f'Encoders L={left_count} R={right_count}')

    def handle_response(self, line: str):
        msg = String()
        msg.data = line
        self.response_pub.publish(msg)
        self.get_logger().debug(f'Nano: {line}')

    # ====================================================================== #
    #  OUTGOING COMMAND HANDLERS                                              #
    # ====================================================================== #

    def cmd_vel_callback(self, msg: Twist):
        self.last_cmd_time = self.get_clock().now()

        linear  = msg.linear.x
        angular = msg.angular.z

        left_vel  = linear - (angular * WHEEL_BASE / 2.0)
        right_vel = linear + (angular * WHEEL_BASE / 2.0)

        max_vel   = max(abs(left_vel), abs(right_vel), 1.0)
        left_pwm  = int((left_vel  / max_vel) * MAX_SPEED)
        right_pwm = int((right_vel / max_vel) * MAX_SPEED)

        left_pwm  = max(-MAX_SPEED, min(MAX_SPEED, left_pwm))
        right_pwm = max(-MAX_SPEED, min(MAX_SPEED, right_pwm))

        self.send_command(f'SET {left_pwm} {right_pwm}')

    def check_timeout(self):
        elapsed = (self.get_clock().now() - self.last_cmd_time).nanoseconds / 1e9
        if elapsed > CMD_TIMEOUT:
            self.send_command('STOP')

    def send_command(self, cmd: str):
        try:
            with self.serial_lock:
                self.ser.write(f'{cmd}\n'.encode('utf-8'))
            self.get_logger().debug(f'Sent: {cmd}')
        except serial.SerialException as e:
            self.get_logger().error(f'Serial write error: {e}')

    # ====================================================================== #
    #  CLEANUP                                                                #
    # ====================================================================== #

    def destroy_node(self):
        self.running = False
        self.send_command('STOP')
        if self.ser.is_open:
            self.ser.close()
        super().destroy_node()


def main(args=None):
    rclpy.init(args=args)
    try:
        node = AlfredBridgeNode()
    except serial.SerialException:
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
