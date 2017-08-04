#ifndef SLAM_CTOR_ROS_INIT_UTILS_H
#define SLAM_CTOR_ROS_INIT_UTILS_H

#include <string>
#include <memory>

#include <ros/ros.h>

#include "../core/states/world.h"

#include "topic_with_transform.h"
#include "pose_correction_tf_publisher.h"
#include "robot_pose_observers.h"
#include "occupancy_grid_publisher.h"

std::string get_string_param(const std::string &name,
                             const std::string &dflt_value) {
  std::string value;
  ros::param::param<std::string>(name, value, dflt_value);
  return value;
}

std::string tf_odom_frame_id() {
  return get_string_param("~ros/tf/odom_frame_id", "odom_combined");
}

std::string tf_map_frame_id() {
  return get_string_param("~ros/tf/map_frame_id", "map");
}

std::string tf_robot_pose_frame_id() {
  return get_string_param("~ros/tf/robot_pose_frame_id", "robot_pose");
}

bool is_async_correction() {
  bool async_correction;
  ros::param::param<bool>("~ros/tf/async_correction", async_correction, false);
  return async_correction;
}

// TODO: to scan filtering

bool init_skip_exceeding_lsr() {
  bool param_value;
  ros::param::param<bool>("~ros/skip_exceeding_lsr_vals", param_value, false);
  return param_value;
}

// TODO: move to IO

template <typename ObservT, typename MapT>
std::shared_ptr<PoseCorrectionTfPublisher<ObservT>>
create_pose_correction_tf_publisher(WorldObservable<MapT> *slam,
                                    TopicWithTransform<ObservT> *scan_prov) {
  auto pose_publisher = std::make_shared<PoseCorrectionTfPublisher<ObservT>>(
    tf_map_frame_id(), tf_odom_frame_id(), is_async_correction()
  );
  scan_prov->subscribe(pose_publisher);
  slam->subscribe_pose(pose_publisher);
  return pose_publisher;
}

template <typename MapT>
std::shared_ptr<RobotPoseTfPublisher>
create_robot_pose_tf_publisher(WorldObservable<MapT> *slam) {
  auto pose_publisher = std::make_shared<RobotPoseTfPublisher>(
    tf_map_frame_id(), tf_robot_pose_frame_id());
  slam->subscribe_pose(pose_publisher);
  return pose_publisher;
}

template <typename MapT>
std::shared_ptr<OccupancyGridPublisher<MapT>>
create_occupancy_grid_publisher(WorldObservable<MapT> *slam,
                                ros::NodeHandle nh,
                                double ros_map_publishing_rate) {
  auto map_publisher = std::make_shared<OccupancyGridPublisher<MapT>>(
    nh.advertise<nav_msgs::OccupancyGrid>("map", 5),
    tf_map_frame_id(), ros_map_publishing_rate);
  slam->subscribe_map(map_publisher);
  return map_publisher;
}

void init_constants_for_ros(double &ros_tf_buffer_size,
                            double &ros_map_rate,
                            int &ros_filter_queue,
                            int &ros_subscr_queue) {
  ros::param::param<double>("~ros/tf/buffer_duration",ros_tf_buffer_size, 5.0);
  ros::param::param<double>("~ros/rviz/map_publishing_rate", ros_map_rate, 5.0);
  ros::param::param<int>("~ros/filter_queue_size",ros_filter_queue, 1000);
  ros::param::param<int>("~ros/subscribers_queue_size",ros_subscr_queue, 1000);
}

#endif
