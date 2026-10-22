// =======================================================================================
// Progress 2: Kontrol Motor DC berbasis ESP32 terintegrasi Blynk IoT
// Description: Mengontrol kecepatan motor DC menggunakan PWM melalui Serial Monitor 
//              dan Dashboard Blynk.
//
// PERHATIAN SEBELUM UPLOAD:
// Ganti parameter BLYNK dan WiFi di bawah dengan kredensial milik Anda.
// JANGAN PERNAH mengunggah Auth Token atau Password WiFi asli Anda ke GitHub publik!
// =======================================================================================

// 1. KONFIGURASI BLYNK (Wajib berada di baris paling atas!)
#define BLYNK_TEMPLATE_ID   "GANTI_DENGAN_TEMPLATE_ID_ANDA"
#define BLYNK_TEMPLATE_NAME "GANTI_DENGAN_TEMPLATE_NAME_ANDA"
#define BLYNK_AUTH_TOKEN    "GANTI_DENGAN_AUTH_TOKEN_ANDA"

#define BLYNK_PRINT Serial  // Menampilkan status koneksi Blynk di Serial Monitor

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// 2. KONFIGURASI WIFI / HOTSPOT
char ssid[] = "GANTI_DENGAN_NAMA_WIFI_ANDA";
char pass[] = "GANTI_DENGAN_PASSWORD_WIFI_ANDA";

// 3. KONFIGURASI PIN & PWM MOTOR
const int ledPin = 2;
const int motor1Pin1 = 27;
const int motor1Pin2 = 26;
const int enable1Pin = 12;

const int freq = 5000;       // Frekuensi PWM 5000 Hz
const int resolution = 8;    // Resolusi 8-bit (0 - 255)
int dutyCycle = 0;           // Mulai dalam kondisi mati (0)
const int DUTY_STEP = 10;    // Langkah naik/turun kecepatan
const int DUTY_MIN  = 0;
const int DUTY_MAX  = 255;

String inputString = "";     // Buffer untuk perintah Serial

// ============================================================
// FUNGSI INTI KONTROL MOTOR & SINKRONISASI BLYNK
// Mapping Datastreams (Blynk Virtual Pins):
// V0 = RPM (0 - 13000)
// V1 = ONOFF Switch (0 - 1)
// V2 = PWMSLIDER (0 - 255)
// V3 = PWMDISPLAY (0 - 255)
// ============================================================
void applyMotor()
{
  // Terapkan arah putaran dan kecepatan (dutyCycle) ke driver motor
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  ledcWrite(enable1Pin, dutyCycle); // Sintaks ESP32 Core 3.x

  // LED indikator: menyala jika motor bergerak (dutyCycle > 0)
  digitalWrite(ledPin, dutyCycle > 0 ? HIGH : LOW);

  // Hitung estimasi RPM (proporsional dari rentang PWM 0-255 ke RPM 0-13000)
  int estimasiRPM = map(dutyCycle, 0, 255, 0, 13000);

  // Kirim feedback/pembaruan status ke Dashboard Blynk
  Blynk.virtualWrite(V0, estimasiRPM);              
  Blynk.virtualWrite(V1, dutyCycle > 0 ? 1 : 0);    
  Blynk.virtualWrite(V2, dutyCycle);                
  Blynk.virtualWrite(V3, dutyCycle);                
}

void MotorOn()
{
  if (dutyCycle == 0) dutyCycle = 200; // Berikan torsi awal (default kecepatan)
  applyMotor();
  Serial.print("[INFO] Motor ON, duty = ");
  Serial.println(dutyCycle);
}

void MotorOff()
{
  dutyCycle = 0;
  applyMotor();
  Serial.println("[INFO] Motor OFF, duty = 0");
}

void naikkanKecepatan()
{
  dutyCycle += DUTY_STEP;
  if (dutyCycle > DUTY_MAX) dutyCycle = DUTY_MAX;
  applyMotor();
  Serial.print("[INFO] Kecepatan NAIK, duty = ");
  Serial.println(dutyCycle);
}

void turunkanKecepatan()
{
  dutyCycle -= DUTY_STEP;
  if (dutyCycle < DUTY_MIN) dutyCycle = DUTY_MIN;
  applyMotor();
  Serial.print("[INFO] Kecepatan TURUN, duty = ");
  Serial.println(dutyCycle);
}

void setDuty(int val)
{
  if (val < DUTY_MIN) val = DUTY_MIN;
  if (val > DUTY_MAX) val = DUTY_MAX;
  dutyCycle = val;
  applyMotor();
  Serial.print("[INFO] Set duty = ");
  Serial.println(dutyCycle);
}

// ============================================================
// FUNGSI CALLBACK BLYNK (Menerima perintah dari Cloud)
// ============================================================

// V1: Tombol Switch ONOFF (0 atau 1)
BLYNK_WRITE(V1)
{
  int tombolOnOff = param.asInt();
  if (tombolOnOff == 1) {
    MotorOn();
  } else {
    MotorOff();
  }
}

// V2: PWMSLIDER Kecepatan (0 - 255)
BLYNK_WRITE(V2)
{
  int nilaiSlider = param.asInt();
  setDuty(nilaiSlider);
}

// ============================================================
// SETUP & LOOP UTAMA
// ============================================================
void setup()
{
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);

  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);

  // Inisialisasi PWM (menggunakan standar ESP32 Core 3.x)
  ledcAttach(enable1Pin, freq, resolution);

  // Kondisi aman saat mikrokontroler baru dihidupkan
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  ledcWrite(enable1Pin, 0);

  Serial.println("Menghubungkan ke WiFi & Server Blynk...");
  
  // Memulai koneksi ke Blynk Server
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Sinkronisasi status perangkat keras ke Dashboard Blynk saat pertama kali terhubung
  applyMotor();

  Serial.println("=========================================");
  Serial.println(" ESP32 Motor Controller + Blynk Siap.    ");
  Serial.println("=========================================");
  Serial.println("Perintah Serial: 1=ON, 0=OFF, +=naik, -=turun, PWM:xxx=set");
}

void loop()
{
  // Wajib ada agar ESP32 terus merespons ping dan pembaruan dari server Blynk
  Blynk.run();

  // Membaca instruksi dari antarmuka Serial Monitor sebagai kendali alternatif/debugging
  while (Serial.available() > 0)
  {
    char c = Serial.read();

    if (c == '\n' || c == '\r')
    {
      inputString.trim();
      if (inputString.startsWith("PWM:"))
      {
        int val = inputString.substring(4).toInt();
        setDuty(val);
      }
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
      inputString = "";
    }
    else
    {
      inputString += c;
    }
  }
}