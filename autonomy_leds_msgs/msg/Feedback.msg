uint8 TYPE_CLEAR = 0
uint8 TYPE_LOOK_AT = 1
uint8 TYPE_MOVE = 2
uint8 TYPE_MOVE_BLINK = 3
uint8 TYPE_FULL_BLINK = 4
uint8 TYPE_SEARCH_1 = 5
uint8 TYPE_SEARCH_2 = 6

# Shows the message type
# TYPE_CLEAR: to turn off all the LEDs
# TYPE_LOOK_AT: for look at mode (input value as degree)
# TYPE_MOVE: moving speedand direction (input value as speed)
uint8 anim_type

# Shows the color of central LED
# r,g,b should be in [0..1] range, a is ignored
std_msgs/ColorRGBA center_color

# Shows the color of arrow LEDs
# r,g,b should be in [0..1] range, a is ignored
std_msgs/ColorRGBA arrow_color

# The value for which animation should be created
# Angle if the type is LOOK_AT (-view_port_angle < .. < +view_port_angle)
# Speed if the type is TYPE_MOVE (-max_speed < .. < +max_speed)
float64 value

# Frequency (0 to Animation Engine max freq [25])
float64 freq

