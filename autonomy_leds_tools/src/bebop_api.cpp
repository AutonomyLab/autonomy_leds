#include <boost/bind.hpp>
#include <exception>
#include <string>

#include <ros/ros.h>

#include "autonomy_leds_tools/bebop_api.h"

using namespace autonomy_leds;

BebopAnimator::BebopAnimator(ros::NodeHandle& nh, const uint16_t num_leds)
    : nh_(nh),
      cmd_sub_(nh_.subscribe("leds/feedback", 1, &BebopAnimator::DirectionTranslatorCallback, this)),
      kf_pub_(nh_.advertise<autonomy_leds_msgs::Keyframe>("leds/display", 1)),
      anim_pub_(nh_.advertise<autonomy_leds_msgs::Animation>("leds/animation", 1))
{
    update_rate_ = 9;
    num_leds_ = num_leds;
    inited = false;
//    Hard coded values :
    max_vel = 4.5;
    max_view_ang = 70;
    _cc.r = 0.0;
    _cc.g = 0.0;
    _cc.b = 0.0;
}

void BebopAnimator::DirectionTranslatorCallback(const autonomy_leds_msgs::FeedbackConstPtr &dir_ptr)
{
  if( !inited
      || dir_ptr_.get()->anim_type != dir_ptr.get()->anim_type
      || dir_ptr_.get()->arrow_color.r != dir_ptr.get()->arrow_color.r
      || dir_ptr_.get()->arrow_color.g != dir_ptr.get()->arrow_color.g
      || dir_ptr_.get()->arrow_color.b != dir_ptr.get()->arrow_color.b
      || dir_ptr_.get()->center_color.r != dir_ptr.get()->center_color.r
      || dir_ptr_.get()->center_color.g != dir_ptr.get()->center_color.g
      || dir_ptr_.get()->center_color.b != dir_ptr.get()->center_color.b
      || dir_ptr_.get()->freq != dir_ptr.get()->freq
      || dir_ptr_.get()->value != dir_ptr.get()->value)
    dir_ptr_ = dir_ptr;
  inited = true;
  Process();
}

