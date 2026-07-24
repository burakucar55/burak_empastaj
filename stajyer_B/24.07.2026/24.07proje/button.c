#include "button.h"
#include "hal_pcu.h"
#include <stdbool.h>

// ZORUNLU GÖREV: const eşik sabitleri (Debounce ve Uzun basış için)
static const uint32_t DEBOUNCE_MS = 50;
static const uint32_t LONG_PRESS_MS = 800; // 800ms basılı tutulursa LONG sayılır

void Button_Init(void) {
    HAL_PCU_SetInOutMode(PCU_ID_C, PCU_PIN_ID_9, PCU_INOUT_INPUT);
    HAL_PCU_SetPullUpDown(PCU_ID_C, PCU_PIN_ID_9, PCU_PUPD_DISABLED);
}

// ZORUNLU GÖREV: Debounce (Sistemi kilitlemeyen State Machine)
ButtonEvent_t Button_Process(uint32_t current_time_ms) {
    static uint32_t press_start_time = 0;
    static bool is_pressed = false;
    static bool long_event_fired = false;

    PCU_PORT_e pin_val;
    HAL_PCU_GetInputValue(PCU_ID_C, PCU_PIN_ID_9, &pin_val);
    bool is_physically_pressed = (pin_val == PCU_PORT_LOW);

    if (is_physically_pressed && !is_pressed) {
        // Butona yeni basıldı
        is_pressed = true;
        press_start_time = current_time_ms;
        long_event_fired = false;
    }
    else if (is_physically_pressed && is_pressed) {
        // Butona basılmaya devam ediliyor
        if (!long_event_fired && (current_time_ms - press_start_time >= LONG_PRESS_MS)) {
            long_event_fired = true; // Uzun basış tetiklendi
            return BTN_LONG;
        }
    }
    else if (!is_physically_pressed && is_pressed) {
        // Buton bırakıldı
        is_pressed = false;
        // Eğer uzun basış fırlatılmadıysa ve debounce süresi geçtiyse kısa basıştır
        if (!long_event_fired && (current_time_ms - press_start_time >= DEBOUNCE_MS)) {
            return BTN_SHORT;
        }
    }

    return BTN_NONE;
}
