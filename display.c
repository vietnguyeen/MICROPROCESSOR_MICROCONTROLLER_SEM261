#include "display.h"
#include "main.h"

int led_buffer[4] = {0, 3, 0, 5};
int index_pair = 0;

static const uint8_t seg7_table[10] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10};

void display7SEG_Tens(int num) {
    if (num < 0 || num > 9) num = 0;
    uint8_t pattern = seg7_table[num];
    HAL_GPIO_WritePin(GPIOB, SEG_0_Pin, (pattern & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, SEG_1_Pin, (pattern & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, SEG_2_Pin, (pattern & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, SEG_3_Pin, (pattern & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, SEG_4_Pin, (pattern & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, SEG_5_Pin, (pattern & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, SEG_6_Pin, (pattern & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void display7SEG_Units(int num) {
    if (num < 0 || num > 9) num = 0;
    uint8_t pattern = seg7_table[num];
    HAL_GPIO_WritePin(GPIOB, SEG_7_Pin,  (pattern & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, SEG_8_Pin,  (pattern & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, SEG_9_Pin,  (pattern & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, SEG_10_Pin, (pattern & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, SEG_11_Pin, (pattern & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, SEG_12_Pin, (pattern & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, SEG_13_Pin, (pattern & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void update7SEGPair(int pair_index) {
    HAL_GPIO_WritePin(GPIOA, EN0_Pin | EN1_Pin | EN2_Pin | EN3_Pin, GPIO_PIN_SET);
    if (pair_index == 0) {
        display7SEG_Tens(led_buffer[0]);
        display7SEG_Units(led_buffer[1]);
        HAL_GPIO_WritePin(GPIOA, EN0_Pin | EN1_Pin, GPIO_PIN_RESET);
    } else {
        display7SEG_Tens(led_buffer[2]);
        display7SEG_Units(led_buffer[3]);
        HAL_GPIO_WritePin(GPIOA, EN2_Pin | EN3_Pin, GPIO_PIN_RESET);
    }
}

void updateClockBuffer(int val1, int val2) {
    led_buffer[0] = val1 / 10; led_buffer[1] = val1 % 10;
    led_buffer[2] = val2 / 10; led_buffer[3] = val2 % 10;
}

void clearAllTrafficLeds(void) {
    HAL_GPIO_WritePin(GPIOA, LED_RED_EW_Pin | LED_AMBER_EW_Pin | LED_GREEN_EW_Pin |
                             LED_RED_NS_Pin | LED_AMBER_NS_Pin | LED_GREEN_NS_Pin, GPIO_PIN_RESET);
}

void setTrafficLeds(int r_ew, int a_ew, int g_ew, int r_ns, int a_ns, int g_ns) {
    HAL_GPIO_WritePin(LED_RED_EW_GPIO_Port,   LED_RED_EW_Pin,   r_ew ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_AMBER_EW_GPIO_Port, LED_AMBER_EW_Pin, a_ew ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_GREEN_EW_GPIO_Port, LED_GREEN_EW_Pin, g_ew ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_RED_NS_GPIO_Port,   LED_RED_NS_Pin,   r_ns ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_AMBER_NS_GPIO_Port, LED_AMBER_NS_Pin, a_ns ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_GREEN_NS_GPIO_Port, LED_GREEN_NS_Pin, g_ns ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
