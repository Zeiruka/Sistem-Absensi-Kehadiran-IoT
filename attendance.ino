#include "esp_camera.h"
#include "esp_task_wdt.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <time.h>
 
// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------------- WiFi & Telegram ----------------
const char *ssid = "TM";
const char *pwd = "taqiyyamafaza";
const char *botToken = "9277287731:AAEuTSeD8SMw3NlPRp-l6lpnaS3Bn7SHly4";
const char *chatId = "9088871565";

// ---------------- Lokasi ----------------
String detectedCity = "";
int locIndex = 0;

String locDisplay[] = {
    "Lab FMIPA",
    "Gdg H.Anif"
};

String locFull[] = {
    "Lab FMIPA",
    "Gedung Haji Anif"
};

// ---------------- Encoder ----------------
#define CLK 15
#define DT 3
#define SW 2

// ---------------- Data Mahasiswa Lab B1 (23 Orang) ----------------
String nims[] = {
    "261712046", "261712047", "261712048", "261712049", "261712050",
    "261712051", "261712052", "261712054", "261712055", "261712056",
    "261712057", "261712058", "261712059", "261712060", "261712061",
    "261712062", "261712064", "261712065", "261712066", "261712067",
    "261712068", "251712098", "261712092"
};

String names[] = {
    "Cinta Nora Betta Hutasoit",
    "Berma Benjeva Tambun",
    "Bryan Suhendra Marchtinus Sinaga",
    "Habib Maulana Siregar",
    "Giovano Putra Mangaraja",
    "Dance Trisindo Sianturi",
    "Natali Rahel Rowina Sitepu",
    "Debora Juliana Pandiangan",
    "Elvira Natalia Angraini Hutabarat",
    "Gregorius Hot Hasonangan Purba",
    "Mesakh Jordan",
    "Taqiyya Mafaza",
    "Eibel Pajri Sinaga",
    "Diory Briana Siregar",
    "Willy Harel Mulana Panggabean",
    "Christian Arga Hansdio Sitinjak",
    "Bethania Damayanti Hutagaol",
    "Muhammad Raihan Ikrimah Srg",
    "Moh. Reihan Hanafi",
    "Liven Martua Aritonang",
    "Fitzgerald Otnielovzky Ttyto",
    "Fyrda Syafitri",
    "Raisyah Aulia Lubis"
};

int totalUsers = 23;
int selectedIndex = 0;

// ---------------- Mode ----------------
int modeIndex = 0;

String modeOptions[] = {
    "IN",
    "OUT"
};

// ---------------- State Menu ----------------
int menuStep = 0;
int lastCLK = HIGH;
bool lastSW = HIGH;

unsigned long lastEncoderMove = 0;
unsigned long lastSWPress = 0;

// ---------------- Deteksi Kota Otomatis via WiFi ----------------
void detectAutoCity() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.setTimeout(3000); // Dipercepat timeout-nya
        http.begin("http://ip-api.com");

        int httpCode = http.GET();

        if (httpCode == 200) {
            String city = http.getString();
            city.trim();

            if (city.length() > 0) {
                detectedCity = city;
            }
        }

        http.end();
    }

    Serial.println("Kota terdeteksi: " + detectedCity);
}

