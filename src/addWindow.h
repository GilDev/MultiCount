#ifndef ADD_WINDOW_H
#define ADD_WINDOW_H

#define CLICKS_INTERVAL_NAME 100
#define CLICKS_INTERVAL_VALUE 50
#define CHAR_WIDTH 12

enum {ERROR_TOO_MUCH_COUNTERS, ERROR_NO_NAME};

void pushAddWindow(void);
void initAddWindow(void);
void deinitAddWindow(void);

#endif