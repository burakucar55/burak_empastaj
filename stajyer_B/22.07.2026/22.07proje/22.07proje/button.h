#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include <stdbool.h>

// Buton Olayları
typedef enum {
    EVENT_NONE,
    EVENT_CLICK // Tek ve net bir tıklama eventi
} ButtonEvent_t;

// State Machine Durumları
typedef enum {
    STATE_IDLE,
    STATE_DEBOUNCE_PRESS,
    STATE_WAIT_RELEASE,
    STATE_DEBOUNCE_RELEASE
} ButtonState_t;

void Button_Init(void);
ButtonEvent_t Button_Process(uint32_t current_time_ms, bool is_pressed_hardware);

#endif /* BUTTON_H */
