#pragma config(Sensor, S1,     sonar,          sensorSONAR)
#pragma config(Sensor, S2,     line,           sensorColorNxtFULL)
#pragma config(Sensor, S3,     line_check,     sensorLightActive)
#pragma config(Sensor, S4,     button,         sensorTouch)
#pragma config(Motor,  motorA,          motor_right,   tmotorNXT, PIDControl, reversed, driveRight, encoder)
#pragma config(Motor,  motorB,          motor_left,    tmotorNXT, PIDControl, reversed, driveLeft, encoder)
#pragma config(Motor,  motorC,          motor_shoot,   tmotorNXT, PIDControl, reversed, encoder)

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
int dirlist[32];
int dirlist_index = 0;
bool goto_destination = false;
int intersection_count = 0;

#define DIR_N 0
#define DIR_E 1
#define DIR_S 2
#define DIR_W 3

int posx = 0;
int posy = 0;
int grid_direction = DIR_N;

#define MOTOR_SPEED 60
#define MOTOR_SPEED_TURN 80
#define MOTOR_SPEED_TURN_NEGATIVE -25
#define MOTOR_SPEED_TURN_SHARP MOTOR_SPEED_TURN
#define MOTOR_SPEED_TURN_SHARP_NEGATIVE MOTOR_SPEED_TURN_NEGATIVE
#define ROTATE_TIME 2.0
#define ROTATE_TIME_SHARP 3.0

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

void update_position(int turn)
{
	if (grid_direction == DIR_N) {
		posx++;
		if (turn == INT_LEFT) {
			grid_direction = DIR_W;
		} else if (turn == INT_RIGHT) {
			grid_direction = DIR_E;
		}
	} else if (grid_direction == DIR_S) {
		posx--;
		if (turn == INT_LEFT) {
			grid_direction = DIR_E;
		} else if (turn == INT_RIGHT) {
			grid_direction = DIR_W;
		}
	} else if (grid_direction == DIR_W) {
		posy++;
		if (turn == INT_LEFT) {
			grid_direction = DIR_S;
		} else if (turn == INT_RIGHT) {
			grid_direction = DIR_N;
		}
	} else {
		posy--;
		if (turn == INT_LEFT) {
			grid_direction = DIR_N;
		} else if (turn == INT_RIGHT) {
			grid_direction = DIR_S;
		}
	}

	nxtDisplayTextLine(5, "x: %i y: %i", posx, posy);
}

