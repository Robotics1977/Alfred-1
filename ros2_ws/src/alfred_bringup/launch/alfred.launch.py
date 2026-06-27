from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
import os


def generate_launch_description():

    # RPLidar launch file
    rplidar_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory('rplidar_ros'),
                'launch',
                'rplidar_a1_launch.py'
            )
        ),
        launch_arguments={'serial_port': '/dev/alfred_lidar'}.items()
    )

    # Alfred bridge node (Nano serial)
    bridge_node = Node(
        package='alfred_bridge',
        executable='alfred_bridge_node',
        name='alfred_bridge',
        output='screen'
    )

    # Alfred I2C node (AMG8833 + MPU-6050)
    i2c_node = Node(
        package='alfred_i2c',
        executable='alfred_i2c_node',
        name='alfred_i2c',
        output='screen'
    )

    # Camera node
    camera_node = Node(
        package='usb_cam',
        executable='usb_cam_node_exe',
        name='alfred_camera',
        output='screen',
        parameters=[{
            'video_device': '/dev/video0',
            'pixel_format': 'mjpeg2rgb',
            'image_width': 1920,
            'image_height': 1080,
            'framerate': 30.0,
        }]
    )

    return LaunchDescription([
        bridge_node,
        i2c_node,
        camera_node,
        rplidar_launch,
    ])