void BebopAnimator::Process()
{
    if( !dir_ptr_) return;
    if( dir_ptr_->anim_type != autonomy_leds_msgs::FeedbackConstPtr::element_type::TYPE_LOOK_AT) 
      last_look_dir = -9;
    anim_.keyframes.clear();
    if( dir_ptr_->anim_type == autonomy_leds_msgs::FeedbackConstPtr::element_type::TYPE_CLEAR)
    {
        clear_frame_.color_pattern.clear();
        clear_frame_.color_pattern.push_back( _cc);
        clear_frame_.start_index = 0;
        clear_frame_.duration = 0;
        clear_frame_.pattern_repeat = num_leds_;
        anim_.keyframes.push_back(clear_frame_);
        anim_.timing_function = autonomy_leds_msgs::AnimationConstPtr::element_type::TIMING_FUNCTION_LINEAR;
    }
    else if( dir_ptr_->anim_type == autonomy_leds_msgs::FeedbackConstPtr::element_type::TYPE_LOOK_AT)
    {
        clear_frame_.color_pattern.clear();
        clear_frame_.start_index = 0;
        clear_frame_.duration = 0;
        clear_frame_.pattern_repeat = 1;
        key_frame_.start_index = 0;
        key_frame_.duration = 0;
        key_frame_.pattern_repeat = 1;
        int pos = ((dir_ptr_->value > 0) ? 1 : ((dir_ptr_->value < 0) ? -1 : 0))*
                (fabs(dir_ptr_->value/max_view_ang)*(num_leds_/2))+(num_leds_/2);
        if( pos == last_look_dir)
          return;
        last_look_dir = pos;
        if( pos > int(num_leds_))
            pos = num_leds_;
        else if (pos < -1)
            pos = -1;
        for( int i = 0; i < num_leds_; i++)
            if( i == pos)
                clear_frame_.color_pattern.push_back( dir_ptr_->center_color);
            else
                clear_frame_.color_pattern.push_back( _cc);
        for( int i = std::max( int(num_leds_-pos), pos); i > -2; i--)
        {
            key_frame_.color_pattern.clear();
            for( int j = 0; j < num_leds_; j++)
            {
                if( j == pos)
                    key_frame_.color_pattern.push_back( dir_ptr_->center_color);
                else if(j == i+pos || j == pos-i)
                    key_frame_.color_pattern.push_back( dir_ptr_->arrow_color);
                else if( j == i+pos+1 || j == pos-i-1)
                {
                  std_msgs::ColorRGBA light_arrow;
                  light_arrow.r = dir_ptr_->arrow_color.r/2.0;
                  light_arrow.g = dir_ptr_->arrow_color.g/2.0;
                  light_arrow.b = dir_ptr_->arrow_color.b/2.0;
                  key_frame_.color_pattern.push_back( light_arrow);
                } 
                else
                    key_frame_.color_pattern.push_back( _cc);
            }
            anim_.keyframes.push_back(key_frame_);
        }
        for( int j = 0; j < 6; j++)
            anim_.keyframes.push_back(clear_frame_);
        anim_.timing_function = autonomy_leds_msgs::AnimationConstPtr::element_type::TIMING_FUNCTION_LINEAR;//TIMING_FUNCTION_EASE_OUT;
    }
    else if( dir_ptr_->anim_type == autonomy_leds_msgs::FeedbackConstPtr::element_type::TYPE_MOVE
             || dir_ptr_->anim_type == autonomy_leds_msgs::FeedbackConstPtr::element_type::TYPE_MOVE_BLINK)
    {
        key_frame_.color_pattern.clear();
        clear_frame_.color_pattern.clear();
        clear_frame_.color_pattern.push_back( _cc);
        if( dir_ptr_->value >= 0)
            key_frame_.color_pattern.push_back( dir_ptr_->center_color);
        int conv_val = (dir_ptr_->value/max_vel)*(num_leds_/2)+((dir_ptr_->value > 0) ? 1 : ((dir_ptr_->value < 0) ? -1 : 0));
        for( int i = 0; i < abs(conv_val); i++)
        {
            key_frame_.color_pattern.push_back( dir_ptr_->arrow_color);
            clear_frame_.color_pattern.push_back( _cc);
        }
        if( dir_ptr_->value < 0)
            key_frame_.color_pattern.push_back( dir_ptr_->center_color);
        key_frame_.start_index = std::min( 0, conv_val)+(( num_leds_-1)/2);
        key_frame_.duration = 0;
        key_frame_.pattern_repeat = 1;
        anim_.keyframes.push_back(key_frame_);
        if( dir_ptr_->anim_type == autonomy_leds_msgs::FeedbackConstPtr::element_type::TYPE_MOVE_BLINK)
        {
            clear_frame_.start_index = key_frame_.start_index;
            clear_frame_.duration = key_frame_.duration;
            clear_frame_.pattern_repeat = key_frame_.pattern_repeat;
            anim_.keyframes.push_back(clear_frame_);
        }
        anim_.timing_function = autonomy_leds_msgs::AnimationConstPtr::element_type::TIMING_FUNCTION_EASE_OUT;
    }
    else if( dir_ptr_->anim_type == autonomy_leds_msgs::FeedbackConstPtr::element_type::TYPE_FULL_BLINK)
    {
        key_frame_.color_pattern.clear();
        clear_frame_.color_pattern.clear();
        clear_frame_.color_pattern.push_back( _cc);
        key_frame_.color_pattern.push_back( dir_ptr_->arrow_color);
        key_frame_.start_index = 0;
        key_frame_.duration = 0;
        key_frame_.pattern_repeat = num_leds_;
        anim_.keyframes.push_back(key_frame_);
        clear_frame_.start_index = key_frame_.start_index;
        clear_frame_.duration = key_frame_.duration;
        clear_frame_.pattern_repeat = key_frame_.pattern_repeat;
        anim_.keyframes.push_back(clear_frame_);
        key_frame_.color_pattern.clear();
        key_frame_.color_pattern.push_back( dir_ptr_->center_color);
        anim_.keyframes.push_back(key_frame_);
        anim_.keyframes.push_back(clear_frame_);
        anim_.timing_function = autonomy_leds_msgs::AnimationConstPtr::element_type::TIMING_FUNCTION_EASE_OUT;
    }
    else if( dir_ptr_->anim_type == autonomy_leds_msgs::FeedbackConstPtr::element_type::TYPE_SEARCH_1)
    {
        key_frame_.color_pattern.clear();
        key_frame_.color_pattern.push_back( dir_ptr_->arrow_color);
        key_frame_.start_index = 0;
        key_frame_.duration = 0;
        key_frame_.pattern_repeat = 2;
        anim_.keyframes.push_back(key_frame_);
        key_frame_.color_pattern.clear();
        key_frame_.color_pattern.push_back( dir_ptr_->center_color);
        key_frame_.start_index = num_leds_-2;
        anim_.keyframes.push_back(key_frame_);
        anim_.timing_function = autonomy_leds_msgs::AnimationConstPtr::element_type::TIMING_FUNCTION_LINEAR;
    }
    else if( dir_ptr_->anim_type == autonomy_leds_msgs::FeedbackConstPtr::element_type::TYPE_SEARCH_2)
    {
        key_frame_.color_pattern.clear();
        key_frame_.color_pattern.push_back( _cc);
        key_frame_.start_index = 0;
        key_frame_.duration = 0;
        key_frame_.pattern_repeat = 1;
        anim_.keyframes.push_back(key_frame_);
        key_frame_.color_pattern.clear();
        key_frame_.color_pattern.push_back( dir_ptr_->arrow_color);
        key_frame_.start_index = 0;
        key_frame_.duration = 0;
        anim_.keyframes.push_back(key_frame_);
        key_frame_.color_pattern.clear();
        key_frame_.color_pattern.push_back( dir_ptr_->center_color);
        key_frame_.start_index = num_leds_-1;
        anim_.keyframes.push_back(key_frame_);
        key_frame_.color_pattern.clear();
        key_frame_.color_pattern.push_back( _cc);
        key_frame_.start_index = num_leds_-1;
        anim_.keyframes.push_back(key_frame_);
        anim_.timing_function = autonomy_leds_msgs::AnimationConstPtr::element_type::TIMING_FUNCTION_LINEAR;
    }
    else if( dir_ptr_->anim_type == autonomy_leds_msgs::FeedbackConstPtr::element_type::TYPE_FAST_BLINK)
    {
      key_frame_.color_pattern.clear();
      clear_frame_.color_pattern.clear();
      clear_frame_.color_pattern.push_back( _cc);
      key_frame_.color_pattern.push_back( dir_ptr_->arrow_color);
      key_frame_.start_index = 0;
      key_frame_.duration = 0;
      key_frame_.pattern_repeat = num_leds_;
      anim_.keyframes.push_back(key_frame_);
      clear_frame_.start_index = key_frame_.start_index;
      clear_frame_.duration = 3;
      clear_frame_.pattern_repeat = key_frame_.pattern_repeat;
      anim_.keyframes.push_back(clear_frame_);
      key_frame_.color_pattern.clear();
      key_frame_.color_pattern.push_back( dir_ptr_->center_color);
      anim_.keyframes.push_back(key_frame_);
      anim_.keyframes.push_back(clear_frame_);
      anim_.timing_function = autonomy_leds_msgs::AnimationConstPtr::element_type::TIMING_FUNCTION_EASE_OUT;
    }
    else if( dir_ptr_->anim_type == autonomy_leds_msgs::FeedbackConstPtr::element_type::TYPE_BLINK_CLEAR)
    {
      key_frame_.color_pattern.clear();
      clear_frame_.color_pattern.clear();
      clear_frame_.color_pattern.push_back( _cc);
      key_frame_.color_pattern.push_back( dir_ptr_->arrow_color);
      key_frame_.start_index = 0;
      key_frame_.duration = 0;
      key_frame_.pattern_repeat = num_leds_;
      anim_.keyframes.push_back(key_frame_);
      clear_frame_.start_index = key_frame_.start_index;
      clear_frame_.duration = 1;
      clear_frame_.pattern_repeat = key_frame_.pattern_repeat;
      key_frame_.color_pattern.clear();
      key_frame_.color_pattern.push_back( dir_ptr_->center_color);
      anim_.keyframes.push_back(key_frame_);
      key_frame_.color_pattern.clear();
      key_frame_.color_pattern.push_back( dir_ptr_->arrow_color);
      anim_.keyframes.push_back(key_frame_);
      key_frame_.color_pattern.clear();
      key_frame_.color_pattern.push_back( dir_ptr_->center_color);
      anim_.keyframes.push_back(key_frame_);
      key_frame_.color_pattern.clear();
      key_frame_.color_pattern.push_back( dir_ptr_->arrow_color);
      anim_.keyframes.push_back(key_frame_);
      key_frame_.color_pattern.clear();
      key_frame_.color_pattern.push_back( dir_ptr_->center_color);
      anim_.keyframes.push_back(key_frame_);
      anim_.keyframes.push_back(clear_frame_);
      anim_.timing_function = autonomy_leds_msgs::AnimationConstPtr::element_type::TIMING_FUNCTION_EASE_OUT;
    }
    else if( dir_ptr_->anim_type == autonomy_leds_msgs::FeedbackConstPtr::element_type::TYPE_CONST_3)
    {
      key_frame_.color_pattern.clear();
      key_frame_.color_pattern.push_back( dir_ptr_->arrow_color);
      for( int i = 0; i < 3; i++)
        key_frame_.color_pattern.push_back( _cc);
      key_frame_.color_pattern.push_back( dir_ptr_->center_color);
      for( int i = 0; i < 3; i++)
        key_frame_.color_pattern.push_back( _cc);
      key_frame_.color_pattern.push_back( dir_ptr_->arrow_color);
      key_frame_.start_index = 0;
      key_frame_.duration = 0;
      key_frame_.pattern_repeat = num_leds_;
      anim_.keyframes.push_back(key_frame_);
      anim_.timing_function = autonomy_leds_msgs::AnimationConstPtr::element_type::TIMING_FUNCTION_LINEAR;
    }
    else if( dir_ptr_->anim_type == autonomy_leds_msgs::FeedbackConstPtr::element_type::TYPE_EYE)
    {
        clear_frame_.color_pattern.clear();
        clear_frame_.start_index = 0;
        clear_frame_.duration = 0;
        clear_frame_.pattern_repeat = 1;
        key_frame_.color_pattern.clear();
        key_frame_.start_index = 0;
        key_frame_.duration = 2;
        key_frame_.pattern_repeat = 1;
        int pos = ((dir_ptr_->value > 0) ? 1 : ((dir_ptr_->value < 0) ? -1 : 0))*
                (fabs(dir_ptr_->value/max_view_ang)*(num_leds_/2))+(num_leds_/2);
        if( pos == last_look_dir)
          return;
        last_look_dir = pos;
        if( pos > int(num_leds_)-4)
            pos = num_leds_-4;
        else if (pos < 3)
            pos = 3;
        for( int i = 0; i < num_leds_; i++)
        {
            if( i == pos-2 || i == pos-3)
                key_frame_.color_pattern.push_back( dir_ptr_->arrow_color);
            else if( i == pos+3 || i == pos+2)
                key_frame_.color_pattern.push_back( dir_ptr_->center_color);
            else
                key_frame_.color_pattern.push_back( _cc);
            clear_frame_.color_pattern.push_back( _cc);
        }
        anim_.keyframes.push_back(key_frame_);
        anim_.keyframes.push_back(clear_frame_);
        anim_.timing_function = autonomy_leds_msgs::AnimationConstPtr::element_type::TIMING_FUNCTION_LINEAR;//TIMING_FUNCTION_EASE_OUT;
    }
    else if( dir_ptr_->anim_type == autonomy_leds_msgs::FeedbackConstPtr::element_type::TYPE_TIMER_SNAP)
    {
      key_frame_.color_pattern.clear();
      clear_frame_.color_pattern.clear();
      clear_frame_.color_pattern.push_back( _cc);
      key_frame_.color_pattern.push_back( dir_ptr_->arrow_color);
      key_frame_.start_index = 0;
      key_frame_.duration = 0;
      key_frame_.pattern_repeat = num_leds_;
      anim_.keyframes.push_back(key_frame_);
      clear_frame_.start_index = key_frame_.start_index;
      clear_frame_.duration = 0;
      clear_frame_.pattern_repeat = key_frame_.pattern_repeat;
      anim_.keyframes.push_back(clear_frame_);
      anim_.keyframes.push_back(key_frame_);
      anim_.keyframes.push_back(clear_frame_);
      clear_frame_.duration = 0.5;
      anim_.keyframes.push_back(key_frame_);
      anim_.keyframes.push_back(clear_frame_);
      key_frame_.color_pattern.clear();
      key_frame_.color_pattern.push_back( dir_ptr_->center_color);
      key_frame_.duration = 1;
      anim_.keyframes.push_back(key_frame_);
      clear_frame_.duration = 3;
      anim_.keyframes.push_back(clear_frame_);
      anim_.timing_function = autonomy_leds_msgs::AnimationConstPtr::element_type::TIMING_FUNCTION_EASE_IN;
    }
    else if( dir_ptr_->anim_type == autonomy_leds_msgs::FeedbackConstPtr::element_type::TYPE_BYEBYE)
    {
        key_frame_.start_index = 0;
        key_frame_.duration = 0;
        key_frame_.pattern_repeat = 1;
        for(int  i = num_leds_/2+1; i > -1; i--)
        {
          key_frame_.color_pattern.clear();
          for( int j = 0; j < num_leds_; j++)
            if( abs(num_leds_/2-j) < i)
              key_frame_.color_pattern.push_back( dir_ptr_->arrow_color);
            else
              key_frame_.color_pattern.push_back( dir_ptr_->center_color);
          anim_.keyframes.push_back(key_frame_);
        }
        key_frame_.duration = 120;
        anim_.keyframes.push_back(key_frame_);
        anim_.timing_function = autonomy_leds_msgs::AnimationConstPtr::element_type::TIMING_FUNCTION_LINEAR;
    }
    anim_.iteration_count = 0;
    anim_.smooth_transition = true;
    anim_.transition_duration = 1.0/(dir_ptr_->freq/**anim_.keyframes.size()*/);
    anim_pub_.publish( anim_);
}

void BebopAnimator::SpinOnce()
{
    ros::spinOnce();
}

void BebopAnimator::Spin()
{
    ros::Rate rate(update_rate_);
    while (ros::ok())
    {
        SpinOnce();
        // TODO: Graceful shutdown:
        // http://answers.ros.org/question/27655/what-is-the-correct-way-to-do-stuff-before-a-node-is-shutdown/
        if (!rate.sleep())
        {
            ROS_WARN_STREAM("[LEd] missed target loop rate of " << update_rate_ << " hz.");
        }
    }

}

int main(int argc, char* argv[])
{
    ros::init(argc, argv, "bebop_api_node");
    ros::NodeHandle nh;

    BebopAnimator animator(nh, 11);

    ROS_INFO("[BBP] LEDS Animation Engine started ...");

    animator.Spin();

    return 0;
}
