#!/usr/bin/env python
import rospy
from autonomy_leds_msgs.msg import Animation, Keyframe, Command
from std_msgs.msg import ColorRGBA
import time, copy

num_leds = 5

def main():
    display_pub = rospy.Publisher('leds/display', Keyframe, queue_size=10)
    anim_pub = rospy.Publisher('leds/animation', Animation, queue_size=10)
    set_pub = rospy.Publisher('leds/set', Command, queue_size=10)

    rospy.init_node('led_anim_tester', anonymous = True)
    time.sleep(0.5)

    keyframe_msg = Keyframe()
    command_msg = Command()
    color_msg = ColorRGBA()
    anim_msg = Animation()
    
    rospy.loginfo("Single frame test ...")
    keyframe_msg.color_pattern = []
    color_msg.r = 0.2
    color_msg.g = 0.0
    color_msg.b = 0.0
    keyframe_msg.color_pattern.append(copy.deepcopy(color_msg))
    color_msg.r = 0.0
    color_msg.g = 0.0
    color_msg.b = 0.5
    keyframe_msg.color_pattern.append(copy.deepcopy(color_msg))
    color_msg.r = 0.5
    color_msg.g = 0.5
    color_msg.b = 0.5
    keyframe_msg.color_pattern.append(copy.deepcopy(color_msg))
    keyframe_msg.pattern_repeat = num_leds / 3
    keyframe_msg.start_index = 0
    keyframe_msg.duration = 5.0
    display_pub.publish(keyframe_msg)

    time.sleep(5.0)

    rospy.loginfo("Three frame animation test ...")    
    anim_msg.iteration_count = 0
    anim_msg.smooth_transition = True
    anim_msg.transition_duration = 4.0
    anim_msg.timing_function = 0#Animation.TIMING_FUNCTION_EASE_INOUT
    keyframe_msg.color_pattern = []    
    color_msg.r = 0.1
    color_msg.g = 0.0
    color_msg.b = 0.0
    keyframe_msg.color_pattern.append(copy.deepcopy(color_msg))
    keyframe_msg.pattern_repeat = 1
    keyframe_msg.start_index = 0
    keyframe_msg.duration = 0.5
    anim_msg.keyframes.append(copy.deepcopy(keyframe_msg))

    keyframe_msg.color_pattern = []    
    color_msg.r = 0.0
    color_msg.g = 0.9
    color_msg.b = 0.9
    keyframe_msg.color_pattern.append(copy.deepcopy(color_msg))
    keyframe_msg.pattern_repeat = num_leds
    keyframe_msg.start_index = 0
    keyframe_msg.duration = 0.5
    
    anim_msg.keyframes.append(copy.deepcopy(keyframe_msg))


    keyframe_msg.color_pattern = []
    color_msg.r = 0.0
    color_msg.g = 0.9
    color_msg.b = 0.0
    keyframe_msg.color_pattern.append(copy.deepcopy(color_msg))
    keyframe_msg.pattern_repeat = 10
    keyframe_msg.start_index = num_leds - 10
    keyframe_msg.duration = 0.5
    
    #anim_msg.keyframes.append(copy.deepcopy(keyframe_msg))

    anim_pub.publish(anim_msg)
    time.sleep(1)

    # rospy.loginfo("Clearing ...")
    # command_msg.flag = 1
    # command_msg.colors_vec = []
    # set_pub.publish(command_msg)


    
if __name__ == '__main__':
    try:
        main()
    except rospy.ROSInterruptException:
        pass
