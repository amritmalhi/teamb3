#pragma config(Sensor, S1,     sonar,          sensorSONAR)
#pragma config(Sensor, S2,     line,           sensorColorNxtFULL)
#pragma config(Sensor, S3,     line_check,     sensorLightActive)
#pragma config(Sensor, S4,     button,         sensorTouch)
#pragma config(Motor,  motorA,          motor_right,   tmotorNXT, PIDControl, reversed, driveRight, encoder)
#pragma config(Motor,  motorB,          motor_left,    tmotorNXT, PIDControl, reversed, driveLeft, encoder)

bool onLine(int color)
{
	switch (color)
	{
		case 1: return true;
		case 2: return true;
		case 3: return true;
		case 4: return true;
		case 5: return true;
		default: return false;
	}
}

task main()
{
	while (1)
	{
		int distance = SensorValue[sonar];
		while (distance < 20)
		{
			motor[motor_left] = 0;
			motor[motor_right] = 0;
			distance = SensorValue[sonar];
			nxtDisplayTextLine(0, "Stop");
			wait1Msec(300);
		}
		nxtDisplayTextLine(0, "Start");
		int c = SensorValue[line];
		if (onLine(c)) {
			motor[motor_left] = 100;
			motor[motor_right] = 100;
		} else {
			int d = SensorValue[line_check];
			if (d < 44) {
				motor[motor_left] = -30;
				motor[motor_right] = 100;
				wait1Msec(50);
			} else {
				motor[motor_left] = 100;
				motor[motor_right] = -30;
			}
		}
	}
}
