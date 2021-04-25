############################################################
# Name: Chan Car Lin
# UID: 3035604568
# Course: ELEC3442 Embedded System
# Project: Smart print bed for 3d printer
############################################################

############################################################
# Initializing the variables
############################################################
import bluetooth
from sense_hat import *
from time import sleep

# The upper limit value
upper_limit = 330
# The lower limit value
lower_limit = 30
# The list used to store the adjusted value
msg_int = [0] * 4
# The interval of the while loop (unit: second)
interval = 2

# Bluetooth socker object
sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
# Port number
port = 1
# MAC address of ESP32
serverMACAddress = '7C:9E:BD:F4:61:26'
# Connecting to the ESP32 via Bluetooth
sock.connect((serverMACAddress, port))

# Initializing the senseHat
sense = SenseHat()
sense.clear()

############################################################
# Main loop
############################################################
while True:
    # --------------------------------------------------
    # Getting the orientation of the senseHat
    # --------------------------------------------------
    orientation = sense.get_orientation()
    p = round(orientation["pitch"], 0)
    r = round(orientation["roll"], 0)
    # print("p: %s, r: %s" % (p,r))   
    # y = round(orientation["yaw"], 0)
    # print("p: %s, r: %s, y: %s" % (p,r,y))

    # --------------------------------------------------
    # Calculating the value of the adjustment (Pitch)
    # For testing, the pitch value is bounded by
    # pitch <= lower_limit and pitch >= upper_limit
    # --------------------------------------------------
    if (p <= lower_limit or p >= upper_limit):
        if (p <= lower_limit):
            p_angle = round(p/2)
            msg_int[0] += p_angle
            msg_int[1] -= p_angle
            msg_int[2] += p_angle          
            msg_int[3] -= p_angle
        else:
            p_angle = round((360-p)/2)
            msg_int[0] -= p_angle
            msg_int[1] += p_angle
            msg_int[2] -= p_angle           
            msg_int[3] += p_angle

    # msg_string = str(msg_int[0]) + "," + str(msg_int[1]) + "," + str(msg_int[2]) + "," + str(msg_int[3]);
    # print(msg_string)

    # --------------------------------------------------
    # Calculating the value of the adjustment (Roll)
    # For testing, the roll value is bounded by
    # roll <= lower_limit and roll >= upper_limit
    # --------------------------------------------------
    if (r <= lower_limit or r >= upper_limit):
        if (r <= lower_limit):
            r_angle = round(r/2)
            msg_int[0] -= r_angle
            msg_int[1] -= r_angle
            msg_int[2] += r_angle
            msg_int[3] += r_angle           
        else:
            r_angle = round((360-r)/2)
            msg_int[0] += r_angle
            msg_int[1] += r_angle
            msg_int[2] -= r_angle
            msg_int[3] -= r_angle            

    msg_string = str(msg_int[0]) + "," + str(msg_int[1]) + "," + str(msg_int[2]) + "," + str(msg_int[3]);
    print(msg_string)

    # --------------------------------------------------
    # Sending out the value,
    # then clear the previous values
    # --------------------------------------------------
    sock.send(msg_string)
    msg_int[0] = 0
    msg_int[1] = 0
    msg_int[2] = 0
    msg_int[3] = 0
    
    sleep(interval)




