# Sistem Absensi ESP32-CAM

Sistem absensi berbasis IoT menggunakan ESP32-CAM, layar OLED, dan rotary encoder. Pengguna dapat memilih identitas, status kehadiran (IN/OUT), serta lokasi absensi. Perangkat kemudian mengambil foto dan mengirimkan foto beserta informasi kehadiran melalui Telegram Bot menggunakan WiFi.

## Fitur
- Memilih identitas mahasiswa melalui rotary encoder.
- Memilih status kehadiran IN atau OUT.
- Memilih lokasi absensi: Lab FMIPA atau Gedung Haji Anif.
- Menampilkan menu melalui OLED 0,96 inci.
- Mengambil foto menggunakan ESP32-CAM.
- Mengirim foto dan informasi absensi ke Telegram Bot.
- Mengambil informasi waktu melalui NTP.

## Komponen
- ESP32-CAM
- OLED 0,96 inci 128x64 (SSD1306, I2C)
- Rotary encoder dan knob
- Kabel jumper
- Breadboard atau PCB/perfboard
- Kabel USB-to-serial/programmer yang sesuai
- Sumber daya yang sesuai untuk ESP32-CAM

## Perangkat Lunak dan Library
Gunakan Arduino IDE dengan dukungan board ESP32 yang sesuai.

Library eksternal:
- Adafruit GFX Library
- Adafruit SSD1306

Library `WiFi`, `HTTPClient`, `WiFiClientSecure`, `Wire`, `time`, `esp_camera`, dan `esp_task_wdt` berasal dari core/SDK ESP32 yang sesuai.

## Konfigurasi
Sebelum mengunggah program, sesuaikan kredensial berikut di `attendance.ino`:

```cpp
const char *ssid = "NAMA_WIFI";
const char *pwd = "PASSWORD_WIFI";
const char *botToken = "TOKEN_BOT_TELEGRAM";
const char *chatId = "CHAT_ID_TELEGRAM";
```

Ganti data contoh pada array `nims[]` dan `names[]` dengan data yang memang diizinkan untuk digunakan. Jangan mengunggah password, token bot, chat ID, NIM, atau data pribadi ke repository publik.

## Cara Kerja
1. Perangkat dinyalakan dan menginisialisasi OLED serta kamera.
2. ESP32-CAM menghubungkan diri ke WiFi.
3. Daftar identitas ditampilkan pada OLED.
4. Rotary encoder digunakan untuk memilih identitas, lalu tombol ditekan.
5. Pengguna memilih status IN atau OUT.
6. Pengguna memilih lokasi absensi.
7. Perangkat menampilkan hitung mundur dan mengambil foto.
8. Foto beserta informasi absensi dikirim ke Telegram Bot.

## Konfigurasi Pin

### OLED I2C
| OLED | ESP32-CAM |
|---|---|
| SDA | GPIO 14 |
| SCL | GPIO 13 |
| VCC | Sesuaikan dengan modul |
| GND | GND |

Alamat I2C yang dicoba program: `0x3C`, kemudian `0x3D`.

### Rotary Encoder
| Encoder | ESP32-CAM |
|---|---|
| CLK | GPIO 15 |
| DT | GPIO 3 |
| SW | GPIO 2 |
| VCC | Sesuaikan dengan modul |
| GND | GND |

### Flash LED
| Komponen | ESP32-CAM |
|---|---|
| Flash LED | GPIO 4 |

Konfigurasi pin kamera terdapat pada fungsi `initCam()` di `attendance.ino`. Pastikan modul kamera yang digunakan sesuai dengan konfigurasi tersebut.

> Periksa kembali kesesuaian pin dengan rangkaian fisik. Beberapa GPIO ESP32-CAM memiliki fungsi khusus saat proses boot atau komunikasi serial.

## Struktur Folder
```text
ESP32-CAM-Attendance/
├── attendance.ino
├── README.md
├── .gitignore
├── images/
└── docs/
```

## Catatan
File program dalam repository ini merupakan salinan sketch dengan kredensial dan data identitas diganti menjadi placeholder/contoh untuk menjaga keamanan dan privasi. Isi konfigurasi yang benar secara lokal sebelum digunakan.

Paket ini disiapkan sebagai dokumentasi proyek. Program belum diklaim telah dikompilasi atau diuji ulang dari paket ini; lakukan pengujian pada perangkat dan rangkaian yang digunakan.

## Lisensi
Lisensi belum ditentukan. Tambahkan file lisensi jika proyek akan dibagikan atau digunakan kembali oleh pihak lain.
