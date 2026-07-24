# Gün 05 — Görevler

Hafta 1 mini proje (uygulama günü)

Cuma **entegrasyon günü**: yeni uzun teori yok. Pazartesi’den Perşembe’ye kadar öğrendiklerini **tek çalışan demo** haline getiriyorsun. Kod yazma süresi günün büyük kısmı.

Kısa özet not: [`01_Anlatim.md`](01_Anlatim.md)  
Sıra: **Kolay → Orta → Zor**.

---

## Hafta 1’de elinde ne olmalı?

| Gün | Kart / kavram | Cuma’da projede |
|-----|---------------|-----------------|
| Pazartesi | GPIO, LED, buton | Pin init, yak/söndür |
| Salı | MCU, RAM/Flash, debounce | `const`, `uint32_t`, state |
| Çarşamba | Kesme, NVIC, flag | `volatile`, `if (flag)` |
| Perşembe | `for`, `while`, `if`, dosya | Chase, self-test, modüler kod |

Eksik bir gün varsa önce Perşembe kolay/orta görevlerini tamamlayıp Cuma’ya geç.

---

## Ortak hazırlık

- [ ] Kart ve IDE hazır
- [ ] En az **3 LED** ve **1 buton** pin’i belli
- [ ] Perşembe chase veya blink kodun var (kopyalayıp geliştirebilirsin)
- [ ] GitHub staj repon açık (Cuma zor için)

---

## Kolay

### Amaç

Haftanın parçalarını **tek `main.c` içinde** üst üste koymak. Amaç “güzel mimari” değil; **hepsinin bir arada çalıştığını** görmek.

| | |
|---|---|
| **Görev** | Self-test + ana mod + buton tepkisi |
| **Yapıldı** | ☐ |

### Program akışı (kartta göreceğin sıra)

```
Güç / reset
    ↓
for → LED’ler sırayla bir kez yan-sön  (self-test)
    ↓
while(1) sonsuz döngü
    ├── buton oku (poll veya flag)
    ├── if → bir şey değiştir (toggle / mod)
    └── chase veya blink (for veya if ile LED)
```

### Adım adım

**1. Self-test (`for`)**

- `LED_COUNT` kadar LED.  
- Döngüde her birini 100–200 ms yak, söndür.  
- Bittiğinde normal moda geç.

**2. Ana mod (`while(1)`)**

- **Chase:** Perşembe orta görevindeki `Chase_Step` benzeri.  
- **veya Blink:** Perşembe kolay görevindeki `led_on` toggle.

**3. Buton (`if`)**

- Polling: `if (Gpio_Read(BTN) == basili)` → LED toggle veya `reverse = !reverse`.  
- **veya** kesme: `if (button_flag)` → flag temizle → aynı iş.

**4. Minimum C kontrol listesi**

- [ ] En az **1** `for` (self-test veya chase içinde)  
- [ ] En az **2** `if` veya `else`  
- [ ] **1** `while(1)`  
- [ ] **1** sayaç: `uint8_t` / `uint32_t` (index, tick, vb.)

### Doğru çalışıyor mu?

- [ ] Reset sonrası LED’ler bir kez sırayla yanıp sönüyor.  
- [ ] Sonra sürekli chase veya blink var.  
- [ ] Butona basınca davranış **gözle görülür** şekilde değişiyor.  
- [ ] Program kilitlenmiyor (ISR’da uzun delay yok).

---

## Orta

### Amaç

Kodu **dosyalara bölmek** ve **üç kullanıcı senaryosu** tanımlamak. `main` sadece “orkestra şefi” olur; LED ve buton ayrı modüllerde.

| | |
|---|---|
| **Görev** | `led.c` + `button.c` + 3 senaryo |
| **Yapıldı** | ☐ |

### Önerilen dosya yapısı

```
proje/
├── main.c
├── led.h
├── led.c
├── button.h
└── button.c
```

