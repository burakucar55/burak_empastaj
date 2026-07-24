# Gün 04 — Görevler

Temel C uygulaması (kart üzerinde)

Bugün **teori kısa, kod uzun**. Pazartesi–Çarşamba kartta LED, buton, debounce ve kesme gördün; Perşembe’de aynı işleri **`if`**, **`while`**, **`for`** ve doğru **veri türleri** ile kendin yazıyorsun.

Önce oku: [`01_Anlatim.md`](01_Anlatim.md)  
Sıra: **Kolay → Orta → Zor**. Bir seviyeyi bitirmeden diğerine geçme.

---

## Bu günün mantığı

| Seviye | C odağı | Kartta ne göreceksin? |
|--------|---------|------------------------|
| Kolay | `if`, `while(1)`, `const`, `bool` | LED yanıp sönüyor; buton hızı değiştiriyor |
| Orta | `for`, dizi, index, yön `if` | 3+ LED sırayla akıyor; buton yönü çeviriyor |
| Zor | Hepsi + haftadan kesme/debounce + dosya ayırma | Açılış testi, chase, akıllı buton, GitHub |

---

## Ortak hazırlık

- [ ] Kart USB ile bağlı, **Build** hatasız
- [ ] [`01_Anlatim.md`](01_Anlatim.md) okundu (en az veri türleri + if/while/for bölümleri)
- [ ] Elinde çalışan bir GPIO projesi var (Pazartesi veya örnek proje)
- [ ] Buton hangi seviyede basılı (0 mı 1 mi) — kart şemasına veya dünkü koda bak

Takılırsan: önce tek LED yak/söndür; sonra döngü; en son buton ekle.

---

## Kolay

### Amaç

Blink’i **senin yazdığın** `if` ve `while(1)` ile çalıştırmak. Butona basınca yanıp sönme **hızı** değişsin. Böylece “sürekli dönen program + koşula göre karar” mantığını elle tutmuş olursun.

| | |
|---|---|
| **Görev** | Blink + butonla hız değiştir (polling) |
| **Yapıldı** | Yapıldı |

### Hangi C yapıları zorunlu?

| Yapı | Nerede kullanılır? |
|------|---------------------|
| `bool led_on` | LED şu an yakılı mı? |
| `const uint32_t BLINK_SLOW_MS = 500` | Yavaş blink süresi (değişmesin) |
| `const uint32_t BLINK_FAST_MS = 200` | Hızlı blink süresi |
| `bool fast_mode` veya `int mode` | Butonla seçilen hız |
| `while (1)` | Program hiç bitmesin |
| `if` / `else` | LED yak/söndür; hangi süre kullanılsın |

### Adım adım ne yapacaksın?

**1. Değişkenleri tanımla (RAM)**

```c
bool led_on = false;
bool fast_mode = false;   /* false = yavaş, true = hızlı */
const uint32_t BLINK_SLOW_MS = 500;
const uint32_t BLINK_FAST_MS = 200;
```

`const` sayılar Flash’ta kalır; `led_on` ve `fast_mode` çalışırken değişir.

**2. `main` içinde init**

GPIO’yu daha önceki günlerde yaptığın gibi ayarla: LED **output**, buton **input** (+ pull).

**3. `while(1)` döngüsünü kur**

Her turda sıra şöyle olabilir:

1. Butonu oku (`Gpio_Read` veya sürücün ne ise).  
2. `if (buton_basildi)` → `fast_mode = !fast_mode` veya `fast_mode = true/false` toggle.  
   - Basılı tutunca sürekli toggle olmasın diye: “kenar” mantığı veya basit debounce ekleyebilirsin (isteğe bağlı kolay seviyede).  
3. `if (led_on)` → LED yak; `else` → LED söndür.  
4. Süre seç:  
   ```c
   uint32_t delay_ms;
   if (fast_mode) {
       delay_ms = BLINK_FAST_MS;
   } else {
       delay_ms = BLINK_SLOW_MS;
   }
   ```  
