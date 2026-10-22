/*
 * =======================================================================================
 * Progress 2: ESP32 DC Motor Control with Blynk IoT & Serial Monitor
 * Description: Mengontrol arah dan kecepatan motor DC menggunakan sinyal PWM.
 *              Sistem ini memiliki dua mode kendali yang berjalan bersamaan:
 *              1. Melalui Dashboard/Aplikasi Blynk (IoT via WiFi).
 *              2. Melalui Serial Monitor (Lokal via kabel USB).
 *
 * PERHATIAN SEBELUM UPLOAD:
 * Ganti parameter BLYNK dan WiFi di bawah dengan kredensial milik Anda.
 * JANGAN PERNAH mengunggah Auth Token atau Password WiFi asli Anda ke GitHub publik!
 * =======================================================================================
 */

// ============================================================
// 1. KONFIGURASI BLYNK (Wajib berada di baris paling atas!)
// ============================================================
// Dapatkan ID, Nama, dan Token ini dari dashboard web Blynk Anda.
#define BLYNK_TEMPLATE_ID   "GANTI_DENGAN_TEMPLATE_ID_ANDA"
#define BLYNK_TEMPLATE_NAME "GANTI_DENGAN_TEMPLATE_NAME_ANDA"
#define BLYNK_AUTH_TOKEN    "GANTI_DENGAN_AUTH_TOKEN_ANDA"

// Mengarahkan log proses koneksi Blynk ke Serial Monitor untuk keperluan debugging
#define BLYNK_PRINT Serial  

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// ============================================================
// 2. KONFIGURASI WIFI / HOTSPOT
// ============================================================
// Masukkan nama WiFi (SSID) dan kata sandinya agar ESP32 bisa terhubung ke internet
char ssid[] = "GANTI_DENGAN_NAMA_WIFI_ANDA";
char pass[] = "GANTI_DENGAN_PASSWORD_WIFI_ANDA";

// ============================================================
// 3. KONFIGURASI PIN HARDWARE & PARAMETER PWM
// ============================================================
const int ledPin = 2;       // Pin LED built-in ESP32 sebagai indikator motor aktif
const int motor1Pin1 = 27;  // Pin IN1 pada driver motor (untuk arah putaran)
const int motor1Pin2 = 26;  // Pin IN2 pada driver motor (untuk arah putaran)
const int enable1Pin = 12;  // Pin ENA pada driver motor (untuk kendali kecepatan PWM)

// Pengaturan sinyal PWM (Pulse Width Modulation)
const int freq = 5000;       // Frekuensi PWM disetel ke 5000 Hz
const int resolution = 8;    // Resolusi 8-bit berarti rentang nilai kecepatan adalah 0 - 255
int dutyCycle = 0;           // Variabel penyimpan nilai kecepatan saat ini (Mulai dari 0 / Mati)

const int DUTY_STEP = 10;    // Besar penambahan/pengurangan kecepatan tiap kali tombol ditekan
const int DUTY_MIN  = 0;     // Batas kecepatan minimum (Berhenti)
const int DUTY_MAX  = 255;   // Batas kecepatan maksimum (Putaran Penuh)

String inputString = "";     // Buffer penyimpan sementara karakter yang masuk dari Serial Monitor

// ============================================================
// 4. FUNGSI INTI KONTROL MOTOR & SINKRONISASI CLOUD
// Mapping Datastreams (Blynk Virtual Pins):
// V0 = RPM        -> Gauge/Indikator kecepatan motor (0 - 13000)
// V1 = ONOFF      -> Tombol Switch (0 mati, 1 nyala)
// V2 = PWMSLIDER  -> Slider pengatur kecepatan (0 - 255)
// V3 = PWMDISPLAY -> Teks/Angka penampil nilai PWM (0 - 255)
// ============================================================

/*
 * Fungsi applyMotor() adalah jantung dari program ini.
 * Fungsi ini bertugas memperbarui status hardware secara fisik sekaligus 
 * menyinkronkan status tersebut kembali ke aplikasi Blynk.
 */
void applyMotor()
{
  // 1. Terapkan arah putaran motor (HIGH & LOW = maju searah jarum jam)
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  
  // 2. Kirim sinyal PWM ke driver motor untuk mengatur kecepatan (Sintaks ESP32 Core 3.x)
  ledcWrite(enable1Pin, dutyCycle); 

  // 3. Logika Indikator LED: Nyala jika duty cycle lebih dari 0, sebaliknya mati.
  digitalWrite(ledPin, dutyCycle > 0 ? HIGH : LOW);

  // 4. Hitung estimasi RPM dengan memetakan (mapping) rasio PWM (0-255) ke RPM (0-13000)
  int estimasiRPM = map(dutyCycle, 0, 255, 0, 13000);

  // 5. Dorong (push) pembaruan data ke Dashboard Blynk agar tampilan di HP/Web ikut berubah
  Blynk.virtualWrite(V0, estimasiRPM);              
  Blynk.virtualWrite(V1, dutyCycle > 0 ? 1 : 0);    
  Blynk.virtualWrite(V2, dutyCycle);                
  Blynk.virtualWrite(V3, dutyCycle);                
}