// ---------------- Camera Init ----------------
void initCam() {
    camera_config_t cfg;

    cfg.ledc_channel = LEDC_CHANNEL_0;
    cfg.ledc_timer = LEDC_TIMER_0;

    cfg.pin_d0 = 5;
    cfg.pin_d1 = 18;
    cfg.pin_d2 = 19;
    cfg.pin_d3 = 21;
    cfg.pin_d4 = 36;
    cfg.pin_d5 = 39;
    cfg.pin_d6 = 34;
    cfg.pin_d7 = 35;

    cfg.pin_xclk = 0;
    cfg.pin_pclk = 22;
    cfg.pin_vsync = 25;
    cfg.pin_href = 23;

    cfg.pin_sscb_sda = 26;
    cfg.pin_sscb_scl = 27;

    cfg.pin_pwdn = 32;
    cfg.pin_reset = -1;

    cfg.xclk_freq_hz = 20000000;
    cfg.pixel_format = PIXFORMAT_JPEG;
    cfg.frame_size = FRAMESIZE_QVGA; 
    cfg.jpeg_quality = 14; // Dinaikkan sedikit angkanya agar ukuran file foto lebih kecil dan cepat terkirim
    cfg.fb_count = 1;

    if (esp_camera_init(&cfg) != ESP_OK) {
        Serial.println("Camera Init Failed");
        ESP.restart();
    }

    sensor_t *s = esp_camera_sensor_get();
    if (s != NULL) {
        s->set_vflip(s, 1);
        s->set_hmirror(s, 0);
    }
}

// ---------------- OLED Init ----------------
void initOLED() {
    Wire.begin(14, 13);
    Wire.setClock(100000);
    delay(100);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
            Serial.println("OLED FAIL");

            while (true);
        }
    }

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.display();
}

// ---------------- OLED Screens ----------------
void showWelcome() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(5, 20);
    display.println("Attendance");
    display.display();

    delay(1500); // Dipangkas agar booting lebih cepat
}

void showUsers() {
    display.clearDisplay();
    display.setTextSize(2);

    int rowH = 21;
    int visible = 3;

    int startIdx = selectedIndex - 1;

    if (startIdx < 0) {
        startIdx = 0;
    }

    if (startIdx > totalUsers - visible) {
        startIdx = max(0, totalUsers - visible);
    }

    for (int i = startIdx; i < min(startIdx + visible, totalUsers); i++) {
        int y = (i - startIdx) * rowH;

        if (i == selectedIndex) {
            display.fillRect(0, y, SCREEN_WIDTH, rowH, WHITE);
            display.setTextColor(BLACK);
        } else {
            display.setTextColor(WHITE);
        }

        display.setCursor(4, y + 2);
        display.println(nims[i]);
    }

    display.setTextColor(WHITE);
    display.display();
}

void showMode() {
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println(nims[selectedIndex]);

    display.drawLine(0, 10, SCREEN_WIDTH, 10, WHITE);

    display.setTextSize(2);

    if (modeIndex == 0) {
        display.fillRect(0, 14, SCREEN_WIDTH, 22, WHITE);
        display.setTextColor(BLACK);
    } else {
        display.setTextColor(WHITE);
    }

    display.setCursor(4, 16);
    display.println("IN");

    if (modeIndex == 1) {
        display.fillRect(0, 40, SCREEN_WIDTH, 22, WHITE);
        display.setTextColor(BLACK);
    } else {
        display.setTextColor(WHITE);
    }

    display.setCursor(4, 42);
    display.println("OUT");

    display.setTextColor(WHITE);
    display.display();
}

void showLocationMenu() {
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);

    display.println(nims[selectedIndex] + " (" + modeOptions[modeIndex] + ")");
    display.drawLine(0, 10, SCREEN_WIDTH, 10, WHITE);

    display.setTextSize(2);

    if (locIndex == 0) {
        display.fillRect(0, 14, SCREEN_WIDTH, 22, WHITE);
        display.setTextColor(BLACK);
    } else {
        display.setTextColor(WHITE);
    }

    display.setCursor(2, 16);
    display.println(locDisplay[0]);

    if (locIndex == 1) {
        display.fillRect(0, 40, SCREEN_WIDTH, 22, WHITE);
        display.setTextColor(BLACK);
    } else {
        display.setTextColor(WHITE);
    }

    display.setCursor(2, 42);
    display.println(locDisplay[1]);

    display.setTextColor(WHITE);
    display.display();
}

void showMessage(String msg) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println(msg);
    display.display();
}

