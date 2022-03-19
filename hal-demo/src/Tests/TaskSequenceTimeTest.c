/*
 * TaskSequenceTimeTest.c
 *
 *  	Created: 2022-03-18
 *      Author: Dante Corsi
 */

#include <Tests/TaskSequenceTimeTest.h>

// Initialization

#define I2CBUS_SPEED		400000		// I2C bus speed in Hz
#define I2CBUS_TIMEOUT		20			// Timeout per byte for I2C in .100 milliseconds
#define TIME_SYNCINTERVAL	60			// RTT and RTC sync interval in seconds
#define DEFAULT_PRIORITY	2			// Default task priority level

// Command and Timetag Interpreter

void taskCommandSequencer(void)
{
	xTaskHandle sequencedTaskHandle;
	unsigned int incomingCommandCount = 0;
	unsigned char readCommandCount[64] = {0};
	unsigned int i = 0;
	I2Ctransfer i2cRx;

	TRACE_DEBUG_WP("\n\r Task: interpretUplinkedCommands: Starting. \n\r");

	i2cRx.readData = readCommandCount;
	i2cRx.readSize = 2;
	i2cRx.slaveAddress = 0x41;

	incomingCommandCount = I2C_read(&i2cRx); // read the command count, later used to set command array size
	if(incomingCommandCount != 0)
	{
		TRACE_WARNING("\n\r interpretUplinkedCommands: Returned uplink command count of: %d \n\r", incomingCommandCount);
	}

	vTaskDelay(5); // optional delay

	char commandArray[incomingCommandCount][25+1]; // create an array to store the incoming commands as text strings, and for eventual parsing

	for (int i; i < incomingCommandCount; i++) // read commands in order, store in array, sort array based on timetag value
	{
		strcpy(commandArray[i], I2C_read(&i2cRx));
		printf("\n\r commandArray: Entry #%d is %d \n\r", i, commandArray[i]); // uplinked command format should be "001-1647666294" where the first three characters are the command code, and the second set of letters are the timetag in epoch
	}

	for (int i; i < incomingCommandCount; i++) // create a generic task for each entry in the command array, pass the time tag as input parameter
	{
		// get first three chars from command array entry, these are the command code or id which identifies the corresponding task, then concat at the end of "task..." and pass to xTaskGenericCreate
		char commandCode[3];
		char *timeCode[10];
		char taskName[7];
		int k = 0;

		strncpy(commandCode,commandArray,3);
		commandCode[3] = 0;
		taskName = strcat("task", commandCode);

		for (int j = 3; j < 14; j++) // loop through the last 10 characters of the command array entry (these characters should represent the time tag) and store in timeCode for later use
		{
			while (k < 10)
			{
				timeCode[k] = commandArray[j];
				k++;
			}
			timeCode[10] = 0;
		}
		xTaskGenericCreate(taskName, (const signed char*) "sequencedTask", 1024, (void *) timeCode, DEFAULT_PRIORITY, &sequencedTaskHandle, NULL, NULL);
	}
}


// define the placeholder tasks used for this test
void task001(void* inputParameter)
{
	char taskRunEpoch = (char *) inputParameter;
	unsigned long getEpochTime = 0;
	getEpochTime = Time_getUnixEpoch(&getEpochTime);



	printf("\n\r Executing Task 001");
}

void task002(void* inputParameter)
{
	char taskRunEpoch = (char *) inputParameter;
	printf("\n\r Executing Task 002");
}

void task003(void* inputParameter)
{
	char taskRunEpoch = (char *) inputParameter;
	printf("\n\r Executing Task 003");
}


