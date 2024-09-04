# Sweepy Cleaning Robot
 Software implementation for sweeping robot controlled by Mbed LPC1768 microcontroller
 Uses open-source libraries for peripherals

SW implementation of a cleaning robot that is controlled wirelessly via Bluetooth. Includes a LIDAR sensor, SD reader, speaker, and motors controlled by an Mbed microcontroller. It has multiple modes including:
-Spot mode goes in circles cleaning a certain spot
-RC mode is controlled remotely through Bluetooth like an RC car
-Autonomous just cleans until told otherwise using LIDAR for obstacle avoidance
-Entertainment mode plays music and the robot flashes LED lights

System architecture: 

![image](https://github.com/user-attachments/assets/d93e3590-764e-4d70-a6b1-e685e6489124)


Power Requirements for HW system:

![image](https://github.com/user-attachments/assets/c679190a-0331-49f1-aaf9-fbc78253d6bc)

Testing Mode Switching/LIDAR: 

-Robot achieved seamless mode switching in 43/50 trials
-22/50 before fixing synchronization mechanisms
-Robot achieved successful autonomous navigation in fixed environment in 37/50 trials (30 seconds of active time)
-13 collisions involved corners , curved surfaces, or overpasses
-22/25 successful trials with two additional LIDAR chips

Challenges: 

Shared variables and race conditions;
Limited peripheral support with Mbed microcontroller;
Need for multiple battery packs;
Limited cone of view of LIDAR chip;
High cost of high-quality LIDAR (better recently);

Ideas for Improvement:

Have an array of LIDAR sensors or rotating LIDAR
-Roombas and other competition use sensors (slower and faulty in poor lighting);
Develop comprehensive environment map with algorithmic solution; 
Incorporate microcontroller and peripherals into custom PCB;
Eliminate extraneous wiring and breadboard;
Incorporate IoT for entertainment mode;
Use more powerful and capable microcontroller;
Higher processing power, memory, peripheral support, clock speed;
Battery monitoring
ML to improve object detection and classification based on sensor data