void showCountdown() {
    for (int i = 3; i >= 1; i--) {
        display.clearDisplay();
        display.setTextSize(4);
        display.setTextColor(WHITE);
        display.setCursor(50, 12);
        display.println(i);
        display.display();

        esp_task_wdt_reset();
        delay(1000);
    }

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(5, 20);
    display.println("Cheese! :)");
    display.display();

    delay(1500); // Diringkas sedikit agar lebih gesit
}

// ---------------- Encoder ----------------
int readEncoderDelta() {
    int clkState = digitalRead(CLK);

    if (clkState == LOW && lastCLK == HIGH) {
        lastCLK = clkState;

        if (millis() - lastEncoderMove < 150) { // Respons encoder dibuat lebih cepat (150ms)
            return 0;
        }

        lastEncoderMove = millis();

        return (digitalRead(DT) != clkState) ? 1 : -1;
    }

    lastCLK = clkState;

    return 0;
}

bool readSWPress() {
    bool swState = digitalRead(SW);

    if (swState == LOW && lastSW == HIGH) {
        lastSW = swState;

        if (millis() - lastSWPress > 250) { // Batas debounce tombol dipercepat jadi 250ms
            lastSWPress = millis();
            return true;
        }

        return false;
    }

    if (swState == HIGH) {
        lastSW = HIGH;
    }

    return false;
}

// ---------------- Telegram Send ----------------
void sendTelegram(uint8_t *imgBuf, size_t imgLen) {
    if (WiFi.status() != WL_CONNECTED) {
        showMessage("Reconnecting\nWiFi...");
        WiFi.reconnect();

        int w = 0;

        while (WiFi.status() != WL_CONNECTED && w < 10) {
            delay(300);
            w++;
            esp_task_wdt_reset();
        }

        if (WiFi.status() != WL_CONNECTED) {
            showMessage("WiFi Lost!\nRestarting...");
            delay(1000);
            ESP.restart();
        }
    }

    WiFiClientSecure client;

    client.setInsecure();
    client.setTimeout(15000); // Timeout dipangkas ke 15 detik agar tidak menggantung lama jika jaringan berat

    showMessage("Connecting\nto server...");
    esp_task_wdt_reset();

    bool connected = false;

    for (int attempt = 1; attempt <= 2; attempt++) {
        if (client.connect("api.telegram.org", 443)) {
            connected = true;
            break;
        }

        esp_task_wdt_reset();
        delay(500);
    }

    if (!connected) {
        Serial.println("Server Connection Failed");
        showMessage("Server Failed!\nRetry later.");
        delay(1500);
        return;
    }

    esp_task_wdt_reset();

    struct tm ti;
    char ts[64] = "Time Unknown";

    if (getLocalTime(&ti, 1000)) {
        strftime(ts, sizeof(ts), "%H:%M:%S, %d %b %Y", &ti);
    }

    String nimMahasiswa = nims[selectedIndex];
    String namaMahasiswa = names[selectedIndex];
    String statusMode = modeOptions[modeIndex];
    String lokasiLengkap = locFull[locIndex];

    if (detectedCity.length() > 0) {
        lokasiLengkap += " (" + detectedCity + ")";
    }

    String caption = "ABSENSI KEHADIRAN\n"
                     "NIM: " + nimMahasiswa + "\n"
                     "Nama: " + namaMahasiswa + "\n"
                     "Status: " + statusMode + "\n"
                     "Lokasi: " + lokasiLengkap + "\n"
                     "Waktu: " + String(ts) + " WIB";

    String b = "ESP32CAMBound7MA4YWxkTrZu0gW";
    String cr = "\r\n";
    String d = "--";

    String body =
        d + b + cr +
        "Content-Disposition: form-data; name=\"chat_id\"" + cr + cr +
        String(chatId) + cr +
        d + b + cr +
        "Content-Disposition: form-data; name=\"caption\"" + cr + cr +
        caption + cr +
        d + b + cr +
        "Content-Disposition: form-data; name=\"photo\"; filename=\"p.jpg\"" + cr +
        "Content-Type: image/jpeg" + cr + cr;

    String endPart = cr + d + b + d + cr;

    int totalLen = body.length() + imgLen + endPart.length();

    client.println("POST /bot" + String(botToken) + "/sendPhoto HTTP/1.1");
    client.println("Host: api.telegram.org");
    client.println("Content-Type: multipart/form-data; boundary=" + b);
    client.println("Content-Length: " + String(totalLen));
    client.println();

    esp_task_wdt_reset();
    showMessage("Uploading\nimage...");

    client.print(body);
    client.write(imgBuf, imgLen);
    client.print(endPart);

    esp_task_wdt_reset();

    // Membaca respons server dengan batas waktu maksimal 3 detik agar tidak macet
    unsigned long timeout = millis();
    while (client.connected() && millis() - timeout < 3000) {
        esp_task_wdt_reset();
        while (client.available()) {
            client.read();
            timeout = millis();
        }
    }

    client.stop();

    // Tampilkan pesan sukses dan Thanks! selama 1.5 detik (lebih cepat dari sebelumnya)
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(15, 15);
    display.println("Absen OK!");
    display.setCursor(20, 40);
    display.println("Thanks!");
    display.display();
    delay(1500);
}

