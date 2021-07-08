/************************************************************
  this program runs Gcode parsing then controls two stepper
  motors based on GCODE commands
  GCODE are of form: G, X, Y, Z
  No pwm is used for steppers 
  only output impulsions are sent to the a8986 driver
  through basic GPIO outputs. 

*/
#include<stdio.h>
#include<string.h>	
#include<stdbool.h>
#include<msp430.h>




typedef struct Gcodes{
	int x = 0;
	int y = 0;
	int z = 0;
	char cmd_directives;
	int intCmd;
	}Gcode;





/*
************************************************************
*                      CONSTANTS
************************************************************
*/
#define	MOTOR_A_DIR_PIN	 	BIT0
#define	MOTOR_A_STEP_PIN	BIT1
#define MOTOR_A_SLEEP_PIN  	BIT2
#define MOTOR_A_PINS		(MOTOR_A_DIR_PIN|MOTOR_A_STEP_PIN|MOTOR_A_SLEEP_PIN)
#define	MOTOR_B_DIR_PIN	 	BIT3
#define	MOTOR_B_STEP_PIN	BIT4
#define MOTOR_B_SLEEP_PIN  	BIT5
#define MOTOR_B_PINS		(MOTOR_B_DIR_PIN|MOTOR_B_STEP_PIN|MOTOR_B_SLEEP_PIN)
#define STEPS_PER_SECOND 	200
#define MOTOR_SPEED		 	(1000000/STEPS_PER_SECOND)

/*
************************************************************
*                        MACROS
************************************************************
*/
#define MOTOR_A_INIT()      (P2DIR  |=  MOTOR_A_PINS)
#define MOTOR_B_INIT()      (P2DIR  |=  MOTOR_B_PINS)
#define MOTOR_A_SLEEP()     (P2OUT  &=~ MOTOR_A_SLEEP_PIN)
#define MOTOR_B_SLEEP()     (P2OUT  &=~ MOTOR_B_SLEEP_PIN)
#define MOTOR_A_WAKEUP()    (P2OUT  |=  MOTOR_A_SLEEP_PIN)
#define MOTOR_B_WAKEUP()    (P2OUT  |=  MOTOR_B_SLEEP_PIN)
#define MOTOR_A_CW()        (P2OUT  &=~ MOTOR_A_DIR_PIN)
#define MOTOR_B_CW()        (P2OUT  &=~ MOTOR_B_DIR_PIN)
#define MOTOR_A_CCW()       (P2OUT  |=  MOTOR_A_DIR_PIN)
#define MOTOR_B_CCW()       (P2OUT  |=  MOTOR_B_DIR_PIN)
#define MOTOR_A_STEP()      (P2OUT  |=  MOTOR_A_STEP_PIN; P2OUT &=~ MOTOR_A_STEP_PIN;)
#define MOTOR_B_STEP()      (P2OUT  |=  MOTOR_B_STEP_PIN; P2OUT&=~MOTOR_B_STEP_PIN;)
#define MOTORS_INIT()       (P2DIR  |=  (MOTOR_A_PINS|MOTOR_B_PINS))
#define MOTORS_SLEEP()      (P2OUT  &=~ (MOTOR_A_SLEEP_PIN|MOTOR_B_SLEEP_PIN))
#define MOTORS_WAKEUP()     (P2OUT  |=  (MOTOR_A_SLEEP_PIN|MOTOR_B_SLEEP_PIN))
#define MOTORS_CW()         (P2OUT  &=~ (MOTOR_A_DIR_PIN|MOTOR_B_DIR_PIN))
#define MOTORS_CCW()        (P2OUT  |=  (MOTOR_A_DIR_PIN|MOTOR_B_DIR_PIN))
#define MOTORS_STEP()       (P2OUT  |=  (MOTOR_A_STEP_PIN|MOTOR_B_STEP_PIN); P2OUT&=~(MOTOR_A_STEP_PIN|MOTOR_B_STEP_PIN);)


