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

struct WorldPoint {
    double x;
    double y;
};

GridCell coords(int width, int i) {
    int y = i / width;
    int x = i % width;

    GridCell coords = {x, y};
    return coords;
}

// takes in the local gridcell coords relative to map origin, then outputs coords in the /map frame
WorldPoint grid_to_world(GridCell& cell_coords, double resolution, double origin_x, double origin_y) {
    double world_x = origin_x + (cell_coords.x + 0.5) * resolution;
    double world_y = origin_y + (cell_coords.y + 0.5) * resolution;

    WorldPoint coords = {world_x, world_y};
    return coords;
}



// takes in the OccGrid msg, outputs a vector containing the coords of all "frontier" cells
//where "frontier" means the cell is unoccupied and at least one of its neighbours is unexplored
std::vector<GridCell> detect_frontiers(const nav_msgs::msg::OccupancyGrid& msg, int& width, int& height) {

    std::vector<GridCell> frontiers_arr; 

    for(std::size_t i = 0; i < msg.data.size(); i++) { // using std::size_t to avoid errors when comparing uint and int
        if (msg.data[i] != 0) {
            continue;
        }

        GridCell cell_coords = coords(width, i);
        // check below
        if ((cell_coords.y) > 0 && msg.data[i - width] == -1) {
            frontiers_arr.push_back(cell_coords);
        }
        // check above
        else if ((cell_coords.y) < (height - 1) && msg.data[i + width] == -1) {
            frontiers_arr.push_back(cell_coords);
        }
        // check left
        else if ((cell_coords.x) > 0 && msg.data[i - 1] == -1) {
            frontiers_arr.push_back(cell_coords);
        }
        // check right
        else if ((cell_coords.x) < (width - 1) && msg.data[i + 1] == -1) {
            frontiers_arr.push_back(cell_coords);
        }
    }

    return frontiers_arr;
}

std::vector<std::vector<GridCell>> group_clusters(std::vector<GridCell>& frontiers_arr) {
    std::vector<std::vector<GridCell>> clusters_arr;

    for (std::size_t i = 0; i < frontiers_arr.size(); i++) {
        for (std::size_t i = 0; i < clusters_arr.size)
    }


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

            // convert to int to avoid comparing uint with int
            int width = static_cast<int>(msg->info.width);
            int height = static_cast<int>(msg->info.height);

            std::vector<GridCell> frontiers_arr = detect_frontiers(*msg, width, height); // dereference because msg is a SharedPtr

            std::vector<std::vector<GridCell>> clusters_arr = group_clusters(frontiers_arr);

            visualization_msgs::msg::MarkerArray marker_array;

            for (std::size_t i = 0; i < frontiers_arr.size(); i++) { // using std::size_t to avoid errors when comparing uint and int

                visualization_msgs::msg::Marker marker;

                WorldPoint world_coords = grid_to_world(frontiers_arr[i], msg->info.resolution, msg->info.origin.position.x, msg->info.origin.position.y);

                marker.header.frame_id = msg->header.frame_id;
                marker.header.stamp = this->get_clock()->now();

                marker.ns = "frontiers";
                marker.id = i;

                marker.type = visualization_msgs::msg::Marker::SPHERE;
                marker.action = visualization_msgs::msg::Marker::ADD;

                marker.pose.position.x = world_coords.x;
                marker.pose.position.y = world_coords.y;
                marker.pose.position.z = 0.0;

                marker.pose.orientation.w = 1.0;

                marker.scale.x = 0.05;
                marker.scale.y = 0.05;
                marker.scale.z = 0.05;

                marker.color.r = 0.0;
                marker.color.g = 0.0;
                marker.color.b = 1.0;
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