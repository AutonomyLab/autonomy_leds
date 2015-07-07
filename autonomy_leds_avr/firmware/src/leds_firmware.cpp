/* ROS */
#include "ros.h"
#include "std_msgs/String.h"
#include "std_msgs/Empty.h"

/* AVR */
#include "Atmega32u4Hardware.h"
extern "C"
{
    #include <util/delay.h>
    #include <LUFA/Drivers/USB/USB.h>
    #include <avr/io.h>
    #include <avr/interrupt.h>
    #include "light_apa102.h"
}

// Needed for AVR to use virtual functions
extern "C" void __cxa_pure_virtual(void);
void __cxa_pure_virtual(void) {}

/* CONSTANTS */
#define NUM_LEDS 10
#define LED PC7
#define MAX_MSG_SIZE 20

/* LED Memory */
unsigned int led_counter = 0;
struct cRGB led_strip[NUM_LEDS];

void toggle_led()
{
    PORTC ^= (1 << LED);
    for (led_counter = 0; led_counter < NUM_LEDS; led_counter++)
    {
        led_strip[led_counter].r ^= 255;
    }
    apa102_setleds(led_strip, NUM_LEDS);
}

void toggle_leds_cb(const std_msgs::Empty& toggle_msg)
{
    toggle_led();
}

int init_io()
{
    DDRC |= (1 << LED);
}

int init_led_strip()
{
    for (led_counter = 0; led_counter < NUM_LEDS; led_counter++)
    {
        led_strip[led_counter].r = 0;
        led_strip[led_counter].g = 0;
        led_strip[led_counter].b = 0;
    }
}

int main()
{
  /* IO */
  init_io();
  init_led_strip();

  /* Variables */
  char hello[MAX_MSG_SIZE];
  uint32_t lasttime = 0UL;

  /* ROS */
  ros::NodeHandle nh;
  std_msgs::String str_msg;
  ros::Publisher chatter("chatter", &str_msg);
  ros::Subscriber<std_msgs::Empty> led_sub("toggle_led", &toggle_leds_cb );

  nh.initNode();
  nh.advertise(chatter);
  nh.subscribe(led_sub);

  long int counter = 0;

  toggle_led(); _delay_ms(500); toggle_led();

  while(1)
  {
    // Send the message every second
    if(avr_time_now() - lasttime > 1000)
    {
      snprintf(hello, MAX_MSG_SIZE, "Mani %ld", counter++);
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