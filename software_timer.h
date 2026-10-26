#ifndef SOFTWARE_TIMER_H
#define SOFTWARE_TIMER_H

extern volatile int timer0_flag;
extern volatile int timer1_flag;
extern volatile int timer2_flag;

void setTimer0(int duration);
void setTimer1(int duration);
void setTimer2(int duration);
void timer_run(void);

#endif
