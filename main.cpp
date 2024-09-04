
#include "mbed.h"
#include "rtos.h"
#include "uLCD_4DGL.h"
#include "SDFileSystem.h"
#include "wave_player.h"
#include <iterator>
#include <string>
#include "mbed.h"
#include "Motor.h"
#include "XNucleo53L0A1.h"

//LCD
//uLCD_4DGL uLCD(p13, p14, p24); // tx,rx,rst
Serial pc(USBTX,USBRX);

//Bluetooth
Serial blue(p13, p14);

//Lidar stuff
DigitalOut shdn(p26);
#define VL53L0_I2C_SDA   p28
#define VL53L0_I2C_SCL   p27

static XNucleo53L0A1 *board=NULL;
int status;



//Motorstuff
float straightSpeed = 0.5;
float turnSpeed = 0.4;
//Motor left_motor(p21,p5,p6); //pwm,fwd,rev
//Motor right_motor(p22,p7,p8); //pwm,fwd,rev

Motor left_motor(p21,p24,p23); //pwm,fwd,rev
Motor right_motor(p22,p19,p20); //pwm,fwd,rev





SDFileSystem sd(p5, p6, p7, p8, "sd"); //SD card
AnalogOut DACout(p18);
wave_player wav(&DACout); // 
char buffer[4] = {0,0,0,0};

BusOut LEDS(LED1, LED2, LED3, LED4);


//int idx = 0;


Mutex lcd_mutex; // mutex for lcd
Mutex mode_mutex; // mutex for roomba mode
Mutex distance_mutex;






enum Mode {
    SPOT,
    RC,
    AUTONOMOUS,
    ENTERTAINMENT
};

volatile Mode currentMode = RC;

void setMode(Mode mode) {
    currentMode = mode;
}

void wav_thread(void const *args)
{
    while(true) {
        FILE *wave_file;
        mode_mutex.lock();
        Mode mode = currentMode;
        mode_mutex.unlock();
        switch (mode){
            case RC:
                wave_file = fopen("/sd/mydir/rcmode.wav","r");
                break;
            case SPOT:
                wave_file = fopen("/sd/mydir/spotmode.wav","r");
                break;
            case AUTONOMOUS:
                wave_file = fopen("/sd/mydir/autonomousmode.wav","r");
                break;
            case ENTERTAINMENT:
                wave_file = fopen("/sd/mydir/entertainmentmode.wav","r");
                break;
            default:
                break;
        }
        if(wave_file == NULL) pc.printf("Could not open file for read\n\r");
        wav.play(wave_file);
        fclose(wave_file);
    }
}


void bluetooth_control(const void *args) {
    char bnum = 0;
    char bhit = 0;
    while(1) {
        if (blue.getc()=='!') {
            if (blue.getc()=='B') {
                bnum = blue.getc();
                bhit = blue.getc();
                mode_mutex.lock();
                Mode current = currentMode;
                mode_mutex.unlock();
                if (bhit == '1') {
                    switch(bnum) {
                        case '1':
                            mode_mutex.lock();
                            setMode(SPOT);
                            mode_mutex.unlock();
                            LEDS = 1;
                            break;
                        case '2':
                            mode_mutex.lock();
                            setMode(RC);
                            mode_mutex.unlock();
                            left_motor.speed(0);
                            right_motor.speed(0);
                            LEDS = 2;
                            break;
                        case '3':
                            mode_mutex.lock();
                            setMode(AUTONOMOUS);
                            mode_mutex.unlock();
                            LEDS = 4;
                            break;
                        case '4':
                            mode_mutex.lock();
                            setMode(ENTERTAINMENT);
                            mode_mutex.unlock();
                            left_motor.speed(0);
                            right_motor.speed(0);
                            break;
                        case '5': //button 5 up arrow
                            if (current == RC) {
                                left_motor.speed(-1*straightSpeed);
                                right_motor.speed(-1*straightSpeed);
                            }
                            break;
                        case '6': //button 6 down arrow
                            if (current == RC) {
                                left_motor.speed(straightSpeed);
                                right_motor.speed(straightSpeed);
                            }
                            break;
                        case '7': //button 7 left arrow
                            if (current == RC) {
                                left_motor.speed(-1*turnSpeed);
                                right_motor.speed(turnSpeed);
                            }
                            break;
                        case '8': //button 8 right arrow
                            if (current == RC) {
                                left_motor.speed(turnSpeed);
                                right_motor.speed(-1*turnSpeed);
                            }
                            break;
                        default:
                            break;
                    }
                } else {
                    switch(bnum) {
                        case '5': //button 5 up arrow
                            if (current == RC) {
                                left_motor.speed(0);
                                right_motor.speed(0);
                            }
                            break;
                        case '6': //button 6 down arrow
                            if (current == RC) {
                                left_motor.speed(0);
                                right_motor.speed(0);
                            }
                            break;
                        case '7': //button 7 left arrow
                            if (current == RC) {
                                left_motor.speed(0);
                                right_motor.speed(0);
                            }
                            break;
                        case '8': //button 8 right arrow
                            if (current == RC) {
                                left_motor.speed(0);
                                right_motor.speed(0);
                            }
                            break;
                        default:
                            break;
                    }
                }

            }
        }
        lcd_mutex.unlock();
        
    }
}