5. `DelayMs(delay_ms)` veya tick ile bekle.  
6. `led_on = !led_on;` — bir sonraki turda ters durum.

**4. Kartta dene**

- Açılışta yavaş blink (veya seçtiğin varsayılan).  
- Butona bas → hız belirgin şekilde değişmeli.

### Doğru çalışıyor mu?

- [ ] LED düzenli yanıp sönüyor (takılı kalmıyor).  
- [ ] Buton en az bir şeyi değiştiriyor (hız veya mod).  
- [ ] Kodda `while(1)` var.  
- [ ] En az 2 yerde `if` veya `else` var.  
- [ ] En az bir `const` süre sabiti var.

### Sık hata

| Belirti | Olası neden |
|---------|-------------|
| LED hep yanık | `led_on` hiç toggle olmuyor |
| Buton hiç etki etmiyor | Yanlış active-low/high; `if` koşulu ters |
| Çok hızlı titreme | Basılı tutunca `fast_mode` sürekli flip — debounce veya “basıldı anı” kullan |

---

## Orta

### Amaç

**`for` döngüsü** ile birden fazla LED’i yönetmek. Chase (kayan ışık) Pazartesi zor görevinin C tarafıdır: index + dizi + döngü.

| | |
|---|---|
| **Görev** | 3+ LED chase + butonla yön değiştir |
| **Yapıldı** | yapıldı |

### Hangi C yapıları zorunlu?

| Yapı | Nerede? |
|------|---------|
| `const` pin dizisi veya `#define LED_COUNT 3` | Kaç LED var |
| `for (int i = 0; i < LED_COUNT; i++)` | Hepsini tek tek sür |
| `uint8_t active_index` | Hangi LED yanacak |
| `bool reverse` | İleri mi geri mi |
| `if` | Sınıra gelince yön değiştir |

### Adım adım

**1. Pin listesi**

```c
#define LED_COUNT 3
static const uint8_t led_pin[LED_COUNT] = { PIN_LED0, PIN_LED1, PIN_LED2 };
```

Pin isimlerini kendi projenle değiştir.

**2. State**

```c
uint8_t active_index = 0;
bool reverse = false;
```

**3. Bir chase adımı fonksiyonu (önerilir)**

```c
void Chase_Step(void)
{
    for (int i = 0; i < LED_COUNT; i++) {
        if (i == active_index) {
            Gpio_Write(led_pin[i], 1);   /* active-high ise; kartına göre 0 */
        } else {
            Gpio_Write(led_pin[i], 0);
        }
    }

    if (reverse) {
        if (active_index == 0) {
            reverse = false;
        } else {
            active_index--;
        }
    } else {
        if (active_index >= LED_COUNT - 1) {
            reverse = true;
        } else {
            active_index++;
        }
    }
}
```

**4. `while(1)` içinde**

- Buton basıldıysa `reverse = !reverse;` (debounce/kesme isteğe bağlı).  
- `Chase_Step();`  
- Kısa gecikme (ör. 100–200 ms) — chase hızı.

**5. Test**

LED’ler 0→1→2→1→0… gibi aksın; buton yönü tersine çevirsin.

### Doğru çalışıyor mu?

- [ ] `for` ile tüm LED’ler her adımda güncelleniyor.  
- [ ] Aynı anda **tek** LED yanar (chase).  
- [ ] `if` ile sınır veya yön var.  
- [ ] Chase `while(1)` içinde durmadan devam ediyor.

### İpucu

Önce `for` ile hepsini sırayla yakıp söndür (self-test). Chase’e geçince sadece `active_index` değişir.

---

## Zor

### Amaç

Haftanın konularını **tek projede** toplamak ve kodu **dosyalara bölmek**. C tarafında `for` + `if` + `while` zaten var; üstüne Salı **veya** Çarşamba buton mantığı gelir.

| | |
|---|---|
| **Görev** | Self-test + chase + debounce/kesme + modüler dosya + GitHub |
| **Yapıldı** | ☐ |

