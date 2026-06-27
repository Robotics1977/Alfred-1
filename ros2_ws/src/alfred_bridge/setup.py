from setuptools import find_packages, setup

package_name = 'alfred_bridge'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='alfred',
    maintainer_email='alfred@todo.todo',
    description='Alfred 1 serial bridge node',
    license='MIT',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'alfred_bridge_node = alfred_bridge.alfred_bridge_node:main',
        ],
    },
)