volatile uint32_t distanceValue = 0;  // Global variable for storing distance value

void autonomousDistanceThread(const void *args){
        uint32_t distance = 0;
        //loop taking and printing distance
        while (1) {
            status = board->sensor_centre->get_distance(&distance);
            if (status == VL53L0X_ERROR_NONE) {
                pc.printf("D=%ld mm\r\n", distance);
            }
            distance_mutex.lock();
            distanceValue = distance;
            distance_mutex.unlock();
            Thread::wait(100);
        }
}

void spotMode() {

    // Make the robot continuously rotate in place
    left_motor.speed(turnSpeed);
    right_motor.speed(-turnSpeed);
}

void rcMode() {

}

void autonomousMode() {
// Autonomous mode logic
    distance_mutex.lock();
    uint32_t distance = distanceValue;
    distance_mutex.unlock();
    if (distance < 150 && distance > 5) {
    // Reverse slightly when less than 150 mm from an object
        left_motor.speed(straightSpeed);
        right_motor.speed(straightSpeed);
        Thread::wait(1000);  //Time for reverse action
        left_motor.speed(turnSpeed);
        right_motor.speed(-turnSpeed);
        Thread::wait(500);
    } else {
        left_motor.speed(-straightSpeed);
        right_motor.speed(-straightSpeed);
    }
}
void entertainmentMode() {
    LEDS = 1;
    for (int i = 0; i < 6; i++) {
        if (i < 3) {
            LEDS = LEDS << 1;
        } else {
            LEDS = LEDS >> 1;
        }
    }
}

int main()
{

    DevI2C *device_i2c = new DevI2C(VL53L0_I2C_SDA, VL53L0_I2C_SCL);
    /* creates the 53L0A1 expansion board singleton obj */
    board = XNucleo53L0A1::instance(device_i2c, A2, D8, D2);
    shdn = 0; //must reset sensor for an mbed reset to work
    Thread::wait(100);
    shdn = 1;
    Thread::wait(100);
    /* init the 53L0A1 board with default values */
    status = board->init_board();
    while (status) {
        pc.printf("Failed to init board! \r\n");
        status = board->init_board();
    }
    pc.printf("initialized\n");


    Thread th1(bluetooth_control);
    Thread th2(autonomousDistanceThread);
    Thread th3(wav_thread);


    while (true) {
        mode_mutex.lock();
        Mode current = currentMode;
        mode_mutex.unlock();

        switch (current) {
            case SPOT:
                spotMode();
                break;
            case RC:
                rcMode();
                break;
            case AUTONOMOUS:
                autonomousMode();
                break;
            case ENTERTAINMENT:
                entertainmentMode();
                break;
        }
    }
}

