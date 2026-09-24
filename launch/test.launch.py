# test planner.cpp with artifical OG (using publisher.cpp)

from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():

    planner_node = Node(
        package='explorer',
        executable='planner',
        name='planner'
    )

    publisher_node = Node(
        package='explorer',
        executable='publisher',
        name='publisher'
    )

    return LaunchDescription([
        planner_node,
        publisher_node,
    ])