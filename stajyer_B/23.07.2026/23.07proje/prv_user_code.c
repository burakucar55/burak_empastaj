#include <stdint.h>
#include <stdbool.h>
#include "hal_pcu.h"

// 1. ADIM: Pin Listesi ve Ayarlar
#define LED_COUNT 3

// ABOV için Port ve Pin bilgilerini bir arada tutacak yapı
typedef struct {
    PCU_ID_e port;
    PCU_PIN_ID_e pin;
} LedPin_t;

// Kullanılacak LED'lerin dizisi (Senin projendeki 3 LED)
static const LedPin_t led_pins[LED_COUNT] = {
    {PCU_ID_B, PCU_PIN_ID_4}, // 0. İndeks (LED1)
    {PCU_ID_B, PCU_PIN_ID_9}, // 1. İndeks (LED2)
    {PCU_ID_F, PCU_PIN_ID_7}  // 2. İndeks (LED3)
};

// 2. ADIM: State Değişkenleri
uint8_t active_index = 0;
bool reverse = false;

// --- DONANIM OKUMA VE ZAMAN FONKSİYONLARI ---
bool HW_ReadButton(void) {
    PCU_PORT_e pin_val;
    HAL_PCU_GetInputValue(PCU_ID_C, PCU_PIN_ID_9, &pin_val);
    return (pin_val == 0); // Active-Low
}

uint32_t Get_System_Time_ms(void) {
    static uint32_t dummy_tick = 0;
    for (volatile int i = 0; i < 15000; i++) {} // ~1ms gecikme
    return dummy_tick++;
}

// 3. ADIM: Bir Chase (Kayan Işık) Adımı Fonksiyonu
void Chase_Step(void) {
    // for döngüsü ile tüm LED'leri tara
    for (int i = 0; i < LED_COUNT; i++) {
        if (i == active_index) {
            // Sadece aktif olan indeksi YAK (Active-Low: 0)
            HAL_PCU_SetOutputValue(led_pins[i].port, led_pins[i].pin, 0);
        } else {
            // Geri kalanları SÖNDÜR (Active-Low: 1)
            HAL_PCU_SetOutputValue(led_pins[i].port, led_pins[i].pin, 1);
        }
    }

    // Yön ve Sınır Kontrolü (if ile sınır veya yön var)
    if (reverse) {
        if (active_index == 0) {
            reverse = false; // Başa çarptı, ileri dön
            active_index++;  // 0'da iki tur beklememesi için hemen 1'e atlatıyoruz
        } else {
            active_index--;
        }
    } else {
        if (active_index >= LED_COUNT - 1) {
            reverse = true;  // Sona çarptı, geri dön
            active_index--;  // Sonda iki tur beklememesi için bir geri alıyoruz
        } else {
            active_index++;
        }
    }
}

// 4. ADIM: while(1) İçinde Akış
void PRV_USER_Code(void) {

    // LED pinlerini tek tek yazmak yerine for döngüsü ile Init yapıyoruz
    for (int i = 0; i < LED_COUNT; i++) {
        HAL_PCU_SetInOutMode(led_pins[i].port, led_pins[i].pin, PCU_INOUT_OUTPUT_PUSH_PULL);
        // Başlangıçta hepsini söndür (Active-Low: 1)
        HAL_PCU_SetOutputValue(led_pins[i].port, led_pins[i].pin, 1);
    }

    // Buton Init
    HAL_PCU_SetInOutMode(PCU_ID_C, PCU_PIN_ID_9, PCU_INOUT_INPUT);

    bool last_button_state = false;
    uint32_t last_chase_time = 0;
    const uint32_t CHASE_SPEED_MS = 150; // Işıkların kayma hızı

    while (1) {
        uint32_t current_time = Get_System_Time_ms();

        // Buton okuma (Polling)
        bool current_button_state = HW_ReadButton();

        // Butona basıldığında yönü tersine çevir (Edge detection)
        if (current_button_state && !last_button_state) {
            reverse = !reverse;
        }
        last_button_state = current_button_state;

        // Non-blocking (Sistemi kilitlemeyen) gecikme ile chase adımı
        if ((current_time - last_chase_time) >= CHASE_SPEED_MS) {
            last_chase_time = current_time;
            Chase_Step();
        }
    }
}
