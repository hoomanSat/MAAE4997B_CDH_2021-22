/*
 * TaskSequenceTimeTest.c
 *
 *  	Created: 2022-03-18
 *      Author: Dante Corsi
 */

#include <Tests/TaskSequenceTimeTest.h> // all includes are enclosed in the header file

// Initialization
#define DEFAULT_PRIORITY	2					// default task priority level
#define I2C_ADDRESS			0x41				// default slave i2c address
#define COMMAND_LENGTH		14					// default number of characters in a command
#define TESTMODE			1					// (1: run the taskCommandSequencer with I2C input, 0: run the taskCommandSequencer using raw manual inputs
#define TESTCOMMAND1		"001-1111111111"	// test command 1 for use with raw input testmode
#define TESTCOMMAND2		"002-1111111113"	// test command 2 for use with raw input testmode

// Task Sequencer using command id and unixtime timecode

void taskCommandSequencerRAW()
{
	xTaskHandle sequencedTaskHandle;
	int incomingCommandCount = 2; // specify manually the number of commands you wish to test
	char *TEST_COMMAND_ARRAY[2];
	TEST_COMMAND_ARRAY[0]=TESTCOMMAND1;
	TEST_COMMAND_ARRAY[1]=TESTCOMMAND2;

	TRACE_DEBUG_WP("\n\r Task: interpretUplinkedCommands: Starting. \n\r");

	{
		TRACE_WARNING("\n\r incomingCommandCount: %d \n\r", incomingCommandCount);
	}

	char commandArray[incomingCommandCount]; // create an array to store the incoming commands as text strings, and for eventual parsing

	for (int i = 0; i < incomingCommandCount; i++) // read commands in order they arrive, store in array
	{
		commandArray[i] = *TEST_COMMAND_ARRAY[i];
		printf("\n\r commandArray: %d \n\r", commandArray[i]); // uplinked command format should be "001-1647666294" where the first three characters are the command code, and the second set of letters are the timetag in epoch
	}

	for (int i = 0; i < incomingCommandCount; i++) // create a generic task for each entry in the command array, pass the time tag as input parameter
	{
		// get first three chars from command array entry, these are the command code or id which identifies the corresponding task, then concat at the end of "task..." and pass to xTaskGenericCreate
		char commandCode[3];
		char timeCode[10];
		char taskName[7] = "task";
		int k = 0;

		strncpy(commandCode,commandArray,3);
		commandCode[2] = 0;
		printf("\n\r commandCode: %s \n\r",commandCode); // prints the command code for debug and validation

		strcat(taskName, commandCode); // concatenate "task" with the 3 digit command code
		pdTASK_CODE taskCode = (pdTASK_CODE) taskName;
		for (int j = 3; j < 14; j++) // loop through the last 10 characters of the command array entry (these characters should represent the time tag) and store in timeCode for later use
		{
			while (k < 10)
			{
				timeCode[k] = commandArray[j];
				k++;
			}
			timeCode[9] = 0;
		}
		printf("\n\r timeCode: %s \n\r",timeCode); // prints the time code for debug and validation
		xTaskGenericCreate(taskCode, (const signed char*) "sequencedTask", 1024, (void *) timeCode, DEFAULT_PRIORITY, &sequencedTaskHandle, NULL, NULL); // create tasks based on the task sequence obtained from i2c
	}
}

