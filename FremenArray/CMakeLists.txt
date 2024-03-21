cmake_minimum_required(VERSION 3.5)
project(fremenarray)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

# Find packages in ROS 2
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(action_msgs REQUIRED) # or actionlib_msgs, depending on your action definition
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)

# Add message and action dependencies
find_package(rosidl_default_generators REQUIRED)
rosidl_generate_interfaces(${PROJECT_NAME}
  action/FremenArray.action
  DEPENDENCIES action_msgs
)

# Define the executable
add_executable(fremenarray src/fremenarray.cpp)

# Include directories
target_include_directories(fremenarray PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

# Link against required ROS 2 libraries
ament_target_dependencies(fremenarray
  rclcpp
  action_msgs
  geometry_msgs
  std_msgs
  tf2
  tf2_geometry_msgs
  visualization_msgs
)

# Install executable
install(TARGETS
  fremenarray
  DESTINATION lib/${PROJECT_NAME})

# Install headers
install(DIRECTORY include/
  DESTINATION include)

ament_package()
