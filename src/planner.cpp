// begin with code that highlights all "frontier" cells so you can see it in rviz2. This is a good first step
// use visualization_msgs::msg::Marker or visualization_msgs::msg::MarkerArray

// then write the algorithm that chooses the best one to explore next, which then outputs this as a goal pose that
//you can somehow see in rviz2???

// then finally actually feed that as an input to nav2


// INPUTS (sub): occupancy grid, TF for base link
// OUTPUTS (pub): first markers, then eventually a goal pose