### Seçenekler (buton — birini seç)

| Seçenek | Hafta | C’de ne görürsün? |
|---------|-------|-------------------|
| **A — Kesme** | Çarşamba | `volatile bool button_flag;` + ISR + `if (button_flag)` |
| **B — Debounce + süre** | Salı | `uint32_t down_ms`, `if/else if` kısa/uzun |

İkisi de olabilir; zor seviye için **biri düzgün çalışsın** yeter.

### Adım adım

**1. Açılış — `for` self-test**

```c
for (int i = 0; i < LED_COUNT; i++) {
    Gpio_Write(led_pin[i], 1);
    DelayMs(150);
    Gpio_Write(led_pin[i], 0);
}
```

Kart açılınca LED’ler sırayla bir kez yanıp söner.

**2. Ana döngü — chase**

Orta görevdeki `Chase_Step` veya benzeri `while(1)` içinde.

**3. Buton davranışı**

- **Kesme:** ISR sadece `button_flag = true`. `main` içinde:
  ```c
  if (button_flag) {
      button_flag = false;
      /* kısa: yön değiştir veya LED toggle */
  }
  ```
- **Debounce + kısa/uzun:** Salı’daki mantık; `if (duration < 1000)` / `else` uzun → chase hızını veya yönü değiştir.

**4. `switch` veya `else if` (okunaklılık)**

```c
if (event == SHORT_PRESS) {
    reverse = !reverse;
} else if (event == LONG_PRESS) {
    fast_mode = !fast_mode;
}
```

**5. Dosyalara böl (minimum)**

| Dosya | İçerik |
|-------|--------|
| `led.h` / `led.c` | `Led_Init`, `Led_SelfTest`, `Led_ChaseStep` |
| `button.h` / `button.c` | `Button_Init`, okuma veya `Button_Update` |
| `main.c` | `System_Init`, `while(1)` |

`main.c` mümkün olduğunca kısa: init + döngü + fonksiyon çağrıları.

**6. GitHub**

- Klasör: `2026-07-23/`  
- README örneği:
  - `while(1)` → `main` döngüsü  
  - `for` → `Led_SelfTest` veya chase  
  - `if` → buton / flag  
  - `volatile` → (varsa) `button_flag`  
  - Debounce veya EXTI hangi günden

### Doğru çalışıyor mu?

- [ ] Self-test açılışta çalışıyor.  
- [ ] Chase çalışıyor.  
- [ ] Buton anlamlı bir şey değiştiriyor.  
- [ ] En az 2 `.c` + 1 `.h` (main hariç veya dahil).  
- [ ] Repo linki `gunluk_rapor.md` içinde.

---

## Rapora yaz (Perşembe)

`gunluk_rapor.md` — kendi cümlelerinle, 5–10 cümle toplam:

1. `for` döngüsünü hangi fonksiyonda kullandın, kaç LED vardı? 

1) for döngüsünü Get_System_Time_ms() fonksiyonu içinde kullandım. toplam 1 led vardı.( kolay gorev ).

2. `if` ile butonu veya `fast_mode` / `reverse` nasıl kontrol ettin?  

2) Butona basılı tutma hatasını engellemek için if ile kenar yakalama yapıldı.( kolay gorev )

3. `const` bir sabit ile normal değişken (`led_on` gibi) farkını **kendi kodundan** örnekle.  

3) BLINK_FAST_MS gibi program boyunca hiç degismeyen sabit süreleri const ile tanımlayarak korumaya alındı. 
Ancak led_on ve fast_mode gibi çalısırken sürekli güncellenen durumları normal degisken olarak tanımlandı. ( kolay gorev )

4. (Zor yaptıysan) Kesme mi debounce mu; neden onu seçtin?


---

## Teslim

```
teslimler/Stajyer_X/
├── rapor/gunluk_rapor.md
└── proje/
```

GitHub (zor görev): `2026-07-23/`
