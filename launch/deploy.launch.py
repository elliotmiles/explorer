# deployment of planner.cpp with an external OG (do not use publisher.cpp)

from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():

    planner_node = Node(
        package='explorer',
        executable='planner',
        name='planner'
    )


    return LaunchDescription([
        planner_node
    ])