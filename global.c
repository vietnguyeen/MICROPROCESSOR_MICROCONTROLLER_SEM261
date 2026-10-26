#include "global.h"

int mode = 1;
int traffic_status = INIT;

int time_red   = 5;
int time_amber = 2;
int time_green = 3;

int count_ew = 0;
int count_ns = 0;
int temp_value = 1;

int saved_status = INIT;
int saved_count_ew = 0;
int saved_count_ns = 0;
int old_time_red = 5;
int old_time_amber = 2;
int old_time_green = 3;