void clear_direction_array()
{
	for (int i = 0; i < 32; i++) {
		dirlist[i] = 0;
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
    		nxtDisplayTextLine(5, "going");
    		commandStopped = false;
    	} else if (s == "DOWN") {
    		nxtDisplayTextLine(5, "stop");
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

int get_motor_speed(float percent, int begin, int end)
{
	percent = 1 - (percent - 1) * (percent - 1) * (percent - 1) * (percent - 1);

	int diff = end - begin;
	percent = percent * diff;
	return begin + percent;
}

bool found;

void LineFolower()
{
	found = false;

	int old_system_time = nSysTime;
	int new_system_time;
	float deltaTime;
	float rotateTime = 0.0;
	float stopped_time = 0.0;
	float rotate_time_total = 0.0;
	bool was_intersection = false;

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

		nxtDisplayTextLine(6, "Intersection: %i", intersection_count);
		nxtDisplayTextLine(7, "%f", stopped_time);
		nxtDisplayTextLine(0, "Start");
		int c = SensorValue[line];
		int d = SensorValue[line_check];
		nxtDisplayTextLine(3, "Color: %f", c);

		if (onLine(c)) {
			motor[motor_left] = MOTOR_SPEED - (stopped_time * (MOTOR_SPEED / STOP_SECONDS));
			motor[motor_right] = MOTOR_SPEED - (stopped_time * (MOTOR_SPEED / STOP_SECONDS));
			found = false;
			rotateTime = 0.0;
			rotate_time_total = 0.0;

			if (d < 44) {
				was_intersection = true;

				if (direction == 0) {
						motor[motor_left] = 0;
						motor[motor_right] = 0;
						while(1){
							motor[motor_left] = 0;
							motor[motor_right] = 0;
						}
				}

				if (direction == INT_STRAIGHT) {
					update_position(INT_STRAIGHT);
				} else if (direction == INT_LEFT) {
					update_position(INT_LEFT);
					motor[motor_left] = MOTOR_SPEED;
					motor[motor_right] = MOTOR_SPEED;
					wait1Msec(150);
					motor[motor_left] = MOTOR_SPEED_TURN_NEGATIVE;
					motor[motor_right] = MOTOR_SPEED_TURN;
					wait1Msec(700);
					c = SensorValue[line];
					while (!onLine(c))
					{
						c = SensorValue[line];
					}
				} else {
					update_position(INT_RIGHT);
					motor[motor_left] = MOTOR_SPEED;
					motor[motor_right] = MOTOR_SPEED;
					wait1Msec(150);
					motor[motor_left] = MOTOR_SPEED_TURN;
					motor[motor_right] = MOTOR_SPEED_TURN_NEGATIVE;
					wait1Msec(700);
					c = SensorValue[line];
					while (!onLine(c))
					{
						c = SensorValue[line];
					}
				}
			}
		} else {
			if (was_intersection) {
				was_intersection = false;
				intersection_count++;
				if (goto_destination) {
					direction = dirlist[dirlist_index];
					dirlist_index++;
				}
			}

			rotateTime += deltaTime;
			rotate_time_total += deltaTime;
			if (rotateTime > ROTATE_TIME) {
				rotateTime = ROTATE_TIME;
			}
			float rotate_percent = rotateTime / ROTATE_TIME;

			if (d < 44 || found) {
				if (rotate_time_total > ROTATE_TIME_SHARP) {
					motor[motor_left] = MOTOR_SPEED_TURN_SHARP_NEGATIVE;
					motor[motor_right] = MOTOR_SPEED_TURN_SHARP;
				} else {
					motor[motor_left] = get_motor_speed(rotate_percent, MOTOR_SPEED, MOTOR_SPEED_TURN_NEGATIVE - (stopped_time * (MOTOR_SPEED_TURN_NEGATIVE / STOP_SECONDS)));
					motor[motor_right] = get_motor_speed(rotate_percent, MOTOR_SPEED, MOTOR_SPEED_TURN - (stopped_time * (MOTOR_SPEED_TURN / STOP_SECONDS)));
				}
				found = true;
			} else {
				if (rotate_time_total > ROTATE_TIME_SHARP) {
					motor[motor_left] = MOTOR_SPEED_TURN_SHARP;
					motor[motor_right] = MOTOR_SPEED_TURN_SHARP_NEGATIVE;
				} else {
					motor[motor_left] = get_motor_speed(rotate_percent, MOTOR_SPEED, MOTOR_SPEED_TURN - (stopped_time * (MOTOR_SPEED_TURN / STOP_SECONDS)));
					motor[motor_right] = get_motor_speed(rotate_percent, MOTOR_SPEED, MOTOR_SPEED_TURN_NEGATIVE - (stopped_time * (MOTOR_SPEED_TURN_NEGATIVE / STOP_SECONDS)));
				}
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
			wait1Msec(STOP_SECONDS * 1000 + 500);
		} else {
			objectStopped = false;
			nxtDisplayTextLine(1, "No object %f", distance);
		}

		wait1Msec(300);
	}
}

void shoot()
{
	motor[motor_shoot] = 100;
	wait1Msec(20000);
	motor[motor_shoot] = 0;
}

task main()
{
	clear_direction_array();
	dirlist[0] = INT_LEFT;
	dirlist[1] = INT_RIGHT;
	dirlist[2] = INT_LEFT;
	dirlist[3] = INT_LEFT;
	dirlist[4] = INT_RIGHT;
	dirlist[5] = INT_RIGHT;
	dirlist[6] = INT_RIGHT;
	dirlist[7] = INT_LEFT;
	goto_destination = true;
	direction = dirlist[0];
	dirlist_index = 1;

	stopped = false;
	objectStopped = false;
	commandStopped = false;
	//direction = INT_STRAIGHT;

	startTask(commands);
	startTask(ObjectInWay);
	startTask(DrivingSound);

	LineFolower();
}
