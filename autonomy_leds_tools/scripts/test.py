#!/usr/bin/env python
import rospy
from autonomy_leds_msgs.msg import Command, LED

import time

num_leds = 9

def main():
    set_pub = rospy.Publisher('leds/set', Command, queue_size=10)
    set_led_pub = rospy.Publisher('leds/set_led', LED, queue_size=10)
    
    rospy.init_node('led_tester', anonymous = True)
    #rate = rospy.Rate(1)
    
    command_msg = Command()
    
    rospy.loginfo("High freq test ...")
    for i in range(0, 20):
        led = 0b0111110000000000 
        command_msg.colors_vec = [led] * num_leds
        command_msg.flag = 0
        time.sleep(0.05)
        set_pub.publish(command_msg)

        led = 0b0000000000011111
        command_msg.colors_vec = [led] * num_leds
        command_msg.flag = 0
        time.sleep(0.05)
        set_pub.publish(command_msg)

    time.sleep(0.5)    
    rospy.loginfo("Clearing ...")
    command_msg.flag = 1
    command_msg.colors_vec = []
    set_pub.publish(command_msg)

    time.sleep(0.5)    
    command_msg.colors_vec = []
    command_msg.flag = 0    
    for i in range(0, num_leds):
        single_led = 0b0111111111111111 if (i < 2) else 0b0000000000011111
        command_msg.colors_vec.append(single_led)
    set_pub.publish(command_msg)

    time.sleep(0.5)
    rospy.loginfo("Shift right ...")
    for i in range(0, num_leds):
        command_msg.colors_vec = []
        command_msg.flag = 3
        set_pub.publish(command_msg)
        time.sleep(0.04)

    rospy.loginfo("Shift left ...")
    time.sleep(0.5)
    for i in range(0, num_leds):
        command_msg.colors_vec = []
        command_msg.flag = 2
        set_pub.publish(command_msg)
        time.sleep(0.04)

    time.sleep(0.5)
    rospy.loginfo("Inverting ...")
    for i in range(0, 10):
        command_msg.colors_vec = []
        command_msg.flag = 4
        set_pub.publish(command_msg)
        time.sleep(0.1)

    rospy.loginfo("Set individual LED ...")
    led_msg = LED()
    for i in range(0, num_leds):
        led_msg.index = i
        led_msg.color = 0b0000001111100000
        set_led_pub.publish(led_msg)
        time.sleep(0.04)

    time.sleep(0.5)    
    rospy.loginfo("Clearing ...")
    command_msg.flag = 1
    command_msg.colors_vec = []
    set_pub.publish(command_msg)

if __name__ == '__main__':
    try:
        main()
    except rospy.ROSInterruptException:
        pass
