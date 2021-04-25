///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name: Chan Car Lin
// UID: 3035604568
// Course: ELEC3442 Embedded System
// Project: Smart print bed for 3d printer
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Creating a BluetoothSerial object
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "BluetoothSerial.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Creating a TaskHandle_t objects for multi-core programming (Bluetooth communication)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TaskHandle_t task_bluetooth;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Variables for servo motors
// The index refer to the corresponding servo motor: {top_left, top_right, botton_left, botton right}
//
// top_left                         top_right
//        --------------------------
//        |                        |
//        |                        |
//        |                        |
//        |                        |
//        |       Print bed        |
//        |                        |
//        |                        |
//        |                        |
//        --------------------------
// bottom_left                      bottom_right

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int pin_list[4] = {27, 14, 12, 13};   // Servo pins

#include <ESP32Servo.h>
Servo servo_0;
Servo servo_1;
Servo servo_2;
Servo servo_3;
Servo servo_list[4] = {servo_0, servo_1, servo_2, servo_3};   // Servo objects

int angle_list[4] = {0, 0, 0, 0};   // Used to store the servo current angle
int msg_angle[4] = {0, 0, 0, 0};    // Used to store the adjust angle sent from Raspberry Pi

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Variables for servos control
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
String msg = "";    // Incoming message
String command = "";    // This command used to adjust all servo motors

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Variables for core checking
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool core_check_main = false;
bool core_check_bt = false;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Used to adjust all servo
// Para: command_string - The command string
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void servo_adjust() {
  Serial.println("command: " + command);
  
  int first_comma = command.indexOf(',');
  int second_comma = command.indexOf(',', first_comma+1);
  int third_comma = command.indexOf(',', second_comma+1);

  msg_angle[0] = (command.substring(0, first_comma)).toInt();
  msg_angle[1] = (command.substring(first_comma+1, second_comma)).toInt();
  msg_angle[2] = (command.substring(second_comma+1, third_comma)).toInt();
  msg_angle[3] = (command.substring(third_comma+1)).toInt();    

  for (int i = 0; i < 4; i++) {
    angle_list[i] = angle_list[i] + msg_angle[i];
    if (angle_list[i] < 0) {
      angle_list[i] = 0;
    }
    else if (angle_list[i] > 180) {
      angle_list[i] = 180;
    }
    servo_list[i].write(angle_list[i]);
  }

  command = "";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The task function for bluetooth communication core
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void task_bluetooth_code(void * pvParameters) {
  for(;;) {
    // For checking the bluetooth run on which core
    // Execute once on the initial
    if (core_check_bt == false) {
      Serial.println();
      Serial.println("Bluetooth core: " + String(xPortGetCoreID()) + ", with time: " + millis());
      core_check_bt = true;
    }  
    // Listening the BT serial
    if (SerialBT.available()) {
      char temp = SerialBT.read();      
      msg += temp; 
    }
    else {
      // Checking the msg whether is emtpy or not
      if (msg.length() > 0) {
        command = msg;
      }
      msg = "";
    }
    delay(10);
  }  
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setup()
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32"); //Bluetooth device name

  // Initialize the setting of second core
  // Because the default running core of ESP32 is core 1, so the bluetooth task should be assigned to core 0
  xTaskCreatePinnedToCore(
    task_bluetooth_code,  // Task function
    "task_bluetooth",  // Task name
    10000,  // Stack size of task
    NULL,  // Parameter of task
    0, // Priority of task - 0: lower; 1: higher (same as setup() and loop()) 
    &task_bluetooth,  // Task handle
    0   // Core where the task should run
    );

  for (int i = 0; i < 4; i++) {
    servo_list[i].attach(pin_list[i]);
    angle_list[i] = 90;
    servo_list[i].write(angle_list[i]);
  }
  
  Serial.println("Ready!");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// loop()
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() { 
  // For checking the main loop() run on which core
  // Execute once on the initial
  if (core_check_main == false) {
    Serial.println();
    Serial.println("Main core: " + String(xPortGetCoreID()) + ", with time: " + millis());
    core_check_main = true;  
  }
  
  // Adjust the servos with a specific command
  if (command != "") {
    servo_adjust();
  } 
}
