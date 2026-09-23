// begin with code that highlights all "frontier" cells so you can see it in rviz2. This is a good first step
// use visualization_msgs::msg::Marker or visualization_msgs::msg::MarkerArray

// then write the algorithm that chooses the best one to explore next, which then outputs this as a goal pose that
//you can somehow see in rviz2???

// then finally actually feed that as an input to nav2


// INPUTS (sub): occupancy grid, TF for base link
// OUTPUTS (pub): first markers, then eventually a goal pose

#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <visualization_msgs/msg/marker.hpp>
#include <visualization_msgs/msg/marker_array.hpp>

#include <vector>

struct GridCell {
    int x;
    int y;
};


GridCell coords(int width, int i) {
    int y = i / width;
    int x = i % width;

    GridCell coords = {x, y};
    return coords;
}

bool check_east(const nav_msgs::msg::OccupancyGrid& msg, int i) {
    if (msg.data[i+1] == -1) {
        return true;
    }
    else return false;
}

bool check_south(const nav_msgs::msg::OccupancyGrid& msg, int i) {
    if (msg.data[i - (msg.info.width)] == -1) {
        return true;
    }
    else return false;
}

bool check_west(const nav_msgs::msg::OccupancyGrid& msg, int i) {
    if (msg.data[i-1] == -1) {
        return true;
    }
    else return false;
}

bool check_north(const nav_msgs::msg::OccupancyGrid& msg, int i) {
    if (msg.data[i + (msg.info.width)] == -1) {
        return true;
    }
    else return false;
}



// takes in the OccGrid msg, outputs a vector containing the coords of all "frontier" cells
//where "frontier" means the cell is unoccupied and at least one of its neighbours is unexplored
std::vector<GridCell> detect_frontiers(const nav_msgs::msg::OccupancyGrid& msg) {

    std::vector<GridCell> frontiers_arr; 

    for(int i = 0; i < msg.data.size(); i++) {
        if (msg.data[i] != 0) {
            continue;
        }

        // all past this point are unoccupied cells 
        
        // bottom left corner
        if (i == 0) {
            if (msg.data[i] == 0 && (check_north(msg, i) == true || check_east(msg, i) == true)) {
                frontiers_arr.push_back(coords(msg.info.width, i));
                continue;
            }
            else continue;
        }

        // bottom right corner
        if (i == (msg.info.width - 1)) {
            if (msg.data[i] == 0 && (check_north(msg, i) == true || check_west(msg, i) == true)) {
                frontiers_arr.push_back(coords(msg.info.width, i));
                continue;
            }
            else continue;
        }

        // top right corner
        if (i == (msg.data.size() - 1)) {
            if (msg.data[i] == 0 && (check_south(msg, i) == true || check_west(msg, i) == true)) {
                frontiers_arr.push_back(coords(msg.info.width, i));
                continue;
            }
            else continue;
        }

        // top left corner
        if (i == msg.data.size() - (msg.info.width)) {
            if (msg.data[i] == 0 && (check_south(msg, i) == true || check_east(msg, i) == true)) {
                frontiers_arr.push_back(coords(msg.info.width, i));
                continue;
            }
            else continue;
        }

        // bottom row
        if (i < msg.info.width) {
            if (msg.data[i] == 0 && (check_west(msg, i) || check_north(msg, i) || check_east(msg, i) == true)) {
                frontiers_arr.push_back(coords(msg.info.width, i));
                continue;
            }
            else continue;
        }

        // top row
        if (i > (msg.data.size() - msg.info.width - 1)) {
            if (msg.data[i] == 0 && (check_west(msg, i) || check_south(msg, i) || check_east(msg, i) == true)) {
                frontiers_arr.push_back(coords(msg.info.width, i));
                continue;
            }
            else continue;
        }

        // left column
        if (i % msg.info.width == 0) {
            if (msg.data[i] == 0 && (check_west(msg, i) || check_north(msg, i) || check_east(msg, i) == true)) {
                frontiers_arr.push_back(coords(msg.info.width, i));
                continue;
            }
            else continue;
        }

        // right column
        if ((i+1) % msg.info.width == 0) {
            if (msg.data[i] == 0 && (check_west(msg, i) || check_north(msg, i) || check_east(msg, i) == true)) {
                frontiers_arr.push_back(coords(msg.info.width, i));
                continue;
            }
            else continue;
        }

        // central cells (north south east and west all available)
        else {
            if (msg.data[i] == 0 && (check_west(msg, i) || check_north(msg, i) || check_east(msg, i) == true || check_south(msg, i) == true)) {
                frontiers_arr.push_back(coords(msg.info.width, i));
                continue;
            }
            else continue;
        }

    }

    return frontiers_arr;
}

class ExplorerNode : public rclcpp::Node {
    public:
        ExplorerNode() : Node("explorer") {
            subscriber_ = this->create_subscription<nav_msgs::msg::OccupancyGrid>(
                "/map", 10, std::bind(&ExplorerNode::callback, this, std::placeholders::_1));

            publisher_ = this->create_publisher<visualization_msgs::msg::MarkerArray>("/frontiers", 10);
        }



    private:
        void callback(nav_msgs::msg::OccupancyGrid::SharedPtr msg) {
            std::vector<GridCell> frontiers_arr = detect_frontiers(*msg); // dereference because msg is a SharedPtr

            visualization_msgs::msg::MarkerArray marker_array;

            for (int i = 0; i < frontiers_arr.size(); i++) {

                visualization_msgs::msg::Marker marker;

                marker.header.frame_id = "map";
                marker.header.stamp = this->get_clock()->now();

                marker.ns = "frontiers";
                marker.id = i;

                marker.type = visualization_msgs::msg::Marker::SPHERE;
                marker.action = visualization_msgs::msg::Marker::ADD;

                marker.pose.position.x = frontiers_arr[i].x;
                marker.pose.position.y = frontiers_arr[i].y;
                marker.pose.position.z = 0.0;

                marker.pose.orientation.w = 1.0;

                marker.scale.x = 0.2;
                marker.scale.y = 0.2;
                marker.scale.z = 0.2;

                marker.color.r = 1.0;
                marker.color.g = 0.0;
                marker.color.b = 0.0;
                marker.color.a = 1.0;

                marker_array.markers.push_back(marker);
            }

            publisher_->publish(marker_array);


        }


        rclcpp::Subscription<nav_msgs::msg::OccupancyGrid>::SharedPtr subscriber_;
        rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr publisher_;
};

int main(int argc, char *argv[]) {

    rclcpp::init(argc, argv);
    auto node = std::make_shared<ExplorerNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();

    return 0;
}