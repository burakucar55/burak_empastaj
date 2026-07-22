#include <stdint.h>
#include <stdbool.h>
#include "button.h"
#include "hal_pcu.h"

// Kesmeden gelecek ham bayrak
volatile bool interrupt_flag = false;

bool HW_ReadButton(void) {
    PCU_PORT_e pin_val;
    HAL_PCU_GetInputValue(PCU_ID_C, PCU_PIN_ID_9, &pin_val);
    return (pin_val == 0); // Active-Low
}

// En az 3 LED kontrolü (Örnek olarak LED1, LED2, LED3 kullanıyoruz)
void HW_SetLedDirect(uint8_t led_index, bool state) {
    PCU_PORT_e val = state ? (PCU_PORT_e)0 : (PCU_PORT_e)1;
    switch(led_index) {
        case 0: HAL_PCU_SetOutputValue(PCU_ID_B, PCU_PIN_ID_4, val); break; // LED1
        case 1: HAL_PCU_SetOutputValue(PCU_ID_B, PCU_PIN_ID_9, val); break; // LED2
        case 2: HAL_PCU_SetOutputValue(PCU_ID_F, PCU_PIN_ID_7, val); break; // LED3
        default: break;
    }
}

uint32_t Get_System_Time_ms(void) {
    static uint32_t dummy_tick = 0;
    for (volatile int i = 0; i < 15000; i++) {} // ~1ms
    return dummy_tick++;
}

// ISR sadece sinyal veriyor
void Button_ISR_Handler(uint32_t status, void *context) {
    interrupt_flag = true;
}

void PRV_USER_Code(void) {
    // Pin Konfigürasyonları
    HAL_PCU_SetInOutMode(PCU_ID_B, PCU_PIN_ID_4, PCU_INOUT_OUTPUT_PUSH_PULL);
    HAL_PCU_SetInOutMode(PCU_ID_B, PCU_PIN_ID_9, PCU_INOUT_OUTPUT_PUSH_PULL);
    HAL_PCU_SetInOutMode(PCU_ID_F, PCU_PIN_ID_7, PCU_INOUT_OUTPUT_PUSH_PULL);
    HAL_PCU_SetInOutMode(PCU_ID_C, PCU_PIN_ID_9, PCU_INOUT_INPUT);

    // Kesme Ayarları (Her iki kenar - basıldı ve bırakıldı)
    HAL_PCU_SetIntrPort(PCU_ID_C, PCU_PIN_ID_9, PCU_INTR_MODE_EDGE, PCU_INTR_TRG_BOTH_LEVEL_EDGE, 0);
    PCU_IRQ_CFG_t irq_cfg = {0};
    irq_cfg.eId = PCU_ID_C;
    irq_cfg.eOps = PCU_OPS_INTR;
    irq_cfg.pfnHandler = Button_ISR_Handler;
    irq_cfg.un32IRQPrio = 3;
    HAL_PCU_SetIRQ(&irq_cfg);

    Button_Init();

    // Chase (Kayan Işık) Değişkenleri
    int8_t current_led = 0;
    int8_t direction = 1; // 1: İleri, -1: Geri
    uint32_t last_chase_time = 0;
    const uint32_t chase_interval = 300; // Kayma hızı (ms)

    while (1) {
        uint32_t current_time = Get_System_Time_ms();

        // 1. Kesme geldiyse butona aktar
        bool hw_state = false;
        if (interrupt_flag) {
            interrupt_flag = false;
            hw_state = HW_ReadButton();
        } else {
            hw_state = HW_ReadButton(); // Anlık durumu koru
        }

        // 2. Button State Machine işleniyor (Debounce + Event)
        ButtonEvent_t evt = Button_Process(current_time, hw_state);
        if (evt == EVENT_CLICK) {
            // Butona her basıldığında Chase YÖNÜ değişir (İleri <-> Geri)
            direction = -direction;
        }

        // 3. Ana Döngüde Kayan Işık (Chase) Sürekli Çalışıyor
        if ((current_time - last_chase_time) >= chase_interval) {
            last_chase_time = current_time;

            // Tüm LED'leri söndür
            for(int i=0; i<3; i++) {
                HW_SetLedDirect(i, false);
            }

            // Sıradaki LED'i yak
            HW_SetLedDirect(current_led, true);

            // Yöne göre sonraki LED'e geç
            current_led += direction;
            if (current_led > 2) {
                current_led = 0;
            } else if (current_led < 0) {
                current_led = 2;
            }
        }
    }
}
