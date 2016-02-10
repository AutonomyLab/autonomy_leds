#ifndef AUTONOMY_LEDS_TOOLS_H
#define AUTONOMY_LEDS_TOOLS_H

#include <stdint.h>
#include <algorithm>

#include "autonomy_leds_msgs/Command.h"
#include "autonomy_leds_msgs/Keyframe.h"
#include "autonomy_leds_msgs/Animation.h"
#include "autonomy_leds_msgs/Feedback.h"

#include <ros/ros.h>
#include <ros/time.h>

namespace autonomy_leds
{

class BebopAnimator
{
protected:
  ros::NodeHandle nh_;

public:
  BebopAnimator(ros::NodeHandle& nh, const uint16_t num_leds);

  inline uint16_t GetNumLEDs() const {return num_leds_;}
  virtual void Spin();
private:
  uint32_t num_leds_;
  double update_rate_;
  double max_vel;
  double max_view_ang;
  std_msgs::ColorRGBA _cc;
  autonomy_leds_msgs::FeedbackConstPtr dir_ptr_;
  autonomy_leds_msgs::Keyframe key_frame_, clear_frame_;
  autonomy_leds_msgs::Animation anim_;

  ros::Subscriber cmd_sub_;
  ros::Publisher kf_pub_;
  ros::Publisher anim_pub_;

  void DirectionTranslatorCallback(const autonomy_leds_msgs::FeedbackConstPtr& dir_ptr);
  void Process();
  virtual void SpinOnce();

};

}  // namespace autonomy_leds

#endif
