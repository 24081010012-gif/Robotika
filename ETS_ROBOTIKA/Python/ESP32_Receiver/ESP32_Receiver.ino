// KODE ESP32 UNTUK MENERIMA PERINTAH DARI PYTHON GUI
int motor1Pin1 = 27;
int motor1Pin2 = 26;
int enable1Pin = 12;

const int freq = 30000;
const int resolution = 8;

void setup() {
  Serial.begin(115200);
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  ledcAttach(enable1Pin, freq, resolution);
  
  // Pastikan motor mati saat awal menyala
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  ledcWrite(enable1Pin, 0);
  
  Serial.println("Sistem Siap! Menunggu perintah dari Python GUI...");
}

void loop() {
  // Mengecek apakah ada data yang masuk dari kabel USB (Python)
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim(); // Menghapus spasi atau karakter enter berlebih

    if (command == "1") {
      digitalWrite(motor1Pin1, HIGH);
      digitalWrite(motor1Pin2, LOW);
      ledcWrite(enable1Pin, 200); // Kecepatan default saat ON
      Serial.println(">>> Motor Dinyalakan");
      
    } else if (command == "0") {
      digitalWrite(motor1Pin1, LOW);
      digitalWrite(motor1Pin2, LOW);
      ledcWrite(enable1Pin, 0);
      Serial.println(">>> Motor Dimatikan");
      
    } else if (command.startsWith("PWM:")) {
      String pwmValueStr = command.substring(4); // Mengambil angka setelah teks "PWM:"
      int pwmValue = pwmValueStr.toInt();
      ledcWrite(enable1Pin, pwmValue);
      Serial.print(">>> Kecepatan (PWM) diubah menjadi: ");
      Serial.println(pwmValue);
    }
  }
}



