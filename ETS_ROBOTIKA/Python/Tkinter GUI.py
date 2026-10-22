import tkinter as tk
from tkinter import ttk, messagebox
import serial
import serial.tools.list_ports

class MotorControllerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Python ESP32 Motor Controller")
        self.root.geometry("400x380")
        self.root.resizable(False, False)
        
        self.serial_port = None
        
        self.create_widgets()
        self.refresh_ports()
        
    def create_widgets(self):
        # Frame Koneksi Serial
        conn_frame = ttk.LabelFrame(self.root, text="Pengaturan Koneksi Serial")
        conn_frame.pack(pady=10, padx=10, fill="x")
        
        self.port_var = tk.StringVar()
        self.port_cb = ttk.Combobox(conn_frame, textvariable=self.port_var, state="readonly", width=15)
        self.port_cb.grid(row=0, column=0, padx=10, pady=10)
        
        btn_refresh = ttk.Button(conn_frame, text="Refresh Port", command=self.refresh_ports)
        btn_refresh.grid(row=0, column=1, padx=5, pady=10)
        
        self.btn_connect = ttk.Button(conn_frame, text="Connect", command=self.toggle_connection)
        self.btn_connect.grid(row=0, column=2, padx=5, pady=10)
        
        # Frame Kontrol Motor Dasar (ON/OFF)
        ctrl_frame = ttk.LabelFrame(self.root, text="Kontrol Motor Dasar")
        ctrl_frame.pack(pady=10, padx=10, fill="x")
        
        btn_on = tk.Button(ctrl_frame, text="MOTOR ON", bg="#4CAF50", fg="white", font=("Arial", 10, "bold"), command=lambda: self.send_command("1"))
        btn_on.pack(side="left", expand=True, fill="x", padx=10, pady=15)
        
        btn_off = tk.Button(ctrl_frame, text="MOTOR OFF", bg="#F44336", fg="white", font=("Arial", 10, "bold"), command=lambda: self.send_command("0"))
        btn_off.pack(side="right", expand=True, fill="x", padx=10, pady=15)
        
        # Frame Kecepatan (PWM)
        pwm_frame = ttk.LabelFrame(self.root, text="Kontrol Kecepatan (PWM)")
        pwm_frame.pack(pady=10, padx=10, fill="x")
        
        self.pwm_slider = tk.Scale(pwm_frame, from_=0, to=255, orient="horizontal", tickinterval=51)
        self.pwm_slider.set(150) # Set nilai awal ke 150
        self.pwm_slider.pack(fill="x", padx=10, pady=5)
        
        btn_set_pwm = ttk.Button(pwm_frame, text="Set Kecepatan", command=self.set_pwm)
        btn_set_pwm.pack(pady=10)
        
        # Status Label di bagian bawah
        self.status_label = ttk.Label(self.root, text="Status: Disconnected", foreground="red", font=("Arial", 10, "bold"))
        self.status_label.pack(side="bottom", pady=10)

    def refresh_ports(self):
        """Mencari semua Port COM (Kabel USB) yang tersedia di laptop"""
        ports = serial.tools.list_ports.comports()
        self.port_cb['values'] = [port.device for port in ports]
        if self.port_cb['values']:
            self.port_cb.current(0)
            
    def toggle_connection(self):
        """Menghubungkan atau memutus koneksi Serial dengan ESP32"""
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
            self.btn_connect.config(text="Connect")
            self.status_label.config(text="Status: Disconnected", foreground="red")
            self.port_cb.config(state="readonly")
        else:
            port = self.port_var.get()
            if port:
                try:
                    # Baudrate 115200 harus sama dengan Serial.begin di ESP32
                    self.serial_port = serial.Serial(port, 115200, timeout=1) 
                    self.btn_connect.config(text="Disconnect")
                    self.status_label.config(text=f"Status: Connected to {port}", foreground="green")
                    self.port_cb.config(state="disabled")
                except Exception as e:
                    messagebox.showerror("Error", f"Gagal terhubung ke {port}: {str(e)}\n\nPastikan Serial Monitor di Arduino IDE sudah ditutup!")
            else:
                messagebox.showwarning("Peringatan", "Pilih port COM terlebih dahulu!")
                
    def send_command(self, cmd):
        """Mengirimkan teks perintah ke ESP32"""
        if self.serial_port and self.serial_port.is_open:
            try:
                # Mengirim data dengan tambahan newline (\n) agar bisa dibaca oleh Serial.readStringUntil
                self.serial_port.write((cmd + '\n').encode('utf-8'))
            except Exception as e:
                messagebox.showerror("Error", f"Gagal mengirim data: {str(e)}")
        else:
            messagebox.showwarning("Peringatan", "Silakan klik tombol 'Connect' terlebih dahulu!")
            
    def set_pwm(self):
        """Mengambil nilai slider dan mengirimkannya ke ESP32 dengan format PWM:xxx"""
        val = self.pwm_slider.get()
        self.send_command(f"PWM:{val}")

if __name__ == "__main__":
    root = tk.Tk()
    app = MotorControllerApp(root)
    root.mainloop()