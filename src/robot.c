#pragma config(Sensor, S1,     sonar,          sensorSONAR)
#pragma config(Sensor, S2,     line,           sensorColorNxtFULL)
#pragma config(Sensor, S3,     line_check,     sensorLightActive)
#pragma config(Sensor, S4,     button,         sensorTouch)
#pragma config(Motor,  motorA,          motor_right,   tmotorNXT, PIDControl, reversed, driveRight, encoder)
#pragma config(Motor,  motorB,          motor_left,    tmotorNXT, PIDControl, reversed, driveLeft, encoder)
#pragma config(Motor,  motorC,          motor_shoot,   tmotorNXT, PIDControl, reversed, encoder)

#pragma platform(NXT)

#define MAP_SIZE 5
#define INT_STRAIGHT 1
#define INT_LEFT 2
#define INT_RIGHT 3
#define INT_U_TURN 4
#define FIND_SIZE MAP_SIZE * MAP_SIZE + 1

#define DESTINATION_A_X 0
#define DESTINATION_A_Y 4
#define DESTINATION_B_X 4
#define DESTINATION_B_Y 4
#define DESTINATION_C_X 4
#define DESTINATION_C_Y 0

int DEST_X = -1;
int DEST_Y = -1;

typedef struct grid_node
{
    int posx;
    int posy;
    struct grid_node* north;
    struct grid_node* south;
    struct grid_node* east;
    struct grid_node* west;
} grid_node_t;

typedef struct pathfind
{
    int posx;
    int posy;
    int dir;
    int todo;
    int count;
    struct pathfind* prev;
} pathfind_t;

grid_node_t GRID_MAP[MAP_SIZE][MAP_SIZE];
unsigned char GRID_MAP_VISITED[MAP_SIZE + 1][MAP_SIZE + 1];
pathfind_t PATHFIND[FIND_SIZE];
int PATH_IND;

long nLastXmitTimeStamp = nPgmTime;
long nDeltaTime         = 0;

const int kMaxSizeOfMessage = 30;
const int INBOX = 5;

bool stopped;
bool objectStopped;
bool commandStopped;
float deltaTime;

int SHOOT_COUNT = 0;

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
int TARGET_X = 0;
int TARGET_Y = 0;

#define MOTOR_SPEED 60
#define MOTOR_SPEED_TURN 80
#define MOTOR_SPEED_TURN_NEGATIVE -25
#define MOTOR_SPEED_TURN_SHARP MOTOR_SPEED_TURN
#define MOTOR_SPEED_TURN_SHARP_NEGATIVE MOTOR_SPEED_TURN_NEGATIVE
#define ROTATE_TIME 2.0
#define ROTATE_TIME_SHARP 3.0

#define STOP_SECONDS 1.0

void initialize_map()
{
    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            grid_node_t* grid_node = &GRID_MAP[i][j];
            grid_node->posx = i;
            grid_node->posy = j;

            if (i + 1 < MAP_SIZE) {
                grid_node->north = &GRID_MAP[i + 1][j];
            } else {
                grid_node->north = NULL;
            }

            if (i - 1 >= 0) {
                grid_node->south = &GRID_MAP[i - 1][j];
            } else {
                grid_node->south = NULL;
            }

            if (j + 1 < MAP_SIZE) {
                grid_node->west = &GRID_MAP[i][j + 1];
            } else {
                grid_node->west = NULL;
            }

            if (j - 1 >= 0) {
                grid_node->east = &GRID_MAP[i][j - 1];
            } else {
                grid_node->east = NULL;
            }
        }
    }
}

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