| Dosya | Fonksiyon örnekleri | Ne iş yapar? |
|-------|---------------------|--------------|
| `led.c` | `Led_Init`, `Led_SelfTest`, `Led_ChaseStep` | Pin’ler, for döngüsü, chase index |
| `button.c` | `Button_Init`, `Button_Update` veya flag okuma | Debounce veya EXTI init |
| `main.c` | `main` | Init + `while(1)` + senaryolara tepki |

### Üç senaryo — ne demek?

Kartta **gösterebileceğin** üç farklı kullanım. Örnek set:

| # | Senaryo | Nasıl tetiklenir? | Beklenen sonuç |
|---|---------|-------------------|----------------|
| 1 | Normal chase | (varsayılan) | LED’ler sırayla akar |
| 2 | Kısa basış / tek basış | Buton | Bir LED toggle **veya** yön değişir |
| 3 | Uzun basış **veya** ikinci davranış | Basılı tut ≥ ~1 sn | Chase hızlanır / yavaşlar / tüm LED flash |

UART yoksa 3. senaryo “uzun basış” olabilir; UART varsa isteğe bağlı `printf("mode=2\n")` ekleyebilirsin.

### `main` iskeleti (örnek)

```c
int main(void)
{
    System_Init();
    Led_Init();
    Button_Init();

    Led_SelfTest();

    while (1) {
        ButtonEvent_t ev = Button_Update();   /* veya flag kontrolü */

        if (ev == BTN_SHORT) {
            /* senaryo 2 */
        } else if (ev == BTN_LONG) {
            /* senaryo 3 */
        }

        Led_ChaseStep();
        DelayMs(chase_delay_ms);
    }
}
```

### Adım adım

1. Önce kolay görevi çalışır halde bırak.  
2. `Led_ChaseStep` ve `Led_SelfTest`’i `led.c`’ye taşı; `led.h`’de prototip.  
3. Buton kodunu `button.c`’ye taşı.  
4. `main.c`’de sadece çağrılar kalsın — mümkünse **40 satır civarı**.  
5. Üç senaryoyu kartta sırayla dene; rapora hangi basışın ne yaptığını yaz.

### Doğru çalışıyor mu?

- [ ] Proje derleniyor, `#include "led.h"` / `button.h` doğru.  
- [ ] Self-test + chase çalışıyor.  
- [ ] Salı **debounce** veya Çarşamba **kesme** projede (en az biri).  
- [ ] Üç senaryo kartta gösterilebilir.  
- [ ] `for` chase veya self-test’te kullanılıyor.

---

## Zor

### Amaç

**Mini panel** hissi: programın bir **modu** var (`enum`), buton modu değiştiriyor (`switch`). Hafta 1 demosu + GitHub + kısa sunum provası.

| | |
|---|---|
| **Görev** | `enum` mod + demo + `2026-07-24/` |
| **Yapıldı** | yapıldı |

### Mod kavramı

Tek bir `bool` yerine “uygulama şu an ne yapıyor?” sorusuna cevap:

```c
typedef enum {
    MODE_CHASE_SLOW,
    MODE_CHASE_FAST,
    MODE_PAUSE
} AppMode_t;

static AppMode_t app_mode = MODE_CHASE_SLOW;
```

`while(1)` içinde:

```c
switch (app_mode) {
case MODE_CHASE_SLOW:
    chase_delay_ms = 200;
    Led_ChaseStep();
    break;
case MODE_CHASE_FAST:
    chase_delay_ms = 80;
    Led_ChaseStep();
    break;
case MODE_PAUSE:
    Led_AllOff();
    break;
default:
    break;
}
```

Buton olayları modu değiştirir:

```c
if (ev == BTN_SHORT) {
    /* örn. slow ↔ fast */
} else if (ev == BTN_LONG) {
  app_mode = MODE_PAUSE;  /* veya pause'tan çık */
}
```

### Zorunlu teknik liste

