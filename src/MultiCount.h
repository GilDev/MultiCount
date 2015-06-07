#ifndef MULTI_COUNTER_H
#define MULTI_COUNTER_H

#include <pebble.h>

#define APP_VERSION "1.0"
#define STORAGE_VERSION 1
#define STORAGE_VERSION_KEY 0
#define COUNTER_NUMBER_KEY 1
#define FIRST_COUNTER_KEY 2


#define COUNTER_NUMBER 20
#define MAX_NAME_SIZE 10
#define MAX_COUNTER_VALUE 65535
#define MAX_COUNTER_DIGITS 5

static uint8_t counterNumber;
static struct Counter *counters[COUNTER_NUMBER];

void createCounter(const char *name, uint16_t value);

struct Counter {
	char name[MAX_NAME_SIZE + 1];
	uint16_t value;
};

#endif