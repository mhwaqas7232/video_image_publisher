#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "cv_bridge/cv_bridge.h"
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <chrono>
#include <sstream>

namespace fs = std::filesystem;

class VideoSubscriber : public rclcpp::Node {
public:
  VideoSubscriber() : Node("video_subscriber"), last_saved_time_(this->now()), frame_counter_(0) {
    subscription_ = this->create_subscription<sensor_msgs::msg::Image>(
      "video_frames", 10,
      std::bind(&VideoSubscriber::frame_callback, this, std::placeholders::_1));
    
    // Create a directory to save frames if it doesn't exist
    fs::create_directories("saved_frames");
  }

private:
  void frame_callback(const sensor_msgs::msg::Image::SharedPtr msg) {
    auto current_time = this->now();

    // Save frame every 5 seconds
    if ((current_time - last_saved_time_).seconds() >= 5.0) {
      cv_bridge::CvImagePtr cv_ptr;
      try {
        cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
      } catch (cv_bridge::Exception& e) {
        RCLCPP_ERROR(this->get_logger(), "cv_bridge exception: %s", e.what());
        return;
      }

      // Generate a unique filename using frame_counter_
      std::stringstream filename;
      filename << "saved_frames/frame_" << frame_counter_ << ".png";

      // Save the frame as a PNG file
      cv::imwrite(filename.str(), cv_ptr->image);

      RCLCPP_INFO(this->get_logger(), "Saved frame: %s", filename.str().c_str());

      // Update the last saved time and increment frame counter
      last_saved_time_ = current_time;
      frame_counter_++;
    }
  }

  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscription_;
  rclcpp::Time last_saved_time_;  // Track the last time a frame was saved
  int frame_counter_;  // Unique counter for each saved frame
};

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);
  auto node = std::make_shared<VideoSubscriber>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
