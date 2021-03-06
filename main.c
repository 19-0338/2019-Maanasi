#include <kipr/botball.h>
#define ET analog(2)
#define LIGHT_1 analog(1)
#define LIGHT_0 analog(0)
#define THRESHOLD 3700
#define OPEN 710
#define CLOSE 0

double bias; //variable to hold the calibration value
int right_motor, left_motor;//ports

void snatch(){
    //open claw
    set_servo_position(0, OPEN);
    //goes foward slightly
    drive_with_gyro(500, 2000.0);
    //shifts left if detects pipe
    /*
    if(ET < 2100) {
        drive_with_gyro(-500, 1300.0);
        turn_with_gyro(-500, 500, 580000.0);
        drive_with_gyro(550, 300.0);
        turn_with_gyro(500, -500, 580000.0);
        drive_with_gyro(500, 1300.0);
    } */
    //moves forward and surrounds people
    drive_with_gyro(800, 2000.0);
    //closes claw
    msleep(2500);
    set_servo_position(0, CLOSE);
    msleep(2000);
    //back out until on black line
    drive_with_gyro(-1000, 2000.0);
    gyro_light_stop(-1000);
    drive_with_gyro(500, 1000.0);
    
    square_up_black_line();
    //turn 
    //line follow black line
    //turn right towards next blue line (90 degrees)
    //forward until near blue line

}

int main()
{ 
    declare_motors(1, 0);
    cg();
    enable_servos();
    set_servo_position(0, CLOSE);
    //turn to be parallel with edge
    turn_with_gyro(500, -500, 380000.0);

    //move forward a set distance until in front of first person line   
    drive_with_gyro(1000, 3700.0);

    //turn left then square up against back pvc pipe
    turn_with_gyro(-500, 500, 580000.0);
    drive_with_gyro(-1000, 1300.0);
    set_servo_position(0, OPEN);
    //move forward till black tape of black/grey line
    gyro_light_stop(1000);
    //move forward; if ET sensor detects pipe in front, then back up, turn right, move slightly forward, then turn left
    drive_with_gyro(1000, 1000.0);
    square_up_black_line();
    snatch();
    //check again to make sure nothing is in front; if there is, repeat previous step

    //func: GrabPeopleAndLineFollow (for loop)

    int i; // forgot that this isn't java :/
    for(i = 0; i <= 4; i++){
        printf("test");

    }
    //repeat 4 times at each person line

    //

    //if:

    //check the closer building; if it doesn't detect a yellow square, turn left, move forward, drop people

    //else, turn left, move forward, turn right, move forward, turn left, and drop people into non-burning building


    return 0;
}



//initializes the ports for the motors.
void declare_motors(int lmotor, int rmotor)
{
    right_motor = rmotor;
    left_motor = lmotor;
}

//Gyroscopes are always off by a specific angular velocity so we need to subtract this bias from all readings.
//Call calibrate_gyro to find what the gyroscope reads when the robot is at a complete stop (this is what the bias is). 
//The bias may change between turning the robot on when compared to the bias that the gyroscope has when it starts moving.
void calibrate_gyro()
{
    //takes the average of 50 readings
    int i = 0;
    double avg = 0;
    while(i < 50)
    {
        avg += gyro_z();
        msleep(1);
        i++;
    }
    bias = avg / 50.0;
    printf("New Bias: %f\n", bias);//prints out your bias. COMMENT THIS LINE OUT IF YOU DON'T WANT BIAS READINGS PRINTED OUT
}

//the same as calibrate_gyro() except faster to type
void cg()
{
    calibrate_gyro();
}

//turns the robot to reach a desired theta. 
//If you are expecting this function to work consistantly then don't take your turns too fast.
//The conversions from each wallaby to normal degrees varies but usually ~580000 KIPR degrees = 90 degrees
void turn_with_gyro(int left_wheel_speed, int right_wheel_speed, double targetTheta)
{
    double theta = 0;//declares the variable that stores the current degrees
    mav(right_motor, right_wheel_speed);//starts the motors
    mav(left_motor, left_wheel_speed);
    //keeps the motors running until the robot reaches the desired angle
    while(theta < targetTheta)
    {
        msleep(10);//turns for .01 seconds
        theta += abs(gyro_z() - bias) * 10;//theta = omega(angular velocity, the value returned by gyroscopes) * time
        printf("%d", theta);
    }

    //stops the motors after reaching the turn
    mav(right_motor, 0);
    mav(left_motor, 0);
}

