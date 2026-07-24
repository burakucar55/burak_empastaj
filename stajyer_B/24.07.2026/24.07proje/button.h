#ifndef BUTTON_H
#define BUTTON_H
#include <stdint.h>

// Resimdeki Buton Eventleri
typedef enum {
    BTN_NONE,
    BTN_SHORT, // Kısa basış (Hız değiştir)
    BTN_LONG   // Uzun basış (Pause/Devam)
} ButtonEvent_t;

void Button_Init(void);
ButtonEvent_t Button_Process(uint32_t current_time_ms);

#endif /* BUTTON_H */
