This is a ROS2 package for a frontier-based exploration algorithm that I've written in C++. 

How it works:
- First, the occupancy grid is extracted from the current `/map`, and all frontier cells are detected. The working definition of a frontier is a cell that is unoccupied and has at least one unexplored cell next to it (four cardinal directions).
- Then the frontiers are grouped into clusters. If a frontier is next to another frontier (eight cardinal directions, so this time using diagonals), then they join the same cluster.
- A cost is calculated for the central cell of each cluster. The formula $C = \alpha (\text{size of cluster}) - \beta (\text{distance from robot to frontier})$ is used.
- The cluster with the lowest cost is picked, and the vector with position as {central cell of cluster} and direction as {direction from robot to central cell of cluster} is sent as a goal pose to nav2. 




Here's a first test of the algorithm working using `test.launch.py`, where `publisher.cpp` generates an artificial occupancy grid. You can see the blue markers on each frontier cell.
<img width="1851" height="1047" alt="image" src="https://github.com/user-attachments/assets/79f18e4d-d373-4365-8412-dac0f3933926" />
