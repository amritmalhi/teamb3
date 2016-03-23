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
bool objectStopped;
bool commandStopped;
float deltaTime;

int direction;

#define MOTOR_SPEED 55
#define MOTOR_SPEED_TURN 100
#define MOTOR_SPEED_TURN_NEGATIVE -10

#define STOP_SECONDS 1.0

#define INT_STRAIGHT 1
#define INT_LEFT 2
#define INT_RIGHT 3

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
    	nxtDisplayTextLine(2, "%s", s);

    	if (s == "FIRE") {
    		nxtDisplayTextLine(3, "going");
    		commandStopped = false;
    	} else if (s == "DOWN") {
    		nxtDisplayTextLine(3, "stop");
    		commandStopped = true;
    	} else if (s == "LEFT") {
    		direction = INT_LEFT;
    	} else if (s == "UP") {
    		direction = INT_STRAIGHT;
    	} else if (s == "RIGHT") {
    		direction = INT_RIGHT;
    	}
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
			PlayTone(695, 14); while(bSoundActive && !stopped);
			PlayTone(695, 14); while(bSoundActive && !stopped);
			PlayTone(695, 14); while(bSoundActive && !stopped);
			PlayTone(929, 83); while(bSoundActive && !stopped);
			PlayTone(1401, 83); while(bSoundActive && !stopped);
			PlayTone(1251, 14); while(bSoundActive && !stopped);
			PlayTone(1188, 14); while(bSoundActive && !stopped);
			PlayTone(1054, 14); while(bSoundActive && !stopped);
			PlayTone(1841, 83); while(bSoundActive && !stopped);
			PlayTone(1401, 41); while(bSoundActive && !stopped);
			PlayTone(1251, 14); while(bSoundActive && !stopped);
			PlayTone(1188, 14); while(bSoundActive && !stopped);
			PlayTone(1054, 14); while(bSoundActive && !stopped);
			PlayTone(1841, 83); while(bSoundActive && !stopped);
			PlayTone(1401, 41); while(bSoundActive && !stopped);
			PlayTone(1251, 14); while(bSoundActive && !stopped);
			PlayTone(1188, 14); while(bSoundActive && !stopped);
			PlayTone(1251, 14); while(bSoundActive && !stopped);
			PlayTone(1054, 55); while(bSoundActive && !stopped);
			wait1Msec(280);
			PlayTone(695, 14); while(bSoundActive && !stopped);
			PlayTone(695, 14); while(bSoundActive && !stopped);
			PlayTone(695, 14); while(bSoundActive && !stopped);
			PlayTone(929, 83); while(bSoundActive && !stopped);
			PlayTone(1401, 83); while(bSoundActive && !stopped);
			PlayTone(1251, 14); while(bSoundActive && !stopped);
			PlayTone(1188, 14); while(bSoundActive && !stopped);
			PlayTone(1054, 14); while(bSoundActive && !stopped);
			PlayTone(1841, 83); while(bSoundActive && !stopped);
			PlayTone(1401, 41); while(bSoundActive && !stopped);
			PlayTone(1251, 14); while(bSoundActive && !stopped);
			PlayTone(1188, 14); while(bSoundActive && !stopped);
			PlayTone(1054, 14); while(bSoundActive && !stopped);
			PlayTone(1841, 83); while(bSoundActive && !stopped);
			PlayTone(1401, 41); while(bSoundActive && !stopped);
			PlayTone(1251, 14); while(bSoundActive && !stopped);
			PlayTone(1188, 14); while(bSoundActive && !stopped);
			PlayTone(1251, 14); while(bSoundActive && !stopped);
			PlayTone(1054, 55); while(bSoundActive && !stopped);
		} else {
			clearSounds();
		}
	}
}

bool found;

void LineFolower()
{
	found = false;

	int old_system_time = nSysTime;
	int new_system_time;
	float deltaTime;
	float stopped_time = 0.0;

	int rl, rr;

	while (1)
	{
		if (objectStopped || commandStopped) {
			stopped = true;
		} else {
			stopped = false;
		}

		new_system_time = nSysTime;
		deltaTime = (new_system_time - old_system_time) / 1000.0;
		old_system_time = new_system_time;

		if (stopped) {
			stopped_time += deltaTime;
		} else {
			stopped_time = 0.0;
		}
		if (stopped_time > STOP_SECONDS) {
			stopped_time = STOP_SECONDS;
		}

		nxtDisplayTextLine(7, "%f", stopped_time);

		/*while (stopped)
		{
			int ml, mr;
			ml = motor[motor_left];
			mr = motor[motor_right];

			ml -= rl;
			mr -= rr;
			if (ml < 0) ml = 0;
			if (mr < 0) mr = 0;

			nxtDisplayTextLine(6, "%i = %i", rl, rr);

			motor[motor_left] = ml;
			motor[motor_right] = mr;

			if (objectStopped || commandStopped) {
			stopped = true;
			} else {
				stopped = false;
			}

			wait1Msec(250);
		}*/

		nxtDisplayTextLine(0, "Start");
		int c = SensorValue[line];
		nxtDisplayTextLine(3, "Color: %f", c);
		if (onLine(c)) {
			motor[motor_left] = MOTOR_SPEED - (stopped_time * (MOTOR_SPEED / STOP_SECONDS));
			motor[motor_right] = MOTOR_SPEED - (stopped_time * (MOTOR_SPEED / STOP_SECONDS));
			found = false;
			int d = SensorValue[line_check];
			if (d < 44) {
				nxtDisplayTextLine(6, "Intersection");
				if (direction == INT_STRAIGHT) {

				} else if (direction == INT_LEFT) {
					motor[motor_left] = MOTOR_SPEED_TURN_NEGATIVE;
					motor[motor_right] = MOTOR_SPEED_TURN;
					wait1Msec(1800);
				} else {
					motor[motor_left] = MOTOR_SPEED_TURN;
					motor[motor_right] = MOTOR_SPEED_TURN_NEGATIVE;
					wait1Msec(1800);
				}
			}
		} else {
			int d = SensorValue[line_check];
			if (d < 44 || found) {
				motor[motor_left] = MOTOR_SPEED_TURN_NEGATIVE - (stopped_time * (MOTOR_SPEED_TURN_NEGATIVE / STOP_SECONDS));
				motor[motor_right] = MOTOR_SPEED_TURN - (stopped_time * (MOTOR_SPEED_TURN / STOP_SECONDS));
				found = true;
			} else {
				motor[motor_left] = MOTOR_SPEED_TURN - (stopped_time * (MOTOR_SPEED_TURN / STOP_SECONDS));
				motor[motor_right] = MOTOR_SPEED_TURN_NEGATIVE - (stopped_time * (MOTOR_SPEED_TURN_NEGATIVE / STOP_SECONDS));
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
			objectStopped = true;
			nxtDisplayTextLine(1, "Object in way %f", distance);
		} else {
			objectStopped = false;
			nxtDisplayTextLine(1, "No object %f", distance);
		}

		wait1Msec(300);
	}
}

task main()
{
	stopped = false;
	objectStopped = false;
	commandStopped = false;
	direction = INT_STRAIGHT;

	startTask(commands);
	startTask(ObjectInWay);
	startTask(DrivingSound);

	LineFolower();
}