// ---------------- Capture & Send ----------------
void captureAndSend() {
    showCountdown();

    display.clearDisplay();
    display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
    display.display();

    digitalWrite(4, HIGH);
    delay(200); // Flash dipercepat jadi 0.2 detik

    camera_fb_t *fb = esp_camera_fb_get();

    if (fb) {
        esp_camera_fb_return(fb);
    }

    delay(50);

    fb = esp_camera_fb_get();

    digitalWrite(4, LOW);

    if (!fb) {
        Serial.println("Camera capture failed");
        showMessage("Kamera\nGagal!");
        delay(1500);
        return;
    }

    sendTelegram(fb->buf, fb->len);

    esp_camera_fb_return(fb);
}

// ---------------- Setup ----------------
void setup() {
    Serial.begin(115200);

    pinMode(CLK, INPUT_PULLUP);
    pinMode(DT, INPUT_PULLUP);
    pinMode(SW, INPUT_PULLUP);

    pinMode(4, OUTPUT);
    digitalWrite(4, LOW);

    initOLED();
    showWelcome();
    initCam();

    showMessage("Connecting\nWiFi...");

    WiFi.begin(ssid, pwd);

    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        Serial.print(".");
    }

    Serial.println("\nWiFi Connected");

    configTime(25200, 0, "pool.ntp.org", "time.nist.gov");

    detectAutoCity();

    esp_task_wdt_init(20, true);
    esp_task_wdt_add(NULL);

    showUsers();
}

// ---------------- Loop ----------------
void loop() {
    esp_task_wdt_reset();

    int delta = readEncoderDelta();

    if (delta != 0) {
        if (menuStep == 0) {
            selectedIndex += delta;

            if (selectedIndex < 0) {
                selectedIndex = totalUsers - 1;
            }

            if (selectedIndex >= totalUsers) {
                selectedIndex = 0;
            }

            showUsers();

        } else if (menuStep == 1) {
            modeIndex = (modeIndex == 0) ? 1 : 0;
            showMode();

        } else if (menuStep == 2) {
            locIndex = (locIndex == 0) ? 1 : 0;
            showLocationMenu();
        }
    }

    if (readSWPress()) {
        if (menuStep == 0) {
            menuStep = 1;
            modeIndex = 0;
            showMode();

        } else if (menuStep == 1) {
            menuStep = 2;
            locIndex = 0;
            showLocationMenu();

        } else if (menuStep == 2) {
            captureAndSend();
            menuStep = 0;
            showUsers();
        }
    }
}
