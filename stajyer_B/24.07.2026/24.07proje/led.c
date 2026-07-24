#include "led.h"
#include "hal_pcu.h"

// Kullanacağımız LED pinleri (PB4, PB9, PF7)
static const PCU_ID_e led_ports[3] = {PCU_ID_B, PCU_ID_B, PCU_ID_F};
static const PCU_PIN_ID_e led_pins[3] = {PCU_PIN_ID_4, PCU_PIN_ID_9, PCU_PIN_ID_7};

static int current_led = 0;

// Bloklayıcı (blocking) basit bekleme - Sadece Self-Test'te kullanılır
static void Delay_Dummy(volatile uint32_t count) {
    while(count--) { __asm("nop"); }
}

void Led_Init(void) {
    for (int i = 0; i < 3; i++) {
        HAL_PCU_SetInOutMode(led_ports[i], led_pins[i], PCU_INOUT_OUTPUT_PUSH_PULL);
        HAL_PCU_SetOutputValue(led_ports[i], led_pins[i], PCU_PORT_HIGH); // Söndür
    }
}

// ZORUNLU GÖREV: for döngüsü ile self-test (Reset sonrası çalışır)
void Led_SelfTest(void) {
    for (int i = 0; i < 3; i++) {
        // Yak, bekle, söndür
        HAL_PCU_SetOutputValue(led_ports[i], led_pins[i], PCU_PORT_LOW);
        Delay_Dummy(500000);
        HAL_PCU_SetOutputValue(led_ports[i], led_pins[i], PCU_PORT_HIGH);
    }
}

void Led_ChaseStep(void) {
    // Öncekini söndür
    HAL_PCU_SetOutputValue(led_ports[current_led], led_pins[current_led], PCU_PORT_HIGH);

    // Sıradakine geç (Başa sar)
    current_led++;
    if (current_led > 2) current_led = 0;

    // Yenisini yak
    HAL_PCU_SetOutputValue(led_ports[current_led], led_pins[current_led], PCU_PORT_LOW);
}

void Led_AllOff(void) {
    for (int i = 0; i < 3; i++) {
        HAL_PCU_SetOutputValue(led_ports[i], led_pins[i], PCU_PORT_HIGH);
    }
}