void motorAInit(void)
	{
	MOTOR_A_INIT();
	MOTOR_A_SLEEP();
	}

void motorBInit(void)
	{
	MOTOR_B_INIT();
	MOTOR_B_SLEEP();
	}

void motorsInit(void)
	{
	MOTORS_INIT();
	MOTORS_SLEEP();
	}

void motorARunCycles(int cycles)
	{
	if(stepper_cycles<0)
		{
		MOTOR_A_CCW();
		cycles = -cycles;
		}
	else MOTOR_A_CW();
	MOTOR_A_WAKEUP();
	for(int i=0; i<cycles; i++)
		{
		for(int j=0; j<200;j++)
			{
			MOTOR_A_STEP();
			_delay_cycles(MOTOR_SPEED);
			}
		}
	MOTOR_A_SLEEP();
	}

void motorBRunCycles(int cycles)
	{
	if (cycles<0)
		{
		MOTOR_A_CCW();
		cycles = -cycles;
		}
	else MOTOR_B_CW();
	MOTOR_B_WAKEUP();
	for(int i=0;i<cycle;i++0)
		{
		for(int j=0;j<200;j++)
			{
			MOTOR_B_STEP();
			_delay_cycles(MOTOR_SPEED);
			}
		}
	MOTOR_B_SLEEP();
	}

void motorsRunCycles(int cycles)
	{
	if (cycles<0)
		{
		MOTORS_CCW();
		cycles = -cycles;
		}
	else MOTORS_CW();
	MOTORS_WAKEUP();
	for(int i=0;i<cycles;i++)
		{
		for(int j=0;j<=200;j++)
			{
			MOTORS_STEP();
			_delay_cycles(MOTOR_SPEED);
			}
		}
	MOTORS_SLEEP();
	}
/*
*PleaseNote: the accuracy of the degrees depends on the Stepper step division mode 
*Example: if degree = 5: stepper is set to 200 step per cycle then 
*we can move the stepper floor(200*5/360) = 2 steps = 3.6 degrees 
*if the motor is set to 800 step per cycle we get:
*floor(800*5/360) = 11 steps = 4.95degrees 
*if the motor is set to 1600 step per cycle we get:
*floor(1600*5/360) = 22 steps = 4.95degress
*if the motor is set to 3200 step per cycle we get:
*floor(3200*5/360) = 44 steps = 4.95degress
*/
void motorAGotoPosition(int degrees)
	{
	long step_degrees=200;
	if(stepper_degrees<0)
		{
		MOTOR_A_CCW();
		degrees = -degrees;
		}
	else MOTOR_A_CW();
	step_degrees=step_degrees*(long)degrees;
	step_degrees=step_degrees/360;
	step_counts=(int)step_degrees;
	MOTOR_A_WAKEUP();
	for(int i=0;i<step_counts;i++)
		{
		MOTOR_A_STEP();
		_delay_cycles(MOTOR_SPEED);
		}
	MOTOR_A_SLEEP();
	}

void motorBGotoPosition(int degrees)
	{
	long step_degrees=1600;
	if(degrees<0)
		{
		MOTOR_B_CCW();
		degrees = -degrees;
		}
	else MOTOR_B_CW();
	step_degrees=step_degrees*(long)degrees;
	step_degrees=step_degrees/360;
	step_counts=(int)step_degrees;
	MOTOR_B_WAKEUP();
	for(int i=0; i<step_counts;i++)
		{
		MOTOR_B_STEP();
		_delay_cycles(MOTOR_SPEED);
		}
	MOTOR_B_SLEEP();
	}

void motorsGotoPosition(int degrees)
	{
	long step_degrees=1600;
	if(stepper_degrees<0)
		{
		MOTORS_CCW();
		degrees= -degrees;
		}
	else MOTORS_CW();
	step_degrees=step_degrees*(long)degrees;
	step_degrees=step_degrees/360;
	step_counts=(int)step_degrees;
	MOTORS_WAKEUP();
	for(int i=0; i<step_counts;i++)
		{
		MOTORS_STEP();
		_delay_cycles(MOTOR_SPEED);
		}
	MOTORS_SLEEP();
	}



