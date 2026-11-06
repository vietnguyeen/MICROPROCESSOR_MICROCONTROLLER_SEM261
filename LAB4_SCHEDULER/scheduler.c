#include "scheduler.h"

/* ------------------------------------------------------------------ */
/*  Bien toan cuc                                                      */
/* ------------------------------------------------------------------ */
unsigned char Error_code_G      = 0;
unsigned char Last_error_code_G = 0;
unsigned int  Error_tick_count_G = 0;

sTask SCH_tasks_G[SCH_MAX_TASKS];

static sTask *SCH_head = 0;                     /* dau danh sach delta */
static volatile uint32_t SCH_Tick_G = 0;        /* dem tick he thong   */

/* Hang doi cac task da den han (ISR ghi, main doc) */
static uint8_t          SCH_ready[SCH_READY_QUEUE_SIZE];
static volatile uint8_t SCH_ready_in  = 0;      /* chi ISR ghi  */
static volatile uint8_t SCH_ready_out = 0;      /* chi main ghi */

/* ------------------------------------------------------------------ */
/*  Vung gang (bao ve bang cach luu/khoi phuc PRIMASK)                 */
/* ------------------------------------------------------------------ */
static inline uint32_t SCH_Enter_Critical(void) {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

static inline void SCH_Exit_Critical(uint32_t primask) {
    if (primask == 0) {
        __enable_irq();
    }
}

/* ------------------------------------------------------------------ */
/*  Thao tac tren danh sach delta (goi trong vung gang hoac trong ISR)*/
/* ------------------------------------------------------------------ */

/* Chen task vao danh sach, 'delay' tinh bang tick tu thoi diem hien tai.
 * Task bang gio duoc chen SAU task cung gio (giu thu tu FIFO). */
static void SCH_List_Insert(sTask *t, uint32_t delay) {
    sTask *prev = 0;
    sTask *cur  = SCH_head;

    while (cur != 0 && delay >= cur->Delay) {
        delay -= cur->Delay;
        prev = cur;
        cur  = cur->pNext;
    }

    t->Delay = delay;
    t->pNext = cur;
    if (cur != 0) {
        cur->Delay -= delay;    /* an toan: delay < cur->Delay */
    }
    if (prev != 0) {
        prev->pNext = t;
    } else {
        SCH_head = t;
    }
}

/* Go task khoi danh sach (neu co), cong don delta cho task ke tiep */
static void SCH_List_Remove(sTask *t) {
    sTask *prev = 0;
    sTask *cur  = SCH_head;

    while (cur != 0 && cur != t) {
        prev = cur;
        cur  = cur->pNext;
    }
    if (cur == 0) {
        return;                 /* khong nam trong danh sach (vd: one-shot da den han) */
    }
    if (cur->pNext != 0) {
        cur->pNext->Delay += cur->Delay;
    }
    if (prev != 0) {
        prev->pNext = cur->pNext;
    } else {
        SCH_head = cur->pNext;
    }
    cur->pNext = 0;
}

static void SCH_Free_Slot(uint32_t idx) {
    SCH_tasks_G[idx].pTask  = 0;
    SCH_tasks_G[idx].Delay  = 0;
    SCH_tasks_G[idx].Period = 0;
    SCH_tasks_G[idx].RunMe  = 0;
    SCH_tasks_G[idx].TaskID = NO_TASK_ID;
    SCH_tasks_G[idx].pNext  = 0;
}

/* ------------------------------------------------------------------ */
/*  Hang doi ready                                                     */
/* ------------------------------------------------------------------ */
static int SCH_Ready_Push(uint8_t idx) {       /* chi goi tu ISR */
    uint8_t next = (uint8_t)((SCH_ready_in + 1) % SCH_READY_QUEUE_SIZE);
    if (next == SCH_ready_out) {
        Error_code_G = ERROR_SCH_READY_QUEUE_FULL;
        return 0;
    }
    SCH_ready[SCH_ready_in] = idx;
    SCH_ready_in = next;
    return 1;
}

/* Lay het cac task co delta == 0 ra khoi dau danh sach */
static void SCH_Pop_Due(void) {
    while (SCH_head != 0 && SCH_head->Delay == 0) {
        sTask *t = SCH_head;
        SCH_head = t->pNext;
        t->pNext = 0;

        if (SCH_Ready_Push((uint8_t)t->TaskID)) {
            t->RunMe += 1;
        } else if (t->Period == 0) {
            SCH_Free_Slot(t->TaskID);   /* one-shot bi mat: giai phong slot */
            continue;
        }

        if (t->Period != 0) {
            SCH_List_Insert(t, t->Period);  /* Period >= 1 nen khong lap vo han */
        }
    }
}

/* ------------------------------------------------------------------ */
/*  API                                                                */
/* ------------------------------------------------------------------ */
void SCH_Init(void) {
    uint32_t i;
    for (i = 0; i < SCH_MAX_TASKS; i++) {
        SCH_Free_Slot(i);
    }
    SCH_head       = 0;
    SCH_ready_in   = 0;
    SCH_ready_out  = 0;
    SCH_Tick_G     = 0;
    Error_code_G   = 0;
    Last_error_code_G = 0;
    Error_tick_count_G = 0;

#ifdef SCH_REPORT_ERRORS
    /* Logic am: ghi 0xFF = tat het LED ngay tu dau */
    GPIOB->ODR = (GPIOB->ODR & 0xFF00) | 0x00FF;
#endif
}

/* Goi trong ngat TIM2 moi 10 ms. Moi tick chi tru delta cua task dau danh sach: O(1).
 * Chi khi co task den han moi chen lai task chu ky (toi da SCH_MAX_TASKS buoc). */
void SCH_Update(void) {
    SCH_Tick_G++;

    if (SCH_head != 0) {
        SCH_head->Delay -= 1;   /* bat bien: Delay cua dau danh sach luon >= 1 */
    }
    SCH_Pop_Due();

    SCH_Report_Status();        /* goi moi tick, dung nhu thiet ke 60000 tick */
}

/* DELAY = 0 -> chay o tick ke tiep; DELAY = d -> chay sau d+1 tick.
 * PERIOD = 0 -> one-shot. Tra ve SCH_MAX_TASKS neu loi. */
uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD) {
    uint32_t primask;
    uint32_t Index = 0;

    if (pFunction == 0) {
        return SCH_MAX_TASKS;
    }

    primask = SCH_Enter_Critical();

    while (Index < SCH_MAX_TASKS && SCH_tasks_G[Index].pTask != 0) {
        Index++;
    }

    if (Index == SCH_MAX_TASKS) {
        Error_code_G = ERROR_SCH_TOO_MANY_TASKS;
        SCH_Exit_Critical(primask);
        return SCH_MAX_TASKS;
    }

    SCH_tasks_G[Index].pTask  = pFunction;
    SCH_tasks_G[Index].Period = PERIOD;
    SCH_tasks_G[Index].RunMe  = 0;
    SCH_tasks_G[Index].TaskID = Index;
    SCH_tasks_G[Index].pNext  = 0;
    SCH_List_Insert(&SCH_tasks_G[Index], DELAY + 1);

    SCH_Exit_Critical(primask);
    return Index;
}