void taskCommandSequencerI2C()
{
	xTaskHandle sequencedTaskHandle;
	int incomingCommandCount = 0;
	unsigned char readCommandCount[64] = {0};
	unsigned char readCommands[64] = {0};

	TRACE_DEBUG_WP("\n\r Task: interpretUplinkedCommands: Starting. \n\r");

	incomingCommandCount = I2C_read(I2C_ADDRESS, readCommandCount, 2); // read the command count, later used to set command array size
	if(incomingCommandCount != 0)
	{
		TRACE_WARNING("\n\r incomingCommandCount: %d \n\r", incomingCommandCount);
	}

	//unsigned int totalIncomingData = 0;
	//totalIncomingData = incomingCommandCount * 14;

	char commandArray[incomingCommandCount]; // create an array to store the incoming commands as text strings, and for eventual parsing

	for (int i = 0; i < incomingCommandCount; i++) // read commands in order they arrive, store in array
	{
		commandArray[i] = I2C_read(I2C_ADDRESS, readCommands, COMMAND_LENGTH);
		printf("\n\r commandArray: %d \n\r", commandArray[i]); // uplinked command format should be "001-1647666294" where the first three characters are the command code, and the second set of letters are the timetag in epoch
	}

	for (int i = 0; i < incomingCommandCount; i++) // create a generic task for each entry in the command array, pass the time tag as input parameter
	{
		// get first three chars from command array entry, these are the command code or id which identifies the corresponding task, then concat at the end of "task..." and pass to xTaskGenericCreate
		char commandCode[3];
		char timeCode[10];
		char taskName[7] = "task";
		int k = 0;

		strncpy(commandCode,commandArray,3);
		commandCode[2] = 0;
		printf("\n\r commandCode: %s \n\r",commandCode); // prints the command code for debug and validation

		strcat(taskName, commandCode); // concatenate "task" with the 3 digit command code
		pdTASK_CODE taskCode = (pdTASK_CODE) taskName;
		for (int j = 3; j < 14; j++) // loop through the last 10 characters of the command array entry (these characters should represent the time tag) and store in timeCode for later use
		{
			while (k < 10)
			{
				timeCode[k] = commandArray[j];
				k++;
			}
			timeCode[9] = 0;
		}
		printf("\n\r timeCode: %s \n\r",timeCode); // prints the time code for debug and validation
		xTaskGenericCreate(taskCode, (const signed char*) "sequencedTask", 1024, (void *) timeCode, DEFAULT_PRIORITY, &sequencedTaskHandle, NULL, NULL); // create tasks based on the task sequence obtained from i2c
	}
}

// define the placeholder tasks used for this test: each task should simply print to the console, then self-delete
void task001(void* inputParameter)
{
	// task timing -> get current epoch from realtime clock (RTC), subtract from taskRunEpoch defined by command uplink, this yields the number of seconds until the task should run, now convert to ms and pass to vTaskDelay
	char * taskRunEpoch = (char *) inputParameter;
	unsigned long currentEpochTime = 0;
	currentEpochTime = Time_getUnixEpoch(&currentEpochTime);
	long taskEpochTime = (long) taskRunEpoch;

	long taskTimeDelay = (taskEpochTime - currentEpochTime)*1000;
	vTaskDelay(taskTimeDelay/portTICK_RATE_MS); // sets the delay between the current time and the desired task run time

	for (;;)
	{
		printf("\n\r Executing Task 001");
		vTaskDelete(NULL); // tasks typically run in an infinite for loop, but by including the vTaskDelete function, we allow the task to execute once after the desired delay, then delete itself from the scheduler
	}
}

void task002(void* inputParameter)
{
	char * taskRunEpoch = (char *) inputParameter;
	unsigned long currentEpochTime = 0;
	currentEpochTime = Time_getUnixEpoch(&currentEpochTime);
	long taskEpochTime = (long) taskRunEpoch;

	long taskTimeDelay = (taskEpochTime - currentEpochTime)*1000;
	vTaskDelay(taskTimeDelay/portTICK_RATE_MS);

	for(;;)
	{
		printf("\n\r Executing Task 002");
		vTaskDelete(NULL);
	}
}

// unused initEpochTest
/*static void initEpochTest(void) {
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
}*/


// unused taskEpochTest
/*void taskEpochTest() {
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

}*/

Boolean TaskSequenceTimeTest()
{
	if (TESTMODE == 0)
	{
		taskCommandSequencerRAW();
	}

	if (TESTMODE == 1)
	{
		taskCommandSequencerI2C();
	}

	return FALSE;
}