void shoot()
{
	motor[motor_shoot] = 60;
	nMotorEncoder[motor_shoot] = 0;
	while (nMotorEncoder[motor_shoot] < 360) {
		wait1Msec(10);
	}
	motor[motor_shoot] = 0;
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

void create_route(pathfind_t* pathfind)
{
	clear_direction_array();
	dirlist_index = 1;

    dirlist[pathfind->count + 1] = 0;
    while (pathfind != NULL)
    {
        if (pathfind->count >= 0) {
            if (pathfind->prev->dir == pathfind->dir) {
                dirlist[pathfind->count] = INT_STRAIGHT;
            } else if (pathfind->prev->dir == DIR_N && pathfind->dir == DIR_E) {
                dirlist[pathfind->count] = INT_RIGHT;
            } else if (pathfind->prev->dir == DIR_E && pathfind->dir == DIR_S) {
                dirlist[pathfind->count] = INT_RIGHT;
            } else if (pathfind->prev->dir == DIR_S && pathfind->dir == DIR_W) {
                dirlist[pathfind->count] = INT_RIGHT;
            } else if (pathfind->prev->dir == DIR_W && pathfind->dir == DIR_N) {
                dirlist[pathfind->count] = INT_RIGHT;
            } else if (pathfind->prev->dir == DIR_N && pathfind->dir == DIR_S) {
                dirlist[pathfind->count] = INT_U_TURN;
            } else if (pathfind->prev->dir == DIR_S && pathfind->dir == DIR_N) {
                dirlist[pathfind->count] = INT_U_TURN;
            } else if (pathfind->prev->dir == DIR_E && pathfind->dir == DIR_W) {
                dirlist[pathfind->count] = INT_U_TURN;
            } else if (pathfind->prev->dir == DIR_W && pathfind->dir == DIR_E) {
                dirlist[pathfind->count] = INT_U_TURN;
            } else {
                dirlist[pathfind->count] = INT_LEFT;
            }
        }

        pathfind = pathfind->prev;
    }

    direction = dirlist[0];
}

void create_path(int targetx, int targety)
{
	TARGET_X = targetx;
	TARGET_Y = targety;
    for (int i = 0; i < FIND_SIZE; i++) {
        PATHFIND[i].todo = 0;
    }
    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            GRID_MAP_VISITED[i][j] = 0;
        }
    }

    PATH_IND = 0;

    int posx_shift = 0;
    int posy_shift = 0;
    if (grid_direction == DIR_N) {
    	posx_shift = 1;
    } else if (grid_direction == DIR_S) {
    	posx_shift = -1;
  	} else if (grid_direction == DIR_W) {
  		posy_shift = 1;
  	} else {
  		posy_shift = -1;
  	}

    PATHFIND[PATH_IND].dir = grid_direction;
    PATHFIND[PATH_IND].posx = posx + posx_shift;
    PATHFIND[PATH_IND].posy = posy + posy_shift;
    PATHFIND[PATH_IND].prev = NULL;
    PATHFIND[PATH_IND].todo = 1;
    PATHFIND[PATH_IND].count = -1;
    PATH_IND++;

    while (1)
    {
        pathfind_t* pathfind = NULL;
        int shortest_count = 1024;
        for (int i = 0; i < FIND_SIZE; i++) {
            if (PATHFIND[i].todo == 1 && PATHFIND[i].count < shortest_count) {
                pathfind = &PATHFIND[i];
                shortest_count = pathfind->count;
            }
        }

        if (pathfind == NULL) {
            break;
        }

        if (pathfind->posx == targetx && pathfind->posy == targety) {
            create_route(pathfind);
            break;
        }

        if (GRID_MAP[pathfind->posx][pathfind->posy].north != NULL && GRID_MAP_VISITED[pathfind->posx + 1][pathfind->posy] == 0) {
            GRID_MAP_VISITED[pathfind->posx + 1][pathfind->posy] = 1;
            PATHFIND[PATH_IND].posx = pathfind->posx + 1;
            PATHFIND[PATH_IND].posy = pathfind->posy;
            PATHFIND[PATH_IND].dir = DIR_N;
            PATHFIND[PATH_IND].prev = pathfind;
            PATHFIND[PATH_IND].todo = 1;
            PATHFIND[PATH_IND].count = pathfind->count + 1;
            PATH_IND++;
        }

        if (GRID_MAP[pathfind->posx][pathfind->posy].south != NULL && GRID_MAP_VISITED[pathfind->posx - 1][pathfind->posy] == 0) {
            GRID_MAP_VISITED[pathfind->posx - 1][pathfind->posy] = 1;
            PATHFIND[PATH_IND].posx = pathfind->posx - 1;
            PATHFIND[PATH_IND].posy = pathfind->posy;
            PATHFIND[PATH_IND].dir = DIR_S;
            PATHFIND[PATH_IND].prev = pathfind;
            PATHFIND[PATH_IND].todo = 1;
            PATHFIND[PATH_IND].count = pathfind->count + 1;
            PATH_IND++;
        }

        if (GRID_MAP[pathfind->posx][pathfind->posy].west != NULL && GRID_MAP_VISITED[pathfind->posx][pathfind->posy + 1] == 0) {
            GRID_MAP_VISITED[pathfind->posx][pathfind->posy + 1] = 1;
            PATHFIND[PATH_IND].posx = pathfind->posx;
            PATHFIND[PATH_IND].posy = pathfind->posy + 1;
            PATHFIND[PATH_IND].dir = DIR_W;
            PATHFIND[PATH_IND].prev = pathfind;
            PATHFIND[PATH_IND].todo = 1;
            PATHFIND[PATH_IND].count = pathfind->count + 1;
            PATH_IND++;
        }

        if (GRID_MAP[pathfind->posx][pathfind->posy].east != NULL && GRID_MAP_VISITED[pathfind->posx][pathfind->posy - 1] == 0) {
            GRID_MAP_VISITED[pathfind->posx][pathfind->posy - 1] = 1;
            PATHFIND[PATH_IND].posx = pathfind->posx;
            PATHFIND[PATH_IND].posy = pathfind->posy - 1;
            PATHFIND[PATH_IND].dir = DIR_E;
            PATHFIND[PATH_IND].prev = pathfind;
            PATHFIND[PATH_IND].todo = 1;
            PATHFIND[PATH_IND].count = pathfind->count + 1;
            PATH_IND++;
        }

        pathfind->todo = 0;
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
    		SHOOT_COUNT++;
    	} else if (s == "DOWN") {
    		commandStopped = commandStopped ? false : true;
    	} else if (s == "LEFT") {
    		direction = INT_LEFT;
    	} else if (s == "UP") {
    		direction = INT_STRAIGHT;
    	} else if (s == "RIGHT") {
    		direction = INT_RIGHT;
    	} else if (s == "A") {
    		DEST_X = DESTINATION_A_X;
    		DEST_Y = DESTINATION_A_Y;
    	} else if (s == "B") {
    		DEST_X = DESTINATION_B_X;
    		DEST_Y = DESTINATION_B_Y;
    	} else if (s == "C") {
    		DEST_X = DESTINATION_C_X;
    		DEST_Y = DESTINATION_C_Y;
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

			  PlayTone(    0,  202); wait1Msec(2243);  // Note(Rest, Duration(Whole))
  PlayTone(  391,   25); wait1Msec( 280);  // Note(G4, Duration(Eighth))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(  391,   25); wait1Msec( 280);  // Note(G4, Duration(Eighth))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(  391,   25); wait1Msec( 280);  // Note(G4, Duration(Eighth))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(  311,   25); wait1Msec( 280);  // Note(D#4, Duration(Eighth))
  PlayTone(    0,   13); wait1Msec( 140);  // Note(Rest, Duration(16th))
  PlayTone(  466,    6); wait1Msec(  70);  // Note(A#4, Duration(32th))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  391,   25); wait1Msec( 280);  // Note(G4, Duration(Eighth))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(  311,   25); wait1Msec( 280);  // Note(D#4, Duration(Eighth))
  PlayTone(    0,   13); wait1Msec( 140);  // Note(Rest, Duration(16th))
  PlayTone(  466,    6); wait1Msec(  70);  // Note(A#4, Duration(32th))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  391,   76); wait1Msec( 841);  // Note(G4, Duration(Quarter .))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(  587,   25); wait1Msec( 280);  // Note(D, Duration(Eighth))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(  587,   25); wait1Msec( 280);  // Note(D, Duration(Eighth))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(  587,   25); wait1Msec( 280);  // Note(D, Duration(Eighth))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(  622,   25); wait1Msec( 280);  // Note(D#, Duration(Eighth))
  PlayTone(    0,   13); wait1Msec( 140);  // Note(Rest, Duration(16th))
  PlayTone(  466,    6); wait1Msec(  70);  // Note(A#4, Duration(32th))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  369,   25); wait1Msec( 280);  // Note(F#4, Duration(Eighth))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(  311,   25); wait1Msec( 280);  // Note(D#4, Duration(Eighth))
  PlayTone(    0,   13); wait1Msec( 140);  // Note(Rest, Duration(16th))
  PlayTone(  466,    6); wait1Msec(  70);  // Note(A#4, Duration(32th))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  391,   76); wait1Msec( 841);  // Note(G4, Duration(Quarter .))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(  783,   25); wait1Msec( 280);  // Note(G, Duration(Eighth))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(  391,   25); wait1Msec( 280);  // Note(G4, Duration(Eighth))
  PlayTone(    0,   13); wait1Msec( 140);  // Note(Rest, Duration(16th))
  PlayTone(  391,    6); wait1Msec(  70);  // Note(G4, Duration(32th))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  783,   25); wait1Msec( 280);  // Note(G, Duration(Eighth))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(  739,   25); wait1Msec( 280);  // Note(F#, Duration(Eighth))
  PlayTone(    0,   13); wait1Msec( 140);  // Note(Rest, Duration(16th))
  PlayTone(  698,    6); wait1Msec(  70);  // Note(F, Duration(32th))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  659,    6); wait1Msec(  70);  // Note(E, Duration(32th))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  622,    6); wait1Msec(  70);  // Note(D#, Duration(32th))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  659,   19); wait1Msec( 210);  // Note(E, Duration(16th .))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  415,   19); wait1Msec( 210);  // Note(G#4, Duration(16th .))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  554,   25); wait1Msec( 280);  // Note(C#, Duration(Eighth))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(  523,   25); wait1Msec( 280);  // Note(C, Duration(Eighth))
  PlayTone(    0,   13); wait1Msec( 140);  // Note(Rest, Duration(16th))
  PlayTone(  493,    6); wait1Msec(  70);  // Note(B4, Duration(32th))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  466,    6); wait1Msec(  70);  // Note(A#4, Duration(32th))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  440,    6); wait1Msec(  70);  // Note(A4, Duration(32th))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  466,   19); wait1Msec( 210);  // Note(A#4, Duration(16th .))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  311,   19); wait1Msec( 210);  // Note(D#4, Duration(16th .))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  369,   25); wait1Msec( 280);  // Note(F#4, Duration(Eighth))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(  311,   25); wait1Msec( 280);  // Note(D#4, Duration(Eighth))
  PlayTone(    0,   13); wait1Msec( 140);  // Note(Rest, Duration(16th))
  PlayTone(  391,    6); wait1Msec(  70);  // Note(G4, Duration(32th))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  466,   25); wait1Msec( 280);  // Note(A#4, Duration(Eighth))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(  391,   25); wait1Msec( 280);  // Note(G4, Duration(Eighth))
  PlayTone(    0,   13); wait1Msec( 140);  // Note(Rest, Duration(16th))
  PlayTone(  466,    6); wait1Msec(  70);  // Note(A#4, Duration(32th))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  587,   76); wait1Msec( 841);  // Note(D, Duration(Quarter .))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(  783,   25); wait1Msec( 280);  // Note(G, Duration(Eighth))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(  391,   25); wait1Msec( 280);  // Note(G4, Duration(Eighth))
  PlayTone(    0,   13); wait1Msec( 140);  // Note(Rest, Duration(16th))
  PlayTone(  391,    6); wait1Msec(  70);  // Note(G4, Duration(32th))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  783,   25); wait1Msec( 280);  // Note(G, Duration(Eighth))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(  739,   25); wait1Msec( 280);  // Note(F#, Duration(Eighth))
  PlayTone(    0,   13); wait1Msec( 140);  // Note(Rest, Duration(16th))
  PlayTone(  698,    6); wait1Msec(  70);  // Note(F, Duration(32th))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  659,    6); wait1Msec(  70);  // Note(E, Duration(32th))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  622,    6); wait1Msec(  70);  // Note(D#, Duration(32th))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  659,   19); wait1Msec( 210);  // Note(E, Duration(16th .))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  415,   19); wait1Msec( 210);  // Note(G#4, Duration(16th .))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  554,   25); wait1Msec( 280);  // Note(C#, Duration(Eighth))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(  523,   25); wait1Msec( 280);  // Note(C, Duration(Eighth))
  PlayTone(    0,   13); wait1Msec( 140);  // Note(Rest, Duration(16th))
  PlayTone(  493,    6); wait1Msec(  70);  // Note(B4, Duration(32th))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  466,    6); wait1Msec(  70);  // Note(A#4, Duration(32th))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  440,    6); wait1Msec(  70);  // Note(A4, Duration(32th))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  466,   19); wait1Msec( 210);  // Note(A#4, Duration(16th .))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  311,   19); wait1Msec( 210);  // Note(D#4, Duration(16th .))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  369,   25); wait1Msec( 280);  // Note(F#4, Duration(Eighth))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(  311,   25); wait1Msec( 280);  // Note(D#4, Duration(Eighth))
  PlayTone(    0,   13); wait1Msec( 140);  // Note(Rest, Duration(16th))
  PlayTone(  466,    6); wait1Msec(  70);  // Note(A#4, Duration(32th))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  391,   25); wait1Msec( 280);  // Note(G4, Duration(Eighth))
  PlayTone(    0,   25); wait1Msec( 280);  // Note(Rest, Duration(Eighth))
  PlayTone(  311,   25); wait1Msec( 280);  // Note(D#4, Duration(Eighth))
  PlayTone(    0,   13); wait1Msec( 140);  // Note(Rest, Duration(16th))
  PlayTone(  466,    6); wait1Msec(  70);  // Note(A#4, Duration(32th))
  PlayTone(    0,    6); wait1Msec(  70);  // Note(Rest, Duration(32th))
  PlayTone(  391,   76); wait1Msec( 841);  // Note(G4, Duration(Quarter .))
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
	bool was_straight_intersection = false;

	while (1)
	{
		if (DEST_X >= 0 && DEST_Y >= 0) {
			create_path(DEST_X, DEST_Y);
			DEST_X = -1;
			DEST_Y = -1;

			goto_destination = true;

			commandStopped = false;
		}

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

		if (objectStopped && goto_destination) {

		motor[motor_left] = MOTOR_SPEED;
		motor[motor_right] = MOTOR_SPEED;
		wait1Msec(120);

			if (grid_direction == DIR_N) {
				if (GRID_MAP[posx][posy].north != NULL) {
					GRID_MAP[posx][posy].north->south = NULL;
				}
				GRID_MAP[posx][posy].north = NULL;
				grid_direction = DIR_S;
				posx += 1;
			} else if (grid_direction == DIR_S) {
				if (GRID_MAP[posx][posy].south != NULL) {
					GRID_MAP[posx][posy].south->north = NULL;
				}
				GRID_MAP[posx][posy].south = NULL;
				grid_direction = DIR_N;
				posx -= 1;
			} else if (grid_direction == DIR_W) {
				if (GRID_MAP[posx][posy].west != NULL) {
					GRID_MAP[posx][posy].west->east = NULL;
				}
				GRID_MAP[posx][posy].west = NULL;
				grid_direction = DIR_E;
				posy += 1;
			} else {
				if (GRID_MAP[posx][posy].east != NULL) {
					GRID_MAP[posx][posy].east->west = NULL;
				}
				GRID_MAP[posx][posy].east = NULL;
				grid_direction = DIR_W;
				posy -= 1;
			}

			create_path(TARGET_X, TARGET_Y);

			motor[motor_left] = -MOTOR_SPEED_TURN;
			motor[motor_right] = MOTOR_SPEED_TURN;
			wait1Msec(1800);
			int c = SensorValue[line];
			while (!onLine(c))
			{
				c = SensorValue[line];
			}

			objectStopped = false;
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
						goto_destination = false;
						commandStopped = true;
				}

				nxtDisplayTextLine(0, "dir: %i",direction);

				if (direction == INT_STRAIGHT) {
					was_straight_intersection = true;
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
				} else if (direction == INT_U_TURN) {
					motor[motor_left] = -MOTOR_SPEED_TURN;
					motor[motor_right] = MOTOR_SPEED_TURN;
					wait1Msec(1800);
					c = SensorValue[line];
					while (!onLine(c))
					{
						c = SensorValue[line];
					}
				} else if (direction == INT_RIGHT) {
					update_position(INT_RIGHT);
					nxtDisplayTextLine(0, "RIGHT");
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

				if (was_straight_intersection) {
					was_straight_intersection = false;
					update_position(INT_STRAIGHT);
				}

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
		if (distance < 17) {
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

task Shooting()
{
	while (true)
	{
		while (SHOOT_COUNT)
		{
			shoot();
			SHOOT_COUNT--;
		}
	}
}

task main()
{
	initialize_map();

	direction = INT_STRAIGHT;
	goto_destination = false;

	create_path(1, 0);

	stopped = true;
	objectStopped = false;
	commandStopped = true;

	startTask(commands);
	startTask(ObjectInWay);
	startTask(DrivingSound);
	startTask(Shooting);

	LineFolower();
}
