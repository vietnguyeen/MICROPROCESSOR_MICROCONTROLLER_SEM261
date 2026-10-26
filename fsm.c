#include "fsm.h"
#include "global.h"
#include "display.h"
#include "button.h"
#include "software_timer.h"
#include "main.h"

void fsm_traffic_normal(void) {
    switch (traffic_status) {
        case INIT:
            count_ew = time_green;
            count_ns = time_red;
            setTrafficLeds(0, 0, 1, 1, 0, 0);
            updateClockBuffer(count_ew, count_ns);
            setTimer0(1000);
            traffic_status = EW_GREEN_NS_RED;
            break;
        case EW_GREEN_NS_RED:
            if (timer0_flag == 1) {
                setTimer0(1000);
                count_ew--; count_ns--;
                if (count_ew <= 0) {
                    count_ew = time_amber;
                    setTrafficLeds(0, 1, 0, 1, 0, 0);
                    traffic_status = EW_AMBER_NS_RED;
                }
                updateClockBuffer(count_ew, count_ns);
            }
            break;
        case EW_AMBER_NS_RED:
            if (timer0_flag == 1) {
                setTimer0(1000);
                count_ew--; count_ns--;
                if (count_ew <= 0) {
                    count_ew = time_red; count_ns = time_green;
                    setTrafficLeds(1, 0, 0, 0, 0, 1);
                    traffic_status = EW_RED_NS_GREEN;
                }
                updateClockBuffer(count_ew, count_ns);
            }
            break;
        case EW_RED_NS_GREEN:
            if (timer0_flag == 1) {
                setTimer0(1000);
                count_ew--; count_ns--;
                if (count_ns <= 0) {
                    count_ns = time_amber;
                    setTrafficLeds(1, 0, 0, 0, 1, 0);
                    traffic_status = EW_RED_NS_AMBER;
                }
                updateClockBuffer(count_ew, count_ns);
            }
            break;
        case EW_RED_NS_AMBER:
            if (timer0_flag == 1) {
                setTimer0(1000);
                count_ew--; count_ns--;
                if (count_ns <= 0) {
                    count_ew = time_green; count_ns = time_red;
                    setTrafficLeds(0, 0, 1, 1, 0, 0);
                    traffic_status = EW_GREEN_NS_RED;
                }
                updateClockBuffer(count_ew, count_ns);
            }
            break;
    }
}

void fsm_for_input_processing(void) {
    /* Xử lý BUTTON_0: Chuyển Mode */
    if (is_button_pressed(0)) {
        if (mode == 1) {
            old_time_red = time_red;
            old_time_amber = time_amber;
            old_time_green = time_green;
        }

        mode++;
        if (mode > 4) mode = 1;
        clearAllTrafficLeds();

        if (mode == 1) {
            /* Kiểm tra tính hợp lệ của thời gian (Red = Green + Amber) */
            if (time_red != (time_green + time_amber)) {
                time_red = old_time_red;
                time_amber = old_time_amber;
                time_green = old_time_green;
            }

            /* Luôn bắt đầu lại từ đầu khi thoát cài đặt */
            traffic_status = INIT;
        }
        else if (mode == 2) { temp_value = time_red; updateClockBuffer(2, temp_value); setTimer1(250); }
        else if (mode == 3) { temp_value = time_amber; updateClockBuffer(3, temp_value); setTimer1(250); }
        else if (mode == 4) { temp_value = time_green; updateClockBuffer(4, temp_value); setTimer1(250); }
    }

    /* Vận hành Mode hiện tại */
    if (mode == 1) {
        fsm_traffic_normal();
    } else {
        /* Chớp tắt LED 2Hz */
        if (timer1_flag == 1) {
            setTimer1(250);
            if (mode == 2) { HAL_GPIO_TogglePin(LED_RED_EW_GPIO_Port, LED_RED_EW_Pin); HAL_GPIO_TogglePin(LED_RED_NS_GPIO_Port, LED_RED_NS_Pin); }
            else if (mode == 3) { HAL_GPIO_TogglePin(LED_AMBER_EW_GPIO_Port, LED_AMBER_EW_Pin); HAL_GPIO_TogglePin(LED_AMBER_NS_GPIO_Port, LED_AMBER_NS_Pin); }
            else if (mode == 4) { HAL_GPIO_TogglePin(LED_GREEN_EW_GPIO_Port, LED_GREEN_EW_Pin); HAL_GPIO_TogglePin(LED_GREEN_NS_GPIO_Port, LED_GREEN_NS_Pin); }
        }

        /* Xử lý BUTTON_1: Tăng giá trị */
        if (is_button_pressed(1)) {
            temp_value++;
            if (temp_value > 99) temp_value = 1;
            updateClockBuffer(mode, temp_value);
        }

        /* Xử lý BUTTON_2: Lưu thông số */
        if (is_button_pressed(2)) {
            if (mode == 2) time_red = temp_value;
            else if (mode == 3) time_amber = temp_value;
            else if (mode == 4) time_green = temp_value;
        }
    }

    /* Quét LED 7 đoạn */
    if (timer2_flag == 1) {
        setTimer2(500);
        update7SEGPair(index_pair);
        index_pair = 1 - index_pair;
    }
}