/* Kumpulan Fungsi Aksi Motor */
void MotorOn()
{
  if (dutyCycle == 0) dutyCycle = 200; // Berikan torsi lonjakan awal agar motor tidak 'stall'
  applyMotor();
  Serial.print("[INFO] Motor ON, duty = ");
  Serial.println(dutyCycle);
}

void MotorOff()
{
  dutyCycle = 0; // Set kecepatan ke 0
  applyMotor();
  Serial.println("[INFO] Motor OFF, duty = 0");
}

void naikkanKecepatan()
{
  dutyCycle += DUTY_STEP;
  if (dutyCycle > DUTY_MAX) dutyCycle = DUTY_MAX; // Mencegah nilai melampaui 255
  applyMotor();
  Serial.print("[INFO] Kecepatan NAIK, duty = ");
  Serial.println(dutyCycle);
}

void turunkanKecepatan()
{
  dutyCycle -= DUTY_STEP;
  if (dutyCycle < DUTY_MIN) dutyCycle = DUTY_MIN; // Mencegah nilai negatif
  applyMotor();
  Serial.print("[INFO] Kecepatan TURUN, duty = ");
  Serial.println(dutyCycle);
}

// Fungsi untuk menerima pengaturan kecepatan secara presisi (angka spesifik)
void setDuty(int val)
{
  // Validasi keamanan rentang nilai masukan
  if (val < DUTY_MIN) val = DUTY_MIN;
  if (val > DUTY_MAX) val = DUTY_MAX;
  
  dutyCycle = val;
  applyMotor();
  Serial.print("[INFO] Set duty = ");
  Serial.println(dutyCycle);
}

// ============================================================
// 5. FUNGSI CALLBACK BLYNK (Menerima perintah dari Cloud)
// ============================================================

/*
 * Fungsi ini otomatis tereksekusi saat pengguna menekan tombol Switch di aplikasi Blynk (Pin V1).
 */
BLYNK_WRITE(V1)
{
  int tombolOnOff = param.asInt(); // Ambil nilai yang dikirim dari aplikasi (0 atau 1)
  if (tombolOnOff == 1) {
    MotorOn();
  } else {
    MotorOff();
  }
}

/*
 * Fungsi ini otomatis tereksekusi saat pengguna menggeser Slider di aplikasi Blynk (Pin V2).
 */
BLYNK_WRITE(V2)
{
  int nilaiSlider = param.asInt(); // Ambil nilai posisi slider (0 hingga 255)
  setDuty(nilaiSlider);
}

// ============================================================
// 6. SETUP & LOOP UTAMA
// ============================================================
void setup()
{
  // Inisialisasi pin sebagai keluaran (OUTPUT)
  pinMode(ledPin, OUTPUT);
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);

  // Memulai jalur komunikasi Serial ke komputer
  Serial.begin(115200);

  // Inisialisasi PWM menggunakan standar library ESP32 Core versi 3.x ke atas
  ledcAttach(enable1Pin, freq, resolution);

  // Pastikan motor dalam kondisi aman (berhenti) saat mikrokontroler baru dihidupkan
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  ledcWrite(enable1Pin, 0);

  Serial.println("\nMenghubungkan ke WiFi & Server Blynk...");
  
  // Memulai proses autentikasi dan koneksi ke server Blynk IoT
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Lakukan sinkronisasi pertama kali agar tampilan di aplikasi Blynk menyesuaikan hardware (0)
  applyMotor();

  // Pesan bahwa sistem siap digunakan
  Serial.println("=========================================");
  Serial.println(" ESP32 Motor Controller + Blynk Siap.    ");
  Serial.println("=========================================");
  Serial.println("Perintah Serial: 1=ON, 0=OFF, +=naik, -=turun, PWM:xxx=set");
}

void loop()
{
  // Perintah krusial agar ESP32 terus merespons "ping" dan membaca data dari server Blynk
  Blynk.run();

  // Logika pembacaan instruksi cadangan dari antarmuka Serial Monitor
  while (Serial.available() > 0)
  {
    char c = Serial.read(); // Baca satu per satu karakter yang dikirim komputer

    // Jika mendeteksi karakter Enter (\n atau \r), berarti perintah selesai diketik
    if (c == '\n' || c == '\r')
    {
      inputString.trim(); // Buang spasi kosong di awal/akhir
      
      // Deteksi perintah set manual (Contoh input: "PWM:150")
      if (inputString.startsWith("PWM:"))
      {
        int val = inputString.substring(4).toInt(); // Ambil angka setelah teks "PWM:"
        setDuty(val);
      }
      // Deteksi perintah karakter tunggal
      else if (inputString.length() > 0)
      {
        char cmd = inputString.charAt(0);
        if (cmd == '1')      MotorOn();
        else if (cmd == '0') MotorOff();
        else if (cmd == '+') naikkanKecepatan();
        else if (cmd == '-') turunkanKecepatan();
        else {
          Serial.print("[ERROR] Perintah tidak dikenal: ");
          Serial.println(inputString);
        }
      }
      
      // Kosongkan buffer agar siap menerima perintah Serial berikutnya
      inputString = "";
    }
    else
    {
      // Kumpulkan karakter ke dalam variabel penyimpan
      inputString += c;
    }
  }
}