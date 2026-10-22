//PROGRESS 1 KONTROL MOTOR DASAR / CONNECT WITH WIRE

// ============================================================
// Kontrol Motor DC + LED dengan PWM via Serial
// Perintah:
//   '1'        -> Motor ON (duty terakhir / default)
//   '0'        -> Motor OFF
//   '+'        -> Naikkan kecepatan (+10)
//   '-'        -> Turunkan kecepatan (-10)
//   'PWM:xxx'  -> Set duty cycle langsung (0-255)
// ============================================================


// --- Konfigurasi Pin Hardware ---
const int ledPin = 2;       // Pin untuk LED indikator (menggunakan LED built-in ESP32)
const int motor1Pin1 = 27;  // Pin IN1 pada driver motor (mengatur arah putaran)
const int motor1Pin2 = 26;  // Pin IN2 pada driver motor (mengatur arah putaran)
const int enable1Pin = 12;  // Pin ENA pada driver motor (mengatur kecepatan via PWM)

// --- Konfigurasi PWM ESP32 (ledc) ---
const int freq = 30000;       // Frekuensi PWM (30 kHz cocok untuk motor DC)
const int pwmChannel = 0;     // Channel PWM ESP32 (0-15)
const int resolution = 8;     // Resolusi 8-bit (nilai 0 - 255)

// --- Variabel Kontrol Kecepatan ---
int dutyCycle = 200;          // Nilai kecepatan awal (default) saat motor dinyalakan
const int DUTY_STEP = 10;     // Besar penambahan/pengurangan kecepatan tiap perintah '+' atau '-'
const int DUTY_MIN  = 0;      // Batas minimum kecepatan (Motor mati)
const int DUTY_MAX  = 255;    // Batas maksimum kecepatan (Kecepatan penuh)

// --- Variabel Buffer Serial ---
String inputString = "";      // Menyimpan string karakter yang diterima dari Serial Monitor

void setup() {
  // Inisialisasi pin sebagai OUTPUT
  pinMode(ledPin, OUTPUT);
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);

  // Inisialisasi komunikasi Serial pada baudrate 115200
  Serial.begin(115200);

  // Konfigurasi modul hardware PWM pada ESP32
  ledcSetup(pwmChannel, freq, resolution);
  
  // Hubungkan channel PWM ke pin Enable motor
  ledcAttachPin(enable1Pin, pwmChannel);

  // Set kondisi awal: Motor berhenti (Brake/Coast)
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW); // Kombinasi HIGH-LOW membuat motor berputar searah jarum jam
  ledcWrite(pwmChannel, 0);      // Kecepatan 0 (mati)

  // Pesan sambutan di Serial Monitor
  Serial.println("=========================================");
  Serial.println(" ESP32 Motor Controller Siap Digunakan!  ");
  Serial.println("=========================================");
  Serial.println("Perintah: 1=ON, 0=OFF, +=Naik, -=Turun, PWM:xxx=Set Manual");
}

/*
 * Fungsi untuk menerapkan nilai dutyCycle saat ini ke hardware.
 * Dipanggil setiap kali ada perubahan kecepatan.
 */
void applyMotor() {
  // Pastikan arah putaran motor konsisten
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  
  // Terapkan sinyal PWM ke driver motor
  ledcWrite(pwmChannel, dutyCycle);

  // Nyalakan LED jika motor bergerak (dutyCycle > 0), matikan jika berhenti
  digitalWrite(ledPin, dutyCycle > 0 ? HIGH : LOW);
}

void MotorOn() {
  // Jika kecepatan sebelumnya 0, kembalikan ke default (200) agar motor bisa berputar
  if (dutyCycle == 0) dutyCycle = 200;
  applyMotor();
  Serial.print("[INFO] Motor ON, duty cycle = ");
  Serial.println(dutyCycle);
}

void MotorOff() {
  dutyCycle = 0;
  applyMotor();
  Serial.println("[INFO] Motor OFF, duty cycle = 0");
}

void naikkanKecepatan() {
  dutyCycle += DUTY_STEP;
  if (dutyCycle > DUTY_MAX) dutyCycle = DUTY_MAX; // Mencegah nilai melebihi 255
  applyMotor();
  Serial.print("[INFO] Kecepatan NAIK, duty cycle = ");
  Serial.println(dutyCycle);
}

void turunkanKecepatan() {
  dutyCycle -= DUTY_STEP;
  if (dutyCycle < DUTY_MIN) dutyCycle = DUTY_MIN; // Mencegah nilai negatif
  applyMotor();
  Serial.print("[INFO] Kecepatan TURUN, duty cycle = ");
  Serial.println(dutyCycle);
}

/*
 * Fungsi untuk memproses perintah set manual (PWM:xxx)
 */
void setDuty(int val) {
  // Validasi nilai input agar tetap berada di rentang 0-255
  if (val < DUTY_MIN) val = DUTY_MIN;
  if (val > DUTY_MAX) val = DUTY_MAX;
  
  dutyCycle = val;
  applyMotor();
  Serial.print("[INFO] Set duty cycle manual = ");
  Serial.println(dutyCycle);
}

void loop() {
  // Selama ada data yang masuk dari Serial Monitor
  while (Serial.available() > 0) {
    char c = Serial.read();

    // Jika mendeteksi karakter Enter/Newline (Tanda perintah selesai diketik)
    if (c == '\n' || c == '\r') {
      inputString.trim(); // Hapus spasi ekstra di awal dan akhir string
      
      // 1. Cek apakah perintah adalah format "PWM:xxx"
      if (inputString.startsWith("PWM:")) {
        int val = inputString.substring(4).toInt(); // Ambil angka setelah "PWM:"
        setDuty(val);
      } 
      // 2. Cek apakah ada perintah satu karakter
      else if (inputString.length() > 0) {
        char cmd = inputString.charAt(0);
        
        if (cmd == '1') {
          MotorOn();
        } 
        else if (cmd == '0') {
          MotorOff();
        } 
        else if (cmd == '+') {
          naikkanKecepatan();
        } 
        else if (cmd == '-') {
          turunkanKecepatan();
        } 
        else {
          Serial.print("[ERROR] Perintah tidak dikenal: ");
          Serial.println(inputString);
        }
      }
      
      // Kosongkan buffer setelah perintah dieksekusi agar siap untuk perintah selanjutnya
      inputString = "";
    } 
    else {
      // Tambahkan karakter yang masuk ke dalam buffer
      inputString += c;
    }
  }
}