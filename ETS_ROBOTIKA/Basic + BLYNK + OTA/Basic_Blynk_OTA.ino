/*
 * =======================================================================================
 * Progress 4: ESP32 DC Motor Control with Blynk IoT & OTA (Over-The-Air) Updates
 * Description: Sistem kendali motor DC nirkabel menggunakan aplikasi Blynk 
 *              yang dilengkapi dengan fitur pembaruan firmware jarak jauh (OTA).
 *              Sistem ini memungkinkan pengunggahan program baru via Wi-Fi 
 *              tanpa perlu mencolokkan kabel USB ke ESP32.
 * 
 * PERHATIAN SEBELUM UPLOAD:
 * Ganti parameter BLYNK, WiFi, dan OTA di bawah dengan kredensial milik Anda.
 * JANGAN PERNAH mengunggah Auth Token atau Password asli Anda ke GitHub publik!
 * =======================================================================================
 */

// ============================================================
// 1. KREDENSIAL BLYNK & WIFI (Wajib Diisi)
// ============================================================
// Dapatkan ID, Nama, dan Token ini dari dashboard web Blynk Anda
#define BLYNK_TEMPLATE_ID   "GANTI_DENGAN_TEMPLATE_ID_ANDA"
#define BLYNK_TEMPLATE_NAME "GANTI_DENGAN_TEMPLATE_NAME_ANDA"
#define BLYNK_AUTH_TOKEN    "GANTI_DENGAN_AUTH_TOKEN_ANDA" 

// Memerintahkan pustaka Blynk untuk mencetak log koneksi ke Serial Monitor
#define BLYNK_PRINT Serial 

// Pustaka inti untuk WiFi, Blynk, dan fitur OTA
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Masukkan nama WiFi (SSID) dan kata sandinya
char ssid[] = "GANTI_DENGAN_NAMA_WIFI_ANDA";
char pass[] = "GANTI_DENGAN_PASSWORD_WIFI_ANDA";

// ==========================================================
// 2. DOKUMENTASI VIRTUAL PIN (Datastream Dashboard Blynk)
// ==========================================================
// V0 : Gauge   - Simulasi Putaran Mesin / RPM (0 - 13000)
// V1 : Button  - Sakelar Motor ON/OFF (Mode Switch, 0/1)
// V2 : Slider  - Pengatur Kecepatan Motor PWM (0 - 255)
// V3 : Gauge   - Tampilan Nilai Kecepatan PWM Aktual (0 - 255)

// ==========================================================
// 3. KONFIGURASI PIN HARDWARE & PARAMETER PWM
// ==========================================================
const int motor1Pin1 = 27;    // Pin IN1 Driver Motor (Arah putaran)
const int motor1Pin2 = 26;    // Pin IN2 Driver Motor (Arah putaran)
const int enable1Pin = 12;    // Pin ENA Driver Motor (Kecepatan PWM)

// Pengaturan sinyal PWM (Pulse Width Modulation)
const int freq = 5000;       // Frekuensi 5 kHz agar motor merespons dengan halus
const int resolution = 8;    // Resolusi 8-bit berarti rentang nilai kecepatan adalah 0 - 255
int dutyCycle = 0;           // Variabel penyimpan nilai kecepatan saat ini
bool isMotorRunning = false; // Status logika operasional motor (hidup/mati)

// --- VARIABEL MANAJEMEN WAKTU (NON-BLOCKING DELAY) ---
unsigned long waktuTeksTerakhir = 0; // Penyimpan waktu terakhir teks dicetak
const long jedaTeks = 5000;          // Interval 5000 milidetik (5 detik) untuk indikator siaga

// ==========================================================
// 4. FUNGSI KONTROL MOTOR & SINKRONISASI BLYNK
// ==========================================================

/*
 * Fungsi applyMotor() bertugas menerapkan logika tegangan fisik ke mikrokontroler 
 * sekaligus memancarkan umpan balik (feedback) data kembali ke dashboard Blynk.
 */
void applyMotor() {
  if (isMotorRunning) {
    // 1. Terapkan arah putaran dan kecepatan (PWM) ke hardware
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
    ledcWrite(enable1Pin, dutyCycle);
    
    // 2. Perbarui tampilan Gauge PWM di aplikasi Blynk (V3)
    Blynk.virtualWrite(V3, dutyCycle); 
    
    // 3. Kalkulasi dan perbarui simulasi RPM (V0) menggunakan fungsi pemetaan map()
    int simulasiRPM = map(dutyCycle, 0, 255, 0, 13000);
    Blynk.virtualWrite(V0, simulasiRPM); 
    
  } else {
    // Jika motor dimatikan, putus arus ke driver motor
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, LOW);
    ledcWrite(enable1Pin, 0);
  
    // Reset seluruh indikator di aplikasi Blynk kembali ke titik nol
    Blynk.virtualWrite(V3, 0); 
    Blynk.virtualWrite(V0, 0); 
  }
}

// ==========================================================
// 5. FUNGSI CALLBACK BLYNK (Menerima Instruksi dari Cloud)
// ==========================================================

/*
 * Blok ini dieksekusi otomatis ketika tombol ON/OFF (V1) ditekan di aplikasi Blynk.
 */
