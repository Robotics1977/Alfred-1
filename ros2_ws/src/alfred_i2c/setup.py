from setuptools import find_packages, setup

package_name = 'alfred_i2c'

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
    description='Alfred 1 I2C sensors node',
    license='MIT',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
            'alfred_i2c_node = alfred_i2c.alfred_i2c_node:main',
        ],
    },
)
