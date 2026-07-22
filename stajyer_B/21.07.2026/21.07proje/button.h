#ifndef BUTTON_H
#define BUTTON_H

// Butonun üreteceği Event'ler (Olaylar)
typedef enum {
    BTN_EVENT_NONE,    // İşlem yok
    BTN_EVENT_PRESS,   // Kısa basış
    BTN_EVENT_LONG,    // Uzun basış
    BTN_EVENT_RELEASE  // Buton bırakıldı
} ButtonEvent_e;

// Dışarıya açılan fonksiyonlar
void Button_Init(void);
ButtonEvent_e Button_Process(void);

#endif /* BUTTON_H */
