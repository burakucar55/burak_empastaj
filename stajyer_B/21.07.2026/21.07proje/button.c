#include "button.h"
#include "hal_pcu.h"

// --- CONST EŞİK DEĞERLERİ (ROM) ---
const unsigned long DEBOUNCE_THRESHOLD = 50;
const unsigned long LONG_PRESS_THRESHOLD = 150000;

// --- RAM'DE TUTULAN DEĞİŞKENLER ---
static unsigned long button_ticks = 0;
static unsigned char btn_state = 0;

void Button_Init(void)
{
    // PC9 (USER) pini Input yapılıyor ve donanımsal pull-up olduğu için dahili olan kapatılıyor
    HAL_PCU_SetInOutMode(PCU_ID_C, PCU_PIN_ID_9, PCU_INOUT_INPUT);
    HAL_PCU_SetPullUpDown(PCU_ID_C, PCU_PIN_ID_9, PCU_PUPD_DISABLED);
}

ButtonEvent_e Button_Process(void)
{
    PCU_PORT_e current_btn;
    HAL_PCU_GetInputValue(PCU_ID_C, PCU_PIN_ID_9, &current_btn);

    if (current_btn == PCU_PORT_LOW)
    {
        button_ticks++;

        // Kısa Basış (Press) Olayı
        if (button_ticks > DEBOUNCE_THRESHOLD && btn_state == 0)
        {
            btn_state = 1;
            return BTN_EVENT_PRESS;
        }

        // Uzun Basış (Long) Olayı
        if (button_ticks >= LONG_PRESS_THRESHOLD && btn_state == 1)
        {
            btn_state = 2;
            return BTN_EVENT_LONG;
        }
    }
    else
    {
        // Buton Bırakılma (Release) Olayı
        if (btn_state > 0)
        {
            btn_state = 0;
            button_ticks = 0;
            return BTN_EVENT_RELEASE;
        }

        button_ticks = 0;
    }

    return BTN_EVENT_NONE;
}
