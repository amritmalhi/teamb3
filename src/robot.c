#pragma config(Sensor, S1,     sonar,          sensorSONAR)
#pragma config(Sensor, S2,     line,           sensorColorNxtFULL)
#pragma config(Sensor, S3,     line_check,     sensorLightActive)
#pragma config(Sensor, S4,     button,         sensorTouch)
#pragma config(Motor,  motorA,          motor_right,   tmotorNXT, PIDControl, reversed, driveRight, encoder)
#pragma config(Motor,  motorB,          motor_left,    tmotorNXT, PIDControl, reversed, driveLeft, encoder)
#pragma platform(NXT)


long nLastXmitTimeStamp = nPgmTime;
long nDeltaTime         = 0;

const int kMaxSizeOfMessage = 30;
const int INBOX = 5;

bool stopped;
float deltaTime;

#define MOTOR_SPEED 40
#define MOTOR_SPEED_TURN 100
#define MOTOR_SPEED_TURN_NEGATIVE -10

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

task commands()
{
	TFileIOResult nBTCmdRdErrorStatus;
  int nSizeOfMessage;
  ubyte nRcvBuffer[kMaxSizeOfMessage];

  while (true)
  {
    nSizeOfMessage = cCmdMessageGetSize(INBOX);

    if (nSizeOfMessage > kMaxSizeOfMessage)
      nSizeOfMessage = kMaxSizeOfMessage;
    if (nSizeOfMessage > 0){
    	nBTCmdRdErrorStatus = cCmdMessageRead(nRcvBuffer, nSizeOfMessage, INBOX);
    	nRcvBuffer[nSizeOfMessage] = '\0';
    	string s = "";
    	stringFromChars(s, (char *) nRcvBuffer);
    	nxtDisplayTextLine(1, "%s", s);
    }
    wait1Msec(100);

  }
  return;
}

task DrivingSound()
{
	bPlaySounds = true;
	while(1)
	{
		if (!stopped) {
			playSound(soundFastUpwardTones);
		} else {
			clearSounds();
		}
	}
}

bool found;

void LineFolower()
{
	found = false;
	int oldSystime, newSystime;
	oldSystime = nSysTime;

	while (1)
	{
		while (stopped)
		{
			motor[motor_left] = 0;
			motor[motor_right] = 0;
		}

		newSystime = nSysTime;
		deltaTime = (newSystime - oldSystime) / 1000.0;
		oldSystime = newSystime;

		nxtDisplayTextLine(0, "Start");
		int c = SensorValue[line];
		nxtDisplayTextLine(3, "Color: %f", c);
		if (onLine(c)) {
			motor[motor_left] = MOTOR_SPEED;
			motor[motor_right] = MOTOR_SPEED;
			found = false;
		} else {
			int d = SensorValue[line_check];
			if (d < 44 || found) {
				motor[motor_left] = MOTOR_SPEED_TURN_NEGATIVE;
				motor[motor_right] = MOTOR_SPEED_TURN;
				found = true;
			} else {
				motor[motor_left] = MOTOR_SPEED_TURN;
				motor[motor_right] = MOTOR_SPEED_TURN_NEGATIVE;
			}
		}

		nxtDisplayTextLine(2, "Motor: %f", motor[motor_left]);
	}
}

task ObjectInWay()
{
	while (1)
	{
		int distance = SensorValue[sonar];
		if (distance < 25) {
			stopped = true;
			nxtDisplayTextLine(1, "Object in way %f", distance);
		} else {
			stopped = false;
			nxtDisplayTextLine(1, "No object %f", distance);
		}

		wait1Msec(300);
	}
}

task main()
{
	stopped = false;

	startTask(commands);
	startTask(ObjectInWay);
	startTask(DrivingSound);

	LineFolower();
}
