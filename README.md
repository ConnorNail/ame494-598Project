# IMU Sensor Package to Detect Accuracy of Virtual Reality Controlled Robotic Arm

This project is designed around a robotic arm system in which a virtual reality headset and controller are used for teleoperated control of a robotic arm. The users arm movements are mimicked by the robot. This projects specifically utilizes an ESP32 and a BNO08x IMU to track the pose of the robotic arms end effector and the user’s hand. This data is then analyzed in order to identify the accuracy of the arm to the users inputs.

## Server Startup Instructions
1. Navigate to t-watch-bno085-main -> aggregator-server
2. Install necessary node dependencies
```npm install```
3. Launch the server
```node server.js```

## Additional Information
This project also contains the necessary files to run on the client ESP32. This file can be found under t-watch-bno085-main -> pod_mcu_code-08x-wifi.

The server will try and stre values in a locally hosted MongoDB at the default port of 27017.
