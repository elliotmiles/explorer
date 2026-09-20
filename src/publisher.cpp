// this should just publish occupancy grid data 

// INPUTS (sub): /map
// OUTPUTS (pub): /occupancy_grid_custom (of standard occupancy grid type)


#include <rclcpp/rclcpp.hpp>

#include <vector>
#include <array>
#include <cmath>

class GridPublisher : public rclcpp::Node {
    public:
        GridPublisher() : Node("occupancy_grid_publisher") {

        }

};

int main(int argc, char *argv[]) {
    // pass in L1, L2, L3, and target z height
    GridPublisher arm_init(240.0, 290.0, 150.0, 53.1);


    rclcpp::init(argc, argv);
    auto node = std::make_shared<IKnode>(arm_init);
    rclcpp::spin(node);
    rclcpp::shutdown();

    return 0;
}