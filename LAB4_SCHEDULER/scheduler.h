#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#include <stdint.h>
#include "main.h"

/* ---------------- Cau hinh ---------------- */
#define SCH_MAX_TASKS           20      /* So task toi da trong pool */
#define SCH_TICK_MS             10      /* 1 tick = 10 ms (TIM2) */
#define SCH_READY_QUEUE_SIZE    32      /* Hang doi task da den han */
#define SCH_REPORT_ERRORS               /* Bo comment nay neu khong can bao loi */
#define SCH_ERROR_DISPLAY_TICKS 6000u   /* 6000 tick x 10 ms = 60 s */

#define NO_TASK_ID              0xFFFFFFFFu
#define RETURN_NORMAL           0
#define RETURN_ERROR            1

/* Ma loi (hien thi tren PB0-PB7) */
#define ERROR_SCH_TOO_MANY_TASKS        1
#define ERROR_SCH_CANNOT_DELETE_TASK    2
#define ERROR_SCH_READY_QUEUE_FULL      3

/* Task: pool tinh + danh sach lien ket sap xep theo thoi gian.
 * Delay la do chenh lech (delta) so voi task dung truoc trong danh sach. */
typedef struct sTask {
    void (*pTask)(void);
    uint32_t Delay;         /* delta tick so voi task truoc do */
    uint32_t Period;        /* 0 = one-shot */
    uint8_t  RunMe;         /* so lan can chay (do SCH_Update tang) */
    uint32_t TaskID;        /* = chi so trong pool */
    struct sTask *pNext;    /* task ke tiep trong danh sach cho */
} sTask;

extern sTask SCH_tasks_G[SCH_MAX_TASKS];
extern unsigned char Error_code_G;

void     SCH_Init(void);
void     SCH_Update(void);
void     SCH_Dispatch_Tasks(void);
uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD);
uint8_t  SCH_Delete_Task(uint32_t taskID);
void     SCH_Report_Status(void);
void     SCH_Set_Error(unsigned char err_code);
uint32_t SCH_Get_Time(void);    /* thoi gian (ms), do chinh xac 1 tick = 10 ms */

#endif /* INC_SCHEDULER_H_ */