/* Chay cac task da den han theo dung thu tu den han */
void SCH_Dispatch_Tasks(void) {
    while (SCH_ready_out != SCH_ready_in) {
        uint8_t idx = SCH_ready[SCH_ready_out];
        SCH_ready_out = (uint8_t)((SCH_ready_out + 1) % SCH_READY_QUEUE_SIZE);

        void (*fn)(void) = 0;

        uint32_t primask = SCH_Enter_Critical();
        if (SCH_tasks_G[idx].pTask != 0 && SCH_tasks_G[idx].RunMe > 0) {
            SCH_tasks_G[idx].RunMe -= 1;
            fn = SCH_tasks_G[idx].pTask;
            if (SCH_tasks_G[idx].Period == 0) {
                SCH_Free_Slot(idx);     /* one-shot: giai phong truoc khi chay */
            }
        }
        SCH_Exit_Critical(primask);

        if (fn != 0) {
            fn();
        }
    }
}

/* Tra ve RETURN_NORMAL (0) neu xoa duoc, RETURN_ERROR (1) neu khong */
uint8_t SCH_Delete_Task(uint32_t taskID) {
    uint8_t Return_code;
    uint32_t primask = SCH_Enter_Critical();

    if (taskID >= SCH_MAX_TASKS || SCH_tasks_G[taskID].pTask == 0) {
        Error_code_G = ERROR_SCH_CANNOT_DELETE_TASK;
        Return_code = RETURN_ERROR;
    } else {
        SCH_List_Remove(&SCH_tasks_G[taskID]);
        SCH_Free_Slot(taskID);      /* RunMe = 0 nen muc con sot trong ready queue se bi bo qua */
        Return_code = RETURN_NORMAL;
    }

    SCH_Exit_Critical(primask);
    return Return_code;
}

uint32_t SCH_Get_Time(void) {
    return SCH_Tick_G * SCH_TICK_MS;
}

void SCH_Report_Status(void) {
#ifdef SCH_REPORT_ERRORS
    if (Error_code_G != Last_error_code_G) {
        /* Logic am tren 8 bit thap cua Port B */
        GPIOB->ODR = (GPIOB->ODR & 0xFF00) | (uint16_t)(255 - Error_code_G);
        Last_error_code_G = Error_code_G;

        if (Error_code_G != 0) {
            Error_tick_count_G = SCH_ERROR_DISPLAY_TICKS;
        } else {
            Error_tick_count_G = 0;
        }
    } else {
        if (Error_tick_count_G != 0) {
            if (--Error_tick_count_G == 0) {
                Error_code_G = 0;   /* het thoi gian hien thi: xoa loi */
            }
        }
    }
#endif
}

void SCH_Set_Error(unsigned char err_code) {
    Error_code_G = err_code;
}
