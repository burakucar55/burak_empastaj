#include "button.h"

static ButtonState_t currentState = STATE_IDLE;
static uint32_t stateTimer = 0;

void Button_Init(void) {
    currentState = STATE_IDLE;
}

ButtonEvent_t Button_Process(uint32_t current_time_ms, bool is_pressed_hardware) {
    ButtonEvent_t event = EVENT_NONE;

    switch (currentState) {
        case STATE_IDLE:
            if (is_pressed_hardware) {
                currentState = STATE_DEBOUNCE_PRESS;
                stateTimer = current_time_ms;
            }
            break;

        case STATE_DEBOUNCE_PRESS:
            if ((current_time_ms - stateTimer) >= 20) { // 20ms debounce
                if (is_pressed_hardware) {
                    currentState = STATE_WAIT_RELEASE;
                    event = EVENT_CLICK; // Net tıklama eventi üretildi
                } else {
                    currentState = STATE_IDLE; // Gürültüymüş
                }
            }
            break;

        case STATE_WAIT_RELEASE:
            if (!is_pressed_hardware) {
                currentState = STATE_DEBOUNCE_RELEASE;
                stateTimer = current_time_ms;
            }
            break;

        case STATE_DEBOUNCE_RELEASE:
            if ((current_time_ms - stateTimer) >= 20) {
                if (!is_pressed_hardware) {
                    currentState = STATE_IDLE;
                } else {
                    currentState = STATE_WAIT_RELEASE;
                }
            }
            break;
    }

    return event;
}
