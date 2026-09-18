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

}

int main() {
    return 0;
}