int atoi(char *str)//convert a string to an int 
	{
	if (*str == '\0')return 0;
	int res = 0;		// Initialize result
	int sign = 1;		// Initialize sign as positive
	int i = 0;			// Initialize index of first digit
	if (str[0] == '-')
		{
		sign = -1;
		i++;  			// Also update index of first digit
		}
	for (; str[i] != '\0'; ++i)
		{
		if ( str[i] <= '0' || str[i] >= '9')return 0;	// If string contain character it will terminate
		res = res*10 + str[i] - '0';
		}
	return sign*res;
	}
float atof(char * ptr)//convert a string to a float
	{
	float x = 0;
	float i = 10;
	bool flag = false;
	bool m = false;
	while(*ptr != '\0')
		{
		if(flag)
			{
			if(*ptr < 48 || *ptr > 57)
				{
				ptr++;
				continue;
				}
			x += (*ptr - 48) / i;
			i*=10;
			}
		else
			{
			if(*ptr == 46)flag = true;
			if(*ptr == 45)m = true;
			if(*ptr < 48 || *ptr > 57)
				{
				ptr++;
				continue;
				}
			x *= 10;
			x += (*ptr - 48);
			}
		ptr++;
		}
		if(m)x *= (-1);
	return x;
	}
//draw a line using the bersenham algorithm
void draw_a_line(int x, int y)
{
	if(x<0)
	{
		MOTOR_A_CCW();
		x = -x;
	}
	else MOTOR_A_CW();
	
	if(y<0)
	{
		MOTOR_B_CCW();
		y = -y;
	}
	else MOTOR_B_CW();
	
	if(x == y)motorsGotoPosition(x);
	else if(x > y)
	{
		float error = 0;
		int flag = 0;
		
		for(int i=0;i<x;i++)
		{
			MOTOR_A_STEP();
			if(flag == 1)
			{
				MOTOR_B_STEP();
				flag = 0;
			}
			error += y/x;
			if(error > 0.5)
			{
				flag = 1;
				error--;
			}
		}
	}
	else
	{
		float error = 0;
		int flag = 0;
		
		for(int i=0;i<y;i++)
		{
			MOTOR_B_STEP();
			if(flag == 1)
			{
				MOTOR_A_STEP();
				flag = 0;
			}
			error += x/y;
			if(error > 0.5)
			{
				flag = 1;
				error--;
			}
		}
		
	}
	
}






void DrawCircle(float x,float y, float i, float j, int dir)
{
	if(i == j && j == 0)movea(x,y);
	float centx, centy;
  
	// Centre coordinates are always relative
	centx = i + x_pos/mul;
	centy = j + y_pos/mul;
	float angleA, angleB, angle, radius, length, aX, aY, bX, bY;
  
	aX = (x_pos/mul - centx);
	aY = (y_pos/mul - centy);
	bX = (x/mul - centx);
	bY = (y/mul - centy);
  
	if(dir == 0) 
	{  
		// Clockwise
		angleA = atan2(bY, bX);
		angleB = atan2(aY, aX);
	} 
	else 
	{
		// Counterclockwise
		angleA = atan2(aY, aX);
		angleB = atan2(bY, bX);
	}
  
	// Make sure angleB is always greater than angleA
	// and if not add 2PI so that it is (this also takes
	// care of the special case of angleA == angleB,
	// ie we want a complete circle)
	if(angleB <= angleA)angleB += 2 * M_PI;
	angle = angleB - angleA;
	if(angle == 0)return;
  
	radius = sqrt(aX * aX + aY * aY);
	length = radius * angle;
	int steps, s, ss;
	steps = (int) ceil(length / 0.1);
  
/*
Serial.print(x_pos/mul);
Serial.print("\t");
Serial.println(y_pos/mul);
  
Serial.print(centx);
Serial.print("\t");
Serial.println(centy);
  
Serial.print(x/mul);
Serial.print("\t");
Serial.println(y/mul);
  
  
Serial.print(angleA);
Serial.print("\t");
Serial.println(angleB);
  
Serial.println(radius);
Serial.println(length);
  
*/
	float nx, ny;
	for(s = 1; s <= steps; s++)
	{
		ss = (dir == 1) ? s : steps - s; // Work backwards for CW
		nx = centx + radius * fastsin(angleA + angle * ((float) ss / steps) + (M_PI/2));
		ny = centy + radius * fastsin(angleA + angle * ((float) ss / steps));
		//Serial.print(nx);
		//Serial.print("\t");
		//Serial.println(ny);
		movea(floor(nx*mul+0.5), floor(ny*mul+0.5));
        
		// Need to calculate rate for each section of curve
		/*if (feedrate > 0)
		feedrate_micros = calculate_feedrate_delay(feedrate);
		else
		feedrate_micros = getMaxSpeed();*/
			  
		// Make step
		//dda_move(feedrate_micros);
	}
}