//drives straight forward or backwards. The closer speed is to 0 the faster it will correct itself and the more consistent it will be but just do not go at max speed. Time is in ms. 
void drive_with_gyro(int speed, double time)
{ 
    double startTime = seconds();
    double theta = 0;
    while(seconds() - startTime < (time / 1000.0))
    {
        if(speed > 0)
        {
            mav(right_motor, (double)(speed - speed * (1.920137e-16 + 0.000004470956*theta - 7.399285e-28*pow(theta, 2) - 2.054177e-18*pow(theta, 3) + 1.3145e-40*pow(theta, 4))));  //here at NAR, we are VERY precise
            mav(left_motor, (double)(speed + speed * (1.920137e-16 + 0.000004470956*theta - 7.399285e-28*pow(theta, 2) - 2.054177e-18*pow(theta, 3) + 1.3145e-40*pow(theta, 4))));
        }
        else//reverses corrections if it is going backwards
        {
            mav(right_motor, (double)(speed + speed * (1.920137e-16 + 0.000004470956*theta - 7.399285e-28*pow(theta, 2) - 2.054177e-18*pow(theta, 3) + 1.3145e-40*pow(theta, 4)))); 
            mav(left_motor, (double)(speed - speed * (1.920137e-16 + 0.000004470956*theta - 7.399285e-28*pow(theta, 2) - 2.054177e-18*pow(theta, 3) + 1.3145e-40*pow(theta, 4))));
        }
        //updates theta
        msleep(10);
        theta += (gyro_z() - bias) * 10;
    }
}

void simple_drive_with_gyro(int speed, double time)
{
    double startTime = seconds(); //used to keep track of time
    double theta = 0; //keeps track of how much the robot has turned
    while(seconds() - startTime < time)
    {
        //if the robot is essentially straight then just drive straight
        if(theta < 1000 && theta > -1000)
        {
            mav(right_motor, speed);
            mav(left_motor, speed);
        }
        //if the robot is off to the right then drift to the left
        else if(theta < 1000)
        {
            mav(right_motor, speed + 100);
            mav(left_motor, speed - 100);
        }
        //if the robot is off to the left then drift to the right
        else
        {
            mav(right_motor, speed - 100);
            mav(left_motor, speed + 100);
        }
        //updates theta
        msleep(10);
        theta += (gyro_z() - bias) * 10;
    }
}

//keeps driving until detects black line
void gyro_light_stop(int speed)
{ 
    double startTime = seconds();
    double theta = 0;
    while(LIGHT_0 < THRESHOLD)
    {
        if(speed > 0)
        {
            mav(right_motor, (double)(speed - speed * (1.920137e-16 + 0.000004470956*theta - 7.399285e-28*pow(theta, 2) - 2.054177e-18*pow(theta, 3) + 1.3145e-40*pow(theta, 4))));  //here at NAR, we are VERY precise
            mav(left_motor, (double)(speed + speed * (1.920137e-16 + 0.000004470956*theta - 7.399285e-28*pow(theta, 2) - 2.054177e-18*pow(theta, 3) + 1.3145e-40*pow(theta, 4))));
        }
        else//reverses corrections if it is going backwards
        {
            mav(right_motor, (double)(speed + speed * (1.920137e-16 + 0.000004470956*theta - 7.399285e-28*pow(theta, 2) - 2.054177e-18*pow(theta, 3) + 1.3145e-40*pow(theta, 4)))); 
            mav(left_motor, (double)(speed - speed * (1.920137e-16 + 0.000004470956*theta - 7.399285e-28*pow(theta, 2) - 2.054177e-18*pow(theta, 3) + 1.3145e-40*pow(theta, 4))));
        }
        //updates theta
        msleep(10);
        theta += (gyro_z() - bias) * 10;
    }
}

//moves servo slowly
void slow_servo(int start, int end) {
    int i = 0;
    for(i; i < 30; i++) {
        start += (end - start)/30;
        set_servo_position(0, start);        
        msleep(100);
    }
}
void square_up_black_line () {
    while (LIGHT_0 < THRESHOLD || LIGHT_1 < THRESHOLD) {
        if(LIGHT_0 < THRESHOLD && LIGHT_1 < THRESHOLD) {
            mav(1, -250);
         	mav(0, -250);
            msleep(1);
        }
        
        else if(LIGHT_0 < THRESHOLD && LIGHT_1 > THRESHOLD) {
            mav(1, 250);
            mav(0, -250);
            msleep(1);
        }
        else if (LIGHT_0 > THRESHOLD && LIGHT_1 < THRESHOLD) {
            mav(1, -250);
            mav(0, 250);
            msleep(1);
        } 
    }
}       