/**
 * @file main.c
 * @brief Exmaple application to demonstrate timer API
 * @version 0.1
 * @date 2022-07-05
 * 
 * @copyright Copyright (c) 2022 Waybyte Solutions
 * 
 */

#include <stdio.h>
#include <unistd.h>

#include <lib.h>
#include <ril.h>
#include <os_api.h>
#include <timer.h>

#ifdef SOC_RDA8910
#define STDIO_PORT "/dev/ttyUSB0"
#else
#define STDIO_PORT "/dev/ttyS0"
#endif

/**
 * URC Handler
 * @param param1	URC Code
 * @param param2	URC Parameter
 */
static void urc_callback(unsigned int param1, unsigned int param2)
{
	switch (param1) {
	case URC_SYS_INIT_STATE_IND:
		if (param2 == SYS_STATE_SMSOK) {
			/* Ready for SMS */
		}
		break;
	case URC_SIM_CARD_STATE_IND:
		switch (param2) {
		case SIM_STAT_NOT_INSERTED:
			debug(DBG_OFF, "SYSTEM: SIM card not inserted!\n");
			break;
		case SIM_STAT_READY:
			debug(DBG_INFO, "SYSTEM: SIM card Ready!\n");
			break;
		case SIM_STAT_PIN_REQ:
			debug(DBG_OFF, "SYSTEM: SIM PIN required!\n");
			break;
		case SIM_STAT_PUK_REQ:
			debug(DBG_OFF, "SYSTEM: SIM PUK required!\n");
			break;
		case SIM_STAT_NOT_READY:
			debug(DBG_OFF, "SYSTEM: SIM card not recognized!\n");
			break;
		default:
			debug(DBG_OFF, "SYSTEM: SIM ERROR: %d\n", param2);
		}
		break;
	case URC_GSM_NW_STATE_IND:
		debug(DBG_OFF, "SYSTEM: GSM NW State: %d\n", param2);
		break;
	case URC_GPRS_NW_STATE_IND:
		break;
	case URC_CFUN_STATE_IND:
		break;
	case URC_COMING_CALL_IND:
		debug(DBG_OFF, "Incoming voice call from: %s\n", ((struct ril_callinfo_t *)param2)->number);
		/* Take action here, Answer/Hang-up */
		break;
	case URC_CALL_STATE_IND:
		switch (param2) {
		case CALL_STATE_BUSY:
			debug(DBG_OFF, "The number you dialed is busy now\n");
			break;
		case CALL_STATE_NO_ANSWER:
			debug(DBG_OFF, "The number you dialed has no answer\n");
			break;
		case CALL_STATE_NO_CARRIER:
			debug(DBG_OFF, "The number you dialed cannot reach\n");
			break;
		case CALL_STATE_NO_DIALTONE:
			debug(DBG_OFF, "No Dial tone\n");
			break;
		default:
			break;
		}
		break;
	case URC_NEW_SMS_IND:
		debug(DBG_OFF, "SMS: New SMS (%d)\n", param2);
		/* Handle New SMS */
		break;
	case URC_MODULE_VOLTAGE_IND:
		debug(DBG_INFO, "VBatt Voltage: %d\n", param2);
		break;
	case URC_ALARM_RING_IND:
		break;
	case URC_FILE_DOWNLOAD_STATUS:
		break;
	case URC_FOTA_STARTED:
		break;
	case URC_FOTA_FINISHED:
		break;
	case URC_FOTA_FAILED:
		break;
	case URC_STKPCI_RSP_IND:
		break;
	default:
		break;
	}
}

/**
 * @brief Callback function for task independent timer
 * 
 * @param id Timer ID
 * @param arg 
 */
static void timer1_callback(int id, void *arg)
{
	printf("Timer %d handler\n", id);
}

static void task_timer_callback(int id, void *arg)
{
	printf("Task Timer %d handler\n", id);
}

/**
 * Sample Task
 * @param arg	Task Argument
 */
static void sample_task(void *arg)
{
	struct osmsg_t msg;
	int timer;

	printf("Creating a task timer\n");

	timer = task_timer_register(task_timer_callback, NULL);
	printf("Timer timer created: %d\n", timer);
	task_timer_start(timer, 2000, true);

	while (1) {
		/* for task timer to run, parent task must block on message */
		os_message_get(&msg);
	}
}

/**
 * Application main entry point
 */
int main(int argc, char *argv[])
{
	int timer_id;
	/*
	 * Initialize library and Setup STDIO
	 */
	logicrom_init(STDIO_PORT, urc_callback);

	printf("System Ready\n");

	/* Test timer */
	printf("Creating task independent timer\n");
	timer_id = timer_register(timer1_callback, NULL);
	if (timer_id < 0) {
		printf("Error creating timer %d\n", timer_id);
	} else {
		printf("Timer created: %d\n", timer_id);
		timer_config(timer_id, 1000, true);
		timer_start(timer_id);
	}

	/* Create Application tasks */
	os_task_create(sample_task, NULL, FALSE);

	printf("System Initialization finished\n");

	while (1) {
		/* Main task */
		sleep(1);
	}
}
