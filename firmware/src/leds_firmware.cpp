#include "ros.h"
#include "std_msgs/String.h"
#include "Atmega32u4Hardware.h"
// Include C headers (ie, non C++ headers) in this block
extern "C"
{
  #include <util/delay.h>
  #include <LUFA/Drivers/USB/USB.h>
}

// Blink
#include <avr/io.h>
#define LED PC7

// Needed for AVR to use virtual functions
extern "C" void __cxa_pure_virtual(void);
void __cxa_pure_virtual(void) {}

ros::NodeHandle nh;

std_msgs::String str_msg;
ros::Publisher chatter("chatter", &str_msg);

#include <stdio.h>
#define MAX_MSG_SIZE 20
char hello[MAX_MSG_SIZE];

int main()
{
  DDRC |= (1 << LED);
  uint32_t lasttime = 0UL;
  // Initialize ROS
  nh.initNode();
  nh.advertise(chatter);

  long int counter = 0;
  while(1)
  {
    // Send the message every second
    
    if(avr_time_now() - lasttime > 1000)
    {
      PORTC ^= (1 << LED);

      snprintf(hello, MAX_MSG_SIZE, "Mani %d", counter++);
      str_msg.data = hello;
      chatter.publish(&str_msg);
      lasttime = avr_time_now();
    }
    nh.spinOnce();
    // LUFA functions that need to be called frequently to keep USB alive
    CDC_Device_USBTask(&Atmega32u4Hardware::VirtualSerial_CDC_Interface);
    USB_USBTask();
  }

  return 0;
}
