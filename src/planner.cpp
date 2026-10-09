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

// for listening to the tf data
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2/exceptions.h>

#include <vector>
#include <cmath>

struct GridCell {
    int id;
    int x;
    int y;
};

struct WorldPoint {
    double x;
    double y;
};

GridCell coords(int id, int width, int i) {
    int y = i / width;
    int x = i % width;

    GridCell coords = {id, x, y};
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

// takes the vector of frontier gridcells as an input, and returns a vector of vector of gridcells. Each of these vectors represents a cluster.
std::vector<std::vector<GridCell>> group_clusters(std::vector<GridCell>& frontiers_arr) {
    std::vector<std::vector<GridCell>> clusters_arr;

    for (std::size_t i = 0; i < frontiers_arr.size(); i++) {
        
        // if (this cell is already in clusters_arr) {continue}
        for (std::size_t j = 0; j < clusters_arr.size(); j++) {
            for (std::size_t k = 0; k < clusters_arr[j].size(); k++) {
                if (frontiers_arr[i].id == clusters_arr[j][k].id) {continue;}
            }
        }

        // this cell is not already in clusters_arr, so check all 8 neighbours to determine whether they are a frontier
        for (std::size_t j = 0; j < frontiers_arr.size(); j++) {
            if (frontiers_arr[j].x == (frontiers_arr[i].x + 1) || // east
                frontiers_arr[j].y == (frontiers_arr[i].y + 1) || // north
                frontiers_arr[j].x == (frontiers_arr[i].x - 1) || // west
                frontiers_arr[j].y == (frontiers_arr[i].y - 1) || // south
                (frontiers_arr[j].x == (frontiers_arr[i].x + 1)) && (frontiers_arr[j].y == (frontiers_arr[i].y + 1)) || // north east
                (frontiers_arr[j].x == (frontiers_arr[i].x - 1)) && (frontiers_arr[j].y == (frontiers_arr[i].y + 1)) || // north west
                (frontiers_arr[j].x == (frontiers_arr[i].x - 1)) && (frontiers_arr[j].y == (frontiers_arr[i].y - 1)) || // south west
                (frontiers_arr[j].x == (frontiers_arr[i].x + 1)) && (frontiers_arr[j].y == (frontiers_arr[i].y - 1)) // south east
                ) {

                
                // check if the neighbour is already in clusters_arr
                for (std::size_t k = 0; k < clusters_arr.size(); k++) {
                    for (std::size_t l = 0; l < clusters_arr[k].size(); l++) {
                        if (frontiers_arr[j].id == clusters_arr[k][l].id) {
                            // the neighbour already is in clusters_arr, so add the current cell to the existing cluster
                            clusters_arr[k].push_back(frontiers_arr[i]);
                            continue;
                        }
                    }
                }
                // neither cell is in clusters_arr, so add both to a new cluster
                clusters_arr.push_back({frontiers_arr[i], frontiers_arr[j]});
            }
        }
    }

    return clusters_arr;

}

std::vector<double> sort_x(std::vector<GridCell>& item_list) {
    std::vector<double> x_list;

    for(const auto& i : item_list) {
        x_list.push_back(item_list[i].x)
    }

    std::sort(x_list.begin(), x_list.end());

    return x_list;
}

std::vector<double> sort_y(std::vector<GridCell>& item_list) {
    std::vector<double> y_list;

    for(const auto& i : item_list) {
        y_list.push_back(item_list[i].y)
    }

    std::sort(y_list.begin(), y_list.end());

    return y_list;
}

// find the central cell for each cluster
std::vector<std::vector<GridCell, double>> find_centres(std::vector<std::vector<GridCell>>& clusters_arr) {

    std::vector<std::vector<GridCell, double>> cluster_centres;
    // for each cluster...
    for (std::size_t i = 0; i < clusters_arr.size(); i++) {
        
        GridCell central_cell;
        central_cell.id = i;

        x_list = sort_x(clusters_arr[i]);

        size_t num_x = x_list.size();

        if (num_x % 2 == 1) {
            // odd number of elements
            central_cell.x = x_list[num_x / 2];
        }
        else {
            // even number of elements
            central_cell.x = ((x_list[num_x / (2 - 1)] + x_list[num_x / 2]) / 2.0);
        }

        // find median Y
        y_list = sort_y(clusters_arr[i]);

        size_t num_y = y_list.size();

        if (num_y % 2 == 1) {
            // odd number of elements
            central_cell.y = y_list[num_y / 2];
        }
        else {
            // even number of elements
            central_cell.y = ((y_list[num_y / (2 - 1)] + y_list[num_y / 2]) / 2.0);
        }

        cluster_centres.push_back({central_cell, num_x + num_y});
    }

    return cluster_centres;
}

GridCell select_cluster(std::vector<std::vector<GridCell>>& cluster_centres) {
    double alpha = 0.5;
    double beta = 0.5;

    GridCell result;
    result.id = 0;
    std::vector<double> costs_list;

    for (const auto& i : cluster_centres) {

        // find distance from robot to cluster centre
        WorldPoint robot_pos = get_robot_position();

        double x_dist = std::abs(robot_pos.x - i[0].x)
        double y_dist = std::abs(robot_pos.y - i[0].y); 
        
        double distance = std::sqrt(std::pow(x_dist, 2) + std::pow(y_dist, 2));

        double cost = (alpha * i[1]) - (beta * distance);
        
        costs_list.push_back(cost);
    }

    std::vector<double> costs_sorted = std::sort(costs_list);

    double best = costs_sorted[0];

    for (const auto& j : costs_list) {
        if (costs_sorted[0] == costs_list[j]) {
            result.x = cluster_centres[0].x;
            result.y = cluster_centres[0].y;
        }
    }

    return result;
}

class ExplorerNode : public rclcpp::Node {
    public:
        ExplorerNode() : Node("explorer") {
            subscriber_ = this->create_subscription<nav_msgs::msg::OccupancyGrid>(
                "/map", 10, std::bind(&ExplorerNode::callback, this, std::placeholders::_1));

            publisher_ = this->create_publisher<visualization_msgs::msg::MarkerArray>("/frontiers", 10);

            tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());

            tf_listener_ = std::make_shared<tf_ros::TransformListener>(*tf_buffer_)
        }



    private:
        void callback(nav_msgs::msg::OccupancyGrid::SharedPtr msg) {

            // convert to int to avoid comparing uint with int
            int width = static_cast<int>(msg->info.width);
            int height = static_cast<int>(msg->info.height);

            std::vector<GridCell> frontiers_arr = detect_frontiers(*msg, width, height); // dereference because msg is a SharedPtr

            std::vector<std::vector<GridCell>> clusters_arr = group_clusters(frontiers_arr);

            std::vector<std::vector<GridCell, double>> cluster_centres = find_centres(clusters_arr);

            GridCell best_centre = select_cluster(cluster_centres);

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
        
        WorldPoint get_robot_position() {
            auto transform = tf_buffer_->lookupTransform("map", "base_link", tf2::TimePointZero);

            return {transform.transform.translation.x, transform.transform.translation.y};
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