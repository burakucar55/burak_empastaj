#include "abov_config.h"
#include "abov_module_config.h"
#include <stdint.h>
#include "led.h"
#include "button.h"

// ZORUNLU GÖREV: Mod Kavramı (enum)
typedef enum {
    MODE_CHASE_SLOW,
    MODE_CHASE_FAST,
    MODE_PAUSE
} AppMode_t;

// Başlangıç durumu
static AppMode_t app_mode = MODE_CHASE_SLOW;

// Non-blocking milisaniye üreten sistem saati (Tick)
static uint32_t Get_System_Time_ms(void) {
    static uint32_t dummy_tick = 0;
    for (volatile int i = 0; i < 15000; i++) {} // ~1ms gecikme (kartınıza göre ayarlayın)
    return dummy_tick++;
}

/**********************************************************************
 * @brief       User Code Here
 **********************************************************************/
void PRV_USER_Code(void)
{
    // 1. İnit (Donanım Başlatma)
    Led_Init();
    Button_Init();

    // 2. Demo Provası 1. Adım: Reset -> Self-Test
    Led_SelfTest();

    uint32_t last_chase_time = 0;
    uint32_t chase_delay_ms = 500;

    // 3. Ana Döngü
    while (1)
    {
        uint32_t current_time = Get_System_Time_ms();

        // Buton Olaylarını (Events) Oku
        ButtonEvent_t ev = Button_Process(current_time);

        // ZORUNLU GÖREV: Buton olayları modu değiştirir
        if (ev == BTN_SHORT) {
            // Kısa Basış: Slow <-> Fast arası geçiş
            if (app_mode == MODE_CHASE_SLOW) {
                app_mode = MODE_CHASE_FAST;
            } else if (app_mode == MODE_CHASE_FAST) {
                app_mode = MODE_CHASE_SLOW;
            } else if (app_mode == MODE_PAUSE) {
                app_mode = MODE_CHASE_SLOW; // Pause'dan çıkar, Slow'a dön
            }
        }
        else if (ev == BTN_LONG) {
            // Uzun Basış: Pause moduna gir / çık
            if (app_mode != MODE_PAUSE) {
                app_mode = MODE_PAUSE;
            } else {
                app_mode = MODE_CHASE_SLOW;
            }
        }

        // ZORUNLU GÖREV: while(1) içinde switch(app_mode)
        switch (app_mode) {
            case MODE_CHASE_SLOW:
                chase_delay_ms = 500;
                if (current_time - last_chase_time >= chase_delay_ms) {
                    last_chase_time = current_time;
                    Led_ChaseStep();
                }
                break;

            case MODE_CHASE_FAST:
                chase_delay_ms = 100;
                if (current_time - last_chase_time >= chase_delay_ms) {
                    last_chase_time = current_time;
                    Led_ChaseStep();
                }
                break;

            case MODE_PAUSE:
                Led_AllOff(); // LED'leri durdur/kapat
                break;

            default:
                break;
        }
    }
}