// unused initEpochTest, ignore
static void initEpochTest(void) {
	int retVal;
	Time fallbackTime = {.seconds = 0, .minutes = 0, .hours = 0, .day = 1, .date = 1, .month = 1, .year = 0xAA};
	Time tempTime = {0};

	retVal = I2C_start(I2CBUS_SPEED, I2CBUS_TIMEOUT);
	if(retVal != 0)
	{
		TRACE_WARNING("Initializing I2C failed with error %d! \n\r", retVal);
	}

	// Initialize time without a set-time value. The Time module will try to restore time from RTC.
	retVal = Time_start(NULL, TIME_SYNCINTERVAL);
	if(retVal != 0)
	{
		// Non-zero return value from Time does not necessarily mean we can't keep a valid time.
		TRACE_WARNING("Initializing Time failed with error %d! \n\r", retVal);
	}

	retVal = RTC_getTime(&tempTime);
	if(retVal==0 || tempTime.year==0) // RTC power-cycles to Y2K (stored as 0)
	{
		// Time is initialized with a dummy time-value.
		// If time is never set on the satellite, will start with 00:00:00, Sunday, January 1st, 2170 (0xAA = 170).
		// If the ground control ever retrieves time or a time-value close to this is found in any logs, a satellite-reset becomes clear.

		Time_set(&fallbackTime);
		TRACE_DEBUG(" Time restored to fallback value. \n\r");
	}

	retVal = Supervisor_start(NULL, 0);
	if(retVal != E_NO_SS_ERR)
	{
		TRACE_WARNING("Initializing Supervisor failed with error %d! \n\r", retVal);
	}
}


// unused taskEpochTest
void taskEpochTest() {
	//unsigned int testIteration;
	unsigned long set_epochtime = 946684800;
	unsigned long get_epochtime = 0;
	unsigned long tickcount1 = 0, tickcount2 = 0, tickresult = 0;
	int retVal;

	printf("\n\r taskEpochTest: Initializing. \n\r\n\r");

	initEpochTest();

	printf("\n\r taskEpochTest: Done initializing. \n\r");

	tickcount1 = xTaskGetTickCount();

	Time_setUnixEpoch(951782400);

	//for(testIteration=0; testIteration<1440; testIteration++)
	while(get_epochtime < 1136073600)//1893456001)
	{
		retVal = Time_setUnixEpoch(set_epochtime);
		if(retVal != 0) {
			TRACE_WARNING(" taskThermalTest: Error during Time_setUnixEpoch: %d \n\r", retVal);
		}
		else
		{
			//vTaskDelay(1);

			retVal = Time_getUnixEpoch(&get_epochtime);
			if(retVal != 0) {
				TRACE_WARNING(" taskThermalTest: Error during Time_getUnixEpoch: %d \n\r", retVal);
			}
			else
			{
				if(set_epochtime == get_epochtime)
				{
					if(get_epochtime == 978307200)
					{
						TRACE_DEBUG("\n\r get_epochtime 2001: %ld \n\r", get_epochtime);
					}
					if(get_epochtime == 1009843200)
					{
						TRACE_DEBUG("\n\r get_epochtime 2002: %ld \n\r", get_epochtime);
					}
					if(get_epochtime == 1041379200)
					{
						TRACE_DEBUG("\n\r get_epochtime 2003: %ld \n\r", get_epochtime);
					}
					if(get_epochtime == 1072915200)
					{
						TRACE_DEBUG("\n\r get_epochtime 2004: %ld \n\r", get_epochtime);
					}
					if(get_epochtime == 1104537600)
					{
						TRACE_DEBUG("\n\r get_epochtime 2005: %ld \n\r", get_epochtime);
					}
					if(get_epochtime == 1136073600)
					{
						TRACE_DEBUG("\n\r get_epochtime 2006: %ld \n\r", get_epochtime);
					}
					set_epochtime+=60;
				}
				else
				{
					TRACE_WARNING(" taskThermalTest: Error during comparison between set and get \n\r");
					TRACE_DEBUG("\n\r set_epochtime: %ld \n\r", set_epochtime);
					TRACE_DEBUG("\n\r get_epochtime: %ld \n\r", get_epochtime);
					break;
				}
				//TRACE_DEBUG("\n\r get_epochtime value: %ld \n\r", get_epochtime);
			}
		}
	}

	tickcount2 = xTaskGetTickCount();

	if(tickcount1 < tickcount2)
	{
		tickresult = tickcount2 - tickcount1;
		TRACE_DEBUG("\n\r tick time: %ld \n\r", tickresult);
	}
	else
	{
		TRACE_DEBUG("\n\r tick time error \n\r");
	}

	// This would cause the watchdog to reset the ARM, this is done on purpose to test the watchdog.
	while(1);

}

Boolean TaskSequenceTimeTest()
{
	xTaskHandle taskCommandSequencer;

	xTaskGenericCreate(taskCommandSequencer, (const signed char*)"taskCommandSequencer", 1024, NULL, 2, &taskCommandSequencer, NULL, NULL);

	return FALSE;
}
