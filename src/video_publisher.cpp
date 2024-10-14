#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "cv_bridge/cv_bridge.h"
#include <opencv2/opencv.hpp>

class VideoPublisher : public rclcpp::Node {
public:
  VideoPublisher() : Node("video_publisher") {
    publisher_ = this->create_publisher<sensor_msgs::msg::Image>("video_frames", 10);
    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(33),  // 30 FPS (1000ms/30 = ~33ms)
      std::bind(&VideoPublisher::publish_frame, this));

    // Open video file
    video_cap_.open("/root/video_image_publisher/src/testvid.mp4");
    if (!video_cap_.isOpened()) {
      RCLCPP_ERROR(this->get_logger(), "Could not open video file.");
      rclcpp::shutdown();
    }

    bridge_ = std::make_shared<cv_bridge::CvImage>();
    bridge_->encoding = sensor_msgs::image_encodings::BGR8;
  }

private:
  void publish_frame() {
    cv::Mat frame;
    video_cap_ >> frame;  // Capture frame-by-frame

    if (frame.empty()) {
      RCLCPP_INFO(this->get_logger(), "End of video reached.");
      rclcpp::shutdown();
    } else {
      // Convert the OpenCV image to a ROS message
      bridge_->image = frame;
      bridge_->header.stamp = this->get_clock()->now();
      publisher_->publish(*bridge_->toImageMsg());
    }
  }

  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  cv::VideoCapture video_cap_;
  std::shared_ptr<cv_bridge::CvImage> bridge_;
};

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);
  auto node = std::make_shared<VideoPublisher>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
