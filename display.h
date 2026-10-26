#ifndef DISPLAY_H
#define DISPLAY_H

extern int led_buffer[4];
extern int index_pair;

void updateClockBuffer(int val1, int val2);
void update7SEGPair(int pair_index);
void clearAllTrafficLeds(void);
void setTrafficLeds(int r_ew, int a_ew, int g_ew, int r_ns, int a_ns, int g_ns);

#endif