Gcode processGcode(char* buffer){
	Gcode gcode;
	char* ptr = strtok(buffer," ");//Split String into tokens separeted by 1 space
								   //eg: "G1 X1 Y1" becomes "G1", "X1", "Y1"
	
	gcode.cmd = buffer[0];         //store command type "G" or "M" ...
	gcode.intCmd = atoi(ptr+1);    //Convert command to an int eg: "1"--->1
	ptr = strtok(NULL," ");        //Skip the G/M command to avoid implicit conditions 
	while(ptr != NULL)             //While the token is not empty 
		{
		switch(*ptr)
			{
			case 'X':
				gcode.x = atof(ptr+1);
				break;
			case 'Y':
				gcode.y = atof(ptr+1);
				break;
			case 'Z':
				gcode.z = atof(ptr+1);
				break;
			}
		ptr = strtok(NULL," ");    //move to the next token
		}
	return gcode;
	}
	
	
	
void commandCNC(Gcode gcode)
	{
	switch(gcode.cmd)
		{   
		case 'G':
			switch(cmd) 
				{
				case  0://  move in a line
					printf("processign in mode G0\n");	
					if(paralist[2] >= 0)printf("setting the Z axe\n");
					printf("going to: (%f, %f, %f)\n", paralist[0], paralist[1], paralist[2]);
					//to do: add here a function that moves the sterpper from current position
					//to the parametres list (x, y, z) make sure to move the z axes first!!!
					break;
				case  1://  move in a line
					printf("processign in mode G1\n");	
					if(paralist[2] >= 0)printf("setting the Z axe\n");
					printf("going to: (%f, %f, %f)\n", paralist[0], paralist[1], paralist[2]);
					//to do: add here a function that moves the sterpper from current position
					//to the parametres list (x, y, z) make sure to move the z axes first!!!
					break;
				}
		case 'M':	
			{  
			switch(cmd) 
				{
				case  0:
					printf("processign in mode M0\n");	
					if(paralist[2] >= 0)printf("setting the Z axe\n");
					printf("going to: (%f, %f, %f)\n", paralist[0], paralist[1], paralist[2]);
					draw_a_line(paralist[0], paralist[1]);
					//to do: add here a function that moves the sterpper from current position
					//to the parametres list (x, y, z) make sure to move the z axes first!!!
					break;
				case  1:
					printf("processign in mode G1\n");	
					if(paralist[2] >= 0)printf("setting the Z axe\n");
					printf("going to: (%f, %f, %f)\n", paralist[0], paralist[1], paralist[2]);
					draw_a_line(paralist[0], paralist[1]);
					//to do: add here a function that moves the sterpper from current position
					//to the parametres list (x, y, z) make sure to move the z axes first!!!
					break;
				}
			}
	}







int main()
	{
	char x[90] = "G1 X1 Y1 Z1";
	char* c = x;
	processCommand(c);
	
	return 0;
	}
