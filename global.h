#ifndef GLOBAL_H
#define GLOBAL_H

/* Traffic Light States in Mode 1 */
#define INIT             0
#define EW_GREEN_NS_RED  1
#define EW_AMBER_NS_RED  2
#define EW_RED_NS_GREEN  3
#define EW_RED_NS_AMBER  4

extern int mode;
extern int traffic_status;

extern int time_red;
extern int time_amber;
extern int time_green;

extern int count_ew;
extern int count_ns;
extern int temp_value;

/* Variables for Revert & Resume */
extern int saved_status;
extern int saved_count_ew;
extern int saved_count_ns;
extern int old_time_red;
extern int old_time_amber;
extern int old_time_green;

#endif
