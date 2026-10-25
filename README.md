# Robotika ESP32 Control System

<div align="center">

![ESP32](https://img.shields.io/badge/ESP32-DC%20Motor%20Control-00A7A7?style=for-the-badge&logo=arduino)
![C%2B%2B](https://img.shields.io/badge/Language-C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B)
![Python](https://img.shields.io/badge/Language-Python-3776AB?style=for-the-badge&logo=python)
![IoT](https://img.shields.io/badge/Platform-Blynk%20IoT-1E88E5?style=for-the-badge)

</div>

<p align="center">
  <strong>Smart Motor Control Project based on ESP32</strong>
</p>

Repository ini berisi proyek robotika berbasis ESP32 untuk pengendalian motor DC, integrasi dengan Blynk IoT, serta fitur OTA (Over-The-Air Update). Proyek ini dibuat sebagai bagian dari pembelajaran dan implementasi kontrol perangkat elektronik secara real-time.

## About the Project

Pada repositori ini, terdapat beberapa tahapan pengembangan yang mencakup:

- Motor DC basic control via Serial Monitor
- Kontrol motor dengan Blynk IoT
- Kontrol motor dengan Blynk + OTA
- Interface Python GUI untuk monitoring / penerimaan data
- Dokumentasi modul setup dan laporan proyek

Tujuan utama dari proyek ini adalah membangun sistem robotika yang dapat dikendalikan secara otomatis maupun manual dengan kecepatan, arah, dan monitoring yang lebih interaktif.

## Creator

Dibuat oleh:

- Eka Ahmad Ferdinand Mahtrus

Dikelola dan dikembangkan dalam lingkungan:

- Universitas Pembangunan Nasional "Veteran" Jawa Timur

## Project Highlights

- Kontrol motor DC menggunakan ESP32
- PWM untuk pengaturan kecepatan motor
- Integrasi dengan aplikasi Blynk
- Fitur OTA update untuk pembaruan firmware dari jarak jauh
- Monitoring melalui Serial Monitor dan Python GUI
- Struktur proyek yang terorganisir sesuai tahapan pengembangan

## Repository Structure

```text
Robotika/
├── ETS_ROBOTIKA/
│   ├── Basic/
│   │   ├── Basic.ino
│   │   └── Laporan (Basic).pdf
│   ├── Basic + BLYNK/
│   │   ├── Basic_BLYNK.ino
│   │   └── Laporan (Basic + BLYNK).pdf
│   ├── Basic + BLYNK + OTA/
│   │   ├── Basic_Blynk_OTA.ino
│   │   └── Laporan (Basic + BLYNK + OTA).pdf
│   ├── Python/
│   │   ├── ESP32_Receiver/
│   │   ├── Tkinter GUI.py
│   │   ├── Laporan (Python).pdf
│   │   └── ...
│   └── SETUP/
│       └── MODUL SETUP.pdf
├── README.md
└── ...
```

## Main Features

### 1. Basic Motor Control
Program `Basic.ino` mengimplementasikan kontrol motor DC dengan:

- ON/OFF motor
- Penambahan dan pengurangan kecepatan
- Input `PWM:xxx` secara manual
- Penggunaan LED indikator

### 2. Blynk IoT Integration
Program `Basic_BLYNK.ino` menambahkan:

- Koneksi WiFi ke ESP32
- Integrasi Blynk dashboard
- Virtual pins untuk kontrol dari smartphone
- Sinkronisasi status motor ke aplikasi

### 3. OTA Update
Program `Basic_Blynk_OTA.ino` memungkinkan:

- Pembaruan firmware tanpa harus mencabut kabel USB
- Pengelolaan perangkat melalui jaringan
- Pengembangan sistem semakin fleksibel dan praktis

### 4. Python GUI
Folder `Python` berisi interface berbasis Python untuk:

- Monitoring data dari ESP32
- Pengolahan data dari perangkat
- GUI interaktif dengan Tkinter

## Tech Stack

- ESP32
- C++ / Arduino
- Python
- Tkinter
- Blynk IoT
- PWM Motor Driver
- Serial Communication

## Hardware Requirements

Beberapa komponen yang umum digunakan pada sistem ini:

- ESP32 DevKit
- Motor DC
- Driver motor (L298N / driver kompatibel)
- LED indikator
- Kabel jumper
- Power supply yang sesuai
- Modem / router WiFi untuk integrasi Blynk

## Getting Started

### 1. Clone Repository

```bash
git clone https://github.com/24081010012-gif/Robotika.git
cd Robotika
```

### 2. Open Project in Arduino IDE

- Buka file `.ino` sesuai tahap proyek yang ingin digunakan
- Pastikan board `ESP32 Dev Module` sudah terpasang di Arduino IDE
- Pilih port COM yang sesuai

### 3. Configure Credentials

Pada proyek yang menggunakan Blynk, sesuaikan:

- `BLYNK_AUTH_TOKEN`
- `ssid`
- `pass`

Pastikan tidak membagikan credential asli ke publik!

### 4. Upload and Run

- Upload program ke ESP32
- Buka Serial Monitor
- Gunakan perintah seperti:

```text
1
0
+
-
PWM:150
```

## Usage Example

### Command List

- `1` → motor ON
- `0` → motor OFF
- `+` → tingkatkan kecepatan
- `-` → turunkan kecepatan
- `PWM:xxx` → atur kecepatan secara manual (0-255)

## Learning Outcomes

Proyek ini membantu mempelajari:

- Kontrol perangkat elektronik berbasis mikrokontroler
- PWM pada motor DC
- Komunikasi serial
- Integrasi IoT dengan Blynk
- Pengembangan firmware untuk ESP32
- Penerapan Python untuk monitoring dan antarmuka pengguna

## Notes

- Gunakan kredensial WiFi dan Blynk yang aman
- Pastikan power supply cukup untuk motor
- Hindari penggunaan kabel yang tidak sesuai agar tidak merusak modul
- Untuk OTA, pastikan perangkat terhubung dengan jaringan yang stabil

## License

Proyek ini dibuat untuk kebutuhan pembelajaran dan pengembangan robotika. Silakan gunakan sesuai kebutuhan dan tetap mencantumkan sumber jika digunakan untuk pengembangan lebih lanjut.

## Closing

Repository ini merupakan bentuk implementasi nyata dari pengendalian motor berbasis ESP32 yang terstruktur dan dapat dikembangkan lebih lanjut untuk proyek robotika dan sistem otomatisasi di masa depan.

Terima kasih telah mengunjungi repositori ini.

---

<p align="center">
  <strong>Created with dedication by Eka Ahmad Ferdinand Mahtrus</strong><br>
  <strong>Universitas Pembangunan Nasional "Veteran" Jawa Timur</strong>
</p>
