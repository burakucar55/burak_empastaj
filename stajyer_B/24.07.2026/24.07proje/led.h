#ifndef LED_H
#define LED_H

void Led_Init(void);
void Led_SelfTest(void); // Açılışta for döngüsü ile test
void Led_ChaseStep(void); // Bir sonraki LED'e geçiş
void Led_AllOff(void); // Tüm LED'leri kapat (Pause modu için)

#endif /* LED_H */