- [ ] `enum` + `switch` **veya** uzun `if / else if` zinciri  
- [ ] `for` (self-test veya chase)  
- [ ] `while(1)`  
- [ ] `const` en az bir süre/eşik sabiti  
- [ ] Debounce **veya** kesme + (`volatile` gerekirse)  
- [ ] Modüler dosya (orta görev gibi)  
- [ ] GitHub `2026-07-24/` + README  

### Demo provası (8–10 dk)

Kendine veya arkadaşına anlatır gibi sırayla:

1. **Reset** → “Şimdi self-test; `for` ile her LED’i kontrol ediyorum.”  
2. **Normal** → “Chase yavaş modda; `while` içinde `Led_ChaseStep`.”  
3. **Kısa basış** → “Mod hızlandı / yön değişti.”  
4. **Uzun basış** → “Pause veya fast mod.”  
5. (Varsa) UART satırı.

Demo sırasında **hangi C yapısının nerede olduğunu** bir cümleyle söylemen rapora da yazılacak.

### GitHub README’de olması iyi olanlar

- Proje adı ve 2 cümle özet  
- Hafta 1 tablosu: hangi özellik hangi günden  
- C yapıları listesi (`for` → …, `volatile` → …)  
- Derleme / kart notu (kısa)

### Doğru çalışıyor mu?

- [ ] En az 2 mod arasında geçiş butonla yapılabiliyor.  
- [ ] Pause modunda chase duruyor veya LED’ler sönük.  
- [ ] Kod okunaklı; `main` şişmemiş.  
- [ ] Repo linki raporda.

---

## Rapora yaz (Cuma)

`gunluk_rapor.md` — detaylı ama kendi cümlelerin:

### 1. Hafta tablosu (örnek format)

| Gün | Projede karşılığı |
|-----|-------------------|
| Pazartesi | mikrodenetleyicinin ilgili pinlerini "Push-Pull Output" (Çıkış) olarak ayarlamayı ve başlangıçta LED'leri kapalı (HIGH) konumda tutmayı öğrendim. |
| Salı | Butona basıldığında oluşan mekanik titreşimleri donanımsal değil, yazılımsal olarak çözmeye odaklandım. |
| Çarşamba | İşlemcinin sürekli butonu beklemesi yerine olay anında tepki vermesi kavramı üzerine çalıştım. |
| Perşembe | for döngüsü yardımıyla tüm LED'leri sırayla test ettim. Ardından Chase_Step mantığıyla işlemciyi kilitlemeden çalışan kayan ışık algoritmasını kurguladım. |
| Cuma |  AppMode_t kullanarak mod kavramını oluşturduk. Bu modları ana döngüde switch-case yapısıyla kontrol ettim. |

### 2. C özeti

Her madde için **fonksiyon adı** yeter:

- `for` → Led_SelfTest, Led_Init, Led_AllOff ve Get_System_Time_ms
- `while` → PRV_USER_Code ve Delay_Dummy
- `if` → Button_Process  ve PRV_USER_Code 
- `switch` → PRV_USER_Code (Modların kontrolü - switch)
- `const` → button.c içindeki DEBOUNCE_MS / LONG_PRESS_MS eşik süreleri ve led.c içindeki pin/port dizileri. 
- `volatile` → Get_System_Time_ms 

### 3. Takıldığın yer

Projenin başlarında butona bastığımda sistemin tepki vermemesi veya hızın değişmemesi sorunuyla karşılaştım. bu sorunu sürekli artan bir sayaç  ile engelleyici olmayan bir zamanlayıcı kurgulayarak çözdüm

### 4. Link

GitHub: https://github.com/burakucar55/burak_empastaj/tree/main/stajyer_B/24.07.2026/24.07proje

---

## Teslim

```
teslimler/Stajyer_X/
├── rapor/gunluk_rapor.md    ← tablo + C özeti + link
└── proje/
```

GitHub (zor): `2026-07-24/`