BLYNK_WRITE(V1) {
  int buttonState = param.asInt(); // Baca nilai yang dikirim dari aplikasi (1 atau 0)
  
  if (buttonState == 1) {
    isMotorRunning = true;
    // Torsi awal (Starting Torque) agar motor tidak stall saat pertama kali dihidupkan
    if (dutyCycle == 0) dutyCycle = 150; 
    Serial.println("[INFO] [BLYNK] Motor ON");
  } else {
    isMotorRunning = false;
    Serial.println("[INFO] [BLYNK] Motor OFF");
  }
  
  applyMotor(); // Terapkan perubahan ke fisik dan layar
}

/*
 * Blok ini dieksekusi otomatis ketika Slider kecepatan (V2) digeser di aplikasi.
 */
BLYNK_WRITE(V2) {
  dutyCycle = param.asInt(); // Ekstrak nilai slider (0 - 255)
  Serial.print("[INFO] [BLYNK] Slider diubah ke kecepatan: ");
  Serial.println(dutyCycle);
  
  // Jika sakelar motor dalam kondisi ON, langsung terapkan kecepatan baru ke mesin
  if (isMotorRunning) {
    applyMotor(); 
  } else {
    // Jika motor sedang OFF, cukup sinkronkan indikator display tanpa menyalakan mesin
    Blynk.virtualWrite(V3, dutyCycle);
  }
}

// ==========================================================
// 6. SETUP AWAL (Inisialisasi Sistem)
// ==========================================================
void setup() {
  // Membuka jalur komunikasi Serial Monitor dengan komputer
  Serial.begin(115200);

  // Inisialisasi pin hardware menggunakan sintaks ESP32 Core 3.x
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  ledcAttach(enable1Pin, freq, resolution);
  
  // Pastikan sistem berada dalam kondisi aman (mati) saat pertama kali boot
  isMotorRunning = false;
  applyMotor();

  // --------------------------------------------------------
  // A. PROSEDUR KONEKSI WIFI LOKAL
  // --------------------------------------------------------
  Serial.println("\n--- Booting WiFi... ---");
  WiFi.mode(WIFI_STA); // Mode Station (sebagai perangkat penerima sinyal Wi-Fi)
  WiFi.begin(ssid, pass);
  
  // Menunggu koneksi, restart otomatis jika gagal terhubung
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("[ERROR] Koneksi WiFi Gagal! Merestart sistem...");
    delay(3000); 
    ESP.restart();
  }
  
  Serial.println("[INFO] WiFi Terhubung!");
  Serial.print("[INFO] IP Address ESP32: ");
  Serial.println(WiFi.localIP()); // Menampilkan IP lokal yang penting untuk keperluan OTA

  // --------------------------------------------------------
  // B. PROSEDUR KONEKSI CLOUD BLYNK
  // --------------------------------------------------------
  // Menggunakan Blynk.config() alih-alih Blynk.begin() karena koneksi WiFi 
  // sudah ditangani secara manual di blok sebelumnya.
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();
  Serial.println("[INFO] Berhasil Terhubung ke Blynk Cloud!");
  
  // Sinkronisasi status Switch V1 di aplikasi agar kembali ke 0 (OFF) saat boot
  Blynk.virtualWrite(V1, 0); 

  // --------------------------------------------------------
  // C. KONFIGURASI GERBANG OTA (Over-The-Air)
  // --------------------------------------------------------
  // Nama perangkat yang akan muncul di Network Port Arduino IDE
  ArduinoOTA.setHostname("ESP32-Robotika"); 
  
  // Kata sandi untuk mencegah pihak tak berwenang menimpa program via jaringan
  ArduinoOTA.setPassword("GANTI_DENGAN_PASSWORD_OTA_ANDA");                
  
  // Callback saat proses transfer program (OTA) dimulai
  ArduinoOTA.onStart([]() { 
    // KEAMANAN KRUSIAL: Putus tegangan ke motor agar tidak bergerak liar saat sistem di-flash ulang
    ledcWrite(enable1Pin, 0); 
    Serial.println("\n[OTA] MENERIMA DATA FIRMWARE DARI WIFI..."); 
  });
  
  // Callback saat proses transfer selesai
  ArduinoOTA.onEnd([]() { Serial.println("\n[OTA] SUKSES! RESTARTING..."); });
  
  // Callback untuk menampilkan progres transfer dalam persen
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100)));
  });

  // Mulai memonitor sinyal OTA
  ArduinoOTA.begin();
  
  Serial.println("\n--- SISTEM KONTROL BLYNK & OTA SIAP ---");
}

// ==========================================================
// 7. LOOP UTAMA (Eksekusi Rutin Berulang)
// ==========================================================
void loop() {
  // Wajib dipanggil untuk menjaga stabilitas protokol TCP/IP ke server Blynk
  Blynk.run();         
  
  // Wajib dipanggil untuk mengawasi apabila ada permintaan unggah program via jaringan OTA
  ArduinoOTA.handle(); 

  // Mekanisme Non-Blocking Delay (tanpa menggunakan fungsi delay() yang bisa mematikan sistem)
  // Berfungsi untuk mencetak status bahwa ESP32 masih hidup dan bersiaga setiap 5 detik
  if (millis() - waktuTeksTerakhir >= jedaTeks) {
    waktuTeksTerakhir = millis(); 
    Serial.println("[INFO] OTA & Blynk Siaga... Menunggu perintah.");
  }
}