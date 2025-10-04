# ❤️ Smart SpO₂ & Heart Rate Monitoring System (ESP8266 + MAX30102 + MLX90614 + Ubidots IoT)

Proyek ini adalah sebuah sistem pemantauan kesehatan berbasis Internet of Things (IoT) yang menggunakan ESP8266 untuk menghubungkan berbagai sensor kesehatan ke internet. Sistem ini mampu mengukur beberapa parameter kesehatan seperti detak jantung (BPM), Saturasi Oksigen dalam Darah (SpO2), dan suhu tubuh. Data yang dikumpulkan kemudian dikirimkan ke platform cloud Ubidots untuk pemantauan jarak jauh, memungkinkan pengguna atau profesional medis untuk memantau kondisi kesehatan secara real-time.

---

## 📚 Referensi Utama
Proyek ini dikembangkan dengan mengadaptasi logika pembacaan PPG dari repositori berikut:  
🔗 [tinyPulsePPG by jeffmer](https://github.com/jeffmer/tinyPulsePPG)

---

## ⚙️ Komponen Utama
| Komponen | Fungsi | Pin ESP8266 |
|-----------|---------|-------------|
| MAX30102 | Sensor detak jantung & oksigen darah (IR & RED LED) | SDA (D2), SCL (D1) |
| MLX90614 | Sensor suhu inframerah tanpa sentuh | SDA (D2), SCL (D1) |
| OLED SSD1306 (0.96") | Tampilan nilai BPM, SpO₂, dan suhu | SDA (D2), SCL (D1) |
| LED (D4) | Indikator detak jantung | D4 |
| Push Button | Fungsi manual opsional | D3 |
| ESP8266 (NodeMCU) | Mikrokontroler utama | - |

---

## 📡 Koneksi Internet (Ubidots IoT)
Sistem menggunakan **Ubidots MQTT** untuk mengirim data secara real-time ke cloud.
```cpp
Data yang dikirim ke Ubidots:
bpm_alat3 → nilai detak jantung (BPM)
spo2_alat3 → kadar oksigen darah (SpO₂ %)
suhu_alat3 → suhu tubuh (°C)
```

---

## 🧩 Library yang Digunakan
- UbidotsESPMQTT
- ESP8266WiFi
- Adafruit_GFX
- Adafruit_SSD1306
- Adafruit_MLX90614
- Wire
- MAX30102
- Pulse

---

## 🧠 Fitur Utama
- 💓 Pembacaan detak jantung (BPM) dengan sensor MAX30102 berbasis inframerah
- 🩸 Deteksi kadar oksigen darah (SpO₂) secara non-invasif
- 🌡️ Pengukuran suhu tubuh menggunakan MLX90614 tanpa sentuhan
- 🖥️ Tampilan real-time pada OLED Display (128x64)
- ☁️ Kirim data otomatis ke Ubidots IoT Platform via MQTT
- ⚙️ PID control & kalibrasi suhu untuk akurasi pengukuran
- 🔄 Reconnect WiFi otomatis jika koneksi terputus

---

## 🩺 Tampilan OLED Display
Tampilan layar menampilkan tiga data utama secara real-time:
| Baris | Informasi            | Contoh         |
| ----- | -------------------- | -------------- |
| 1–2   | BPM (Detak Jantung)  | `BPM: 78`      |
| 3–4   | SpO₂ (Oksigen Darah) | `SpO2: 97%`    |
| 5–6   | Suhu Tubuh           | `Suhu: 36.7°C` |

Ketika jari belum ditempel ke sensor, layar akan menampilkan:
```text
Letakkan
Jari Anda
Pada
Sensor!
```

---

## Contacs us : 
* [Frendi RoboTech](https://www.instagram.com/frendi.co/)
* [Whatsapp : +6287888227410](https://wa.me/+6287888227410)
* [Email    : frendirobotech@gmail.com](https://mail.google.com/mail/u/0/?view=cm&tf=1&fs=1&to=frendirobotech@gmail.com) atau [Email    : frendix45@gmail.com](https://mail.google.com/mail/u/0/?view=cm&tf=1&fs=1&to=frendix45@gmail.com)

---

## 👨‍💻 Author
Dikembangkan oleh: Imam Sa'id Nurfrendi [Reog Robotic & Robotech Electronics]  
Lisensi: Open Source (MIT)
