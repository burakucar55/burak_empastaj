/**
 *******************************************************************************
 * @file        prv_user_code.c
 * @author      ABOV R&D Division
 * @brief       Dummy User Application Main
 *
 * Copyright 2024 ABOV Semiconductor Co.,Ltd. All rights reserved.
 *
 * This file is licensed under terms that are found in the LICENSE file
 * located at Document directory.
 * If this file is delivered or shared without applicable license terms,
 * the terms of the BSD-3-Clause license shall be applied.
 * Reference: https://opensource.org/licenses/BSD-3-Clause
 ******************************************************************************/

#include "abov_config.h"
#include "abov_module_config.h"
#include "hal_pcu.h"
#if (CONFIG_DEBUG == 1)
#include "debug_log.h"
#include "debug.h"
#endif

/* Include HAL header files for your target modules */


/**********************************************************************
 * @brief       User Code Here
 * @param[in]   None
 * @return      None
 **********************************************************************/
const uint32_t LONG_PRESS_THRESHOLD_MS = 1000; // 1 Saniye (1000 ms) eşiği
const uint32_t DEBOUNCE_TIME_MS = 30;          // 30 ms sıçrama önleme süresi

// Arka plandaki Timer kesmesinde her 1ms'de artan global süre sayacımız
volatile uint32_t g_Tick_ms;

void PRV_USER_Code(void)
{
    PCU_PORT_e current_state;
    PCU_PORT_e last_state = PCU_PORT_HIGH;

    // RAM'de tutulan süre sayaçları
    uint32_t press_start_time = 0;
    uint32_t press_duration = 0;

    // Kısa basış için toggle (aç-kapa) durumunu tutan değişken
    uint8_t led_toggle_status = 0;

    // Başlangıçta tüm LED'leri söndür (Active-Low oldukları için HIGH gönder)
    HAL_PCU_SetOutputValue(PCU_ID_B, PCU_PIN_ID_4,  PCU_PORT_HIGH); // D1
    HAL_PCU_SetOutputValue(PCU_ID_B, PCU_PIN_ID_10, PCU_PORT_HIGH); // D5
    HAL_PCU_SetOutputValue(PCU_ID_B, PCU_PIN_ID_9,  PCU_PORT_HIGH); // D2

    // İşlemci hızınıza göre her 1 milisaniyede bir kesme (interrupt) üretir
    SysTick_Config(SystemCoreClock / 1000);

    while (1)
    {
        // 1. Butonun anlık durumunu oku
        HAL_PCU_GetInputValue(PCU_ID_C, PCU_PIN_ID_9, &current_state);

        // 2. Düşen Kenar (Butona basıldığı ilk anı yakala)
        if (current_state == PCU_PORT_LOW && last_state == PCU_PORT_HIGH)
        {
            // --- TICK TABANLI DEBOUNCE ---
            uint32_t debounce_start = g_Tick_ms;
            // 30 milisaniye geçene kadar bekle
            while((g_Tick_ms - debounce_start) < DEBOUNCE_TIME_MS) {
                // Donanım gürültüsünün geçmesini bekle
            }

            // Süre doldu, butonu tekrar oku ve onayla
            PCU_PORT_e confirm_state;
            HAL_PCU_GetInputValue(PCU_ID_C, PCU_PIN_ID_9, &confirm_state);

            if (confirm_state == PCU_PORT_LOW) // Gerçekten basılmış
            {
                // Butona basılma anındaki sistem saatini (tick) RAM'e kaydet
                press_start_time = g_Tick_ms;

                // Buton BIRAKILANA KADAR bu döngüde bekle
                while (confirm_state == PCU_PORT_LOW)
                {
                    HAL_PCU_GetInputValue(PCU_ID_C, PCU_PIN_ID_9, &confirm_state);
                }

                // --- BUTON BIRAKILDI, SÜREYİ HESAPLA ---
                press_duration = g_Tick_ms - press_start_time;

                // 3. AKSİYON SEÇİMİ
                if (press_duration >= LONG_PRESS_THRESHOLD_MS)
                {
                    /* =========================================
                       UZUN BASIŞ: TÜM LEDLERİ 3 KEZ FLAŞ YAP
                       ========================================= */
                    for(int f = 0; f < 3; f++)
                    {
                        // Hepsini Yak
                        HAL_PCU_SetOutputValue(PCU_ID_B, PCU_PIN_ID_4,  PCU_PORT_LOW);
                        HAL_PCU_SetOutputValue(PCU_ID_B, PCU_PIN_ID_10, PCU_PORT_LOW);
                        HAL_PCU_SetOutputValue(PCU_ID_B, PCU_PIN_ID_9,  PCU_PORT_LOW);

                        uint32_t flash_timer = g_Tick_ms;
                        while((g_Tick_ms - flash_timer) < 150) { } // 150 ms bekle

                        // Hepsini Söndür
                        HAL_PCU_SetOutputValue(PCU_ID_B, PCU_PIN_ID_4,  PCU_PORT_HIGH);
                        HAL_PCU_SetOutputValue(PCU_ID_B, PCU_PIN_ID_10, PCU_PORT_HIGH);
                        HAL_PCU_SetOutputValue(PCU_ID_B, PCU_PIN_ID_9,  PCU_PORT_HIGH);

                        flash_timer = g_Tick_ms;
                        while((g_Tick_ms - flash_timer) < 150) { } // 150 ms bekle
                    }
                }
                else
                {
                    /* =========================================
                       KISA BASIŞ: D1 LED'İNİ TOGGLE YAP
                       ========================================= */
                    led_toggle_status = !led_toggle_status; // Durumu tersine çevir

                    if (led_toggle_status == 1)
                    {
                        HAL_PCU_SetOutputValue(PCU_ID_B, PCU_PIN_ID_4, PCU_PORT_LOW); // Yak
                    }
                    else
                    {
                        HAL_PCU_SetOutputValue(PCU_ID_B, PCU_PIN_ID_4, PCU_PORT_HIGH); // Söndür
                    }
                }
            }
        }

        // 4. Sonraki kontrol için durumu hafızaya al
        last_state = current_state;
    }
}
// SysTick zamanlayıcısı her 1 ms'de bir otomatik olarak bu fonksiyonu çalıştırır
void SysTick_Handler(void)
{
    extern volatile uint32_t g_Tick_ms;
    g_Tick_ms++;
}
