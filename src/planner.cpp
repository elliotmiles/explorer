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

class ExplorerNode : public rclcpp::Node {
    public:
        ExplorerNode() : Node("explorer") {
            subscriber_ = this->create_subscription<nav_msgs::msg::OccupancyGrid>(
                "/map", 10, std::bind(&ExplorerNode::callback, this, std::placeholders::_1));

            publisher_ = this->create_publisher<visualization_msgs::msg::MarkerArray>("/frontiers", 10);
        }



    private:
        void callback(nav_msgs::msg::OccupancyGrid::SharedPtr msg) {
            nav_msgs::msg::OccupancyGrid frontier_arr = detect_frontiers(msg);
            std::vector<uint8_t> coords_array = parse_data(frontier_arr);

            visualization_msgs::msg::MarkerArray marker_array;

            for (int i = 0; i < frontier_arr.size(); i++) {

                visualization_msgs::msg::Marker marker;

                marker.header.frame_id = "map";
                marker.header.stamp = this->get_clock->now();

                marker.ns = "frontiers";
                marker.id = i;

                marker.type = visualization_msgs::msg::Marker::SPHERE;
                marker.action = visualization_msgs::msg::Marker::ADD;

                marker.pose.position.x = frontier_arr[i][0];
                marker.pose.position.y = frontier_arr[i][1];
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
        rclcpp::Publisher<nav_msgs::msg::OccupancyGrid>::SharedPtr publisher_;
};


// inputs the data, should output a vector of arrays with the coords and the status of the cell
std::vector<uint8_t> detect(&frontier_arr) {
    std::vector<uint8_t> data = {};

    for (int i = 0; i < frontier_arr->data.size(); i++) {
        
        if (frontier_arr->data[i] == 2) {
            int y = i / frontier_arr->info.width;
            int x = i % frontier_arr->info.width;

            int coords_array[] = {x, y, frontier_arr->data[i]};

            data.push_back(coords_array);
        }
    }

    return data;
}

bool check_east(const nav_msgs::msg::OccupancyGrid& msg, int i) {
    if (msg->data[i+1] == -1) {
        return True;
    }
    else return False;
}

bool check_south(const nav_msgs::msg::OccupancyGrid& msg, int i) {
    if (msg->data[i - (msg->info.width)] == -1) {
        return True;
    }
    else return False;
}

bool check_west(const nav_msgs::msg::OccupancyGrid& msg, int i) {
    if (msg->data[i-1] == -1) {
        return True;
    }
    else return False;
}

bool check_north(const nav_msgs::msg::OccupancyGrid& msg, int i) {
    if (msg->data[i + (msg->info.width)] == -1) {
        return True;
    }
    else return False;
}



// FRONTIER = 2, NOT FRONTIER = -2
nav_msgs::msg::OccupancyGrid detect_frontiers(&msg) {
    for(int i = 0; i < msg->data.size(); i++) {
        if (msg->data[i] == 1 || msg->data[i] == -1) {
            msg->data[i] = -2;
            continue;
        }

        // all past this point are unoccupied cells 
        
        // bottom left corner
        if (i == 0) {
            if (msg->data[i] == 0 && (check_north(msg, i) == True || check_east(msg, i) == True)) {
                msg->data[i] = 2;
                continue;
            }
            else msg->data[i] = -2;
            continue;
        }

        // bottom right corner
        if (i == (msg->info.width - 1)) {
            if (msg->data[i] == 0 && (check_north(msg, i) == True || check_west(msg, i) == True)) {
                msg->data[i] = 2;
                continue;
            }
            else msg->data[i] = -2;
            continue;
        }

        // top right corner
        if (i == (msg->data.size() - 1)) {
            if (msg->data[i] == 0 && (check_south(msg, i) == True || check_west(msg, i) == True)) {
                msg->data[i] = 2;
                continue;
            }
            else msg->data[i] = -2;
            continue;
        }

        // top left corner
        if (i == msg->data.size() - (msg->info.width)) {
            if (msg->data[i] == 0 && (check_south(msg, i) == True || check_east(msg, i) == True)) {
                msg->data[i] = 2;
                continue;
            }
            else msg->data[i] = -2;
            continue;
        }

        // bottom row
        if (i < msg->info.width) {
            if (msg->data[i] == 0 && (check_west(msg, i) || check_north(msg, i) || check_east(msg, i) == True)) {
                msg->data[i] = 2;
                continue;
            }
            else msg->data[i] = -2;
            continue;
        }

        // top row
        if (i > msg->data.size() - msg) {
            if (msg->data[i] == 0 && (check_west(msg, i) || check_south(msg, i) || check_east(msg, i) == True)) {
                msg->data[i] = 2;
                continue;
            }
            else msg->data[i] = -2; 
            continue;
        }

        // left column
        if (i & msg->info.width == 0) {
            if (msg->data[i] == 0 && (check_west(msg, i) || check_north(msg, i) || check_east(msg, i) == True)) {
                msg->data[i] = 2;
                continue;
            }
            else msg->data[i] = -2;
            continue;
        }

        // right column
        if ((i-1) % msg->info.width == 0) {
            if (msg->data[i] == 0 && (check_west(msg, i) || check_north(msg, i) || check_east(msg, i) == True)) {
                msg->data[i] = 2;
                continue;
            }
            else msg->data[i] = -2;
            continue;
        }

        // central cells (north south east and west all available)
        else {
            if (msg->data[i] == 0 && (check_west(msg, i) || check_north(msg, i) || check_east(msg, i) == True || check_south(msg, i) == True)) {
                msg->data[i] = 2;
                continue;
            }
            else msg->data[i] = -2;
            continue;
        }

    }
}



int main(int argc, char *argv[]) {

    rclcpp::init(argc, argv);
    auto node = std::make_shared<ExplorerNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();

    return 0;
}