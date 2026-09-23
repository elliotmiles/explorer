// this should just publish occupancy grid data 

// INPUTS (sub): /map
// OUTPUTS (pub): /occupancy_grid_custom (of standard occupancy grid type)


#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>

#include <vector>
#include <array>
#include <cmath>

using namespace std::chrono_literals;

class GridPublisherNode : public rclcpp::Node {
    public:
        GridPublisherNode() : Node("occupancy_grid_publisher") {
            publisher_ = this->create_publisher<nav_msgs::msg::OccupancyGrid>("/map", 10);
            timer_ = this->create_wall_timer(500ms, std::bind(&GridPublisherNode::og_callback, this));
        }

    private:
        void og_callback() {
            auto occupancy_grid_msg = nav_msgs::msg::OccupancyGrid();

            occupancy_grid_msg.header.stamp = rclcpp::Clock().now();
            occupancy_grid_msg.header.frame_id = "map_frame";

            occupancy_grid_msg.info.resolution = 1;

            occupancy_grid_msg.info.width = 3;
            occupancy_grid_msg.info.height = 3;

            occupancy_grid_msg.info.origin.position.x = 0.0;
            occupancy_grid_msg.info.origin.position.y = 0.0;
            occupancy_grid_msg.info.origin.position.z = 0.0;
            occupancy_grid_msg.info.origin.orientation.x = 0.0;
            occupancy_grid_msg.info.origin.orientation.y = 0.0;
            occupancy_grid_msg.info.origin.orientation.z = 0.0;
            occupancy_grid_msg.info.origin.orientation.w = 1.0;
            occupancy_grid_msg.data = {100, 0, 0, 0, -1, 0, 0, 0, 100};

            publisher_->publish(occupancy_grid_msg);
        }


    rclcpp::Publisher<nav_msgs::msg::OccupancyGrid>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;    
};

int main(int argc, char *argv[]) {

    rclcpp::init(argc, argv);
    auto node = std::make_shared<GridPublisherNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();

    return 0;
}