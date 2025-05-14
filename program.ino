// Referensi Program SPO2 dan Heart Rate https://github.com/jeffmer/tinyPulsePPG


/****************************************
   Include Libraries
 ****************************************/
#include "UbidotsESPMQTT.h"
#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>  //OLED libraries
#include <Adafruit_SSD1306.h>
#include <Adafruit_MLX90614.h>
#include <Wire.h>
#include "MAX30102.h"
#include "Pulse.h"


/****************************************
   Define Constants
 ****************************************/
#define TOKEN "BBFF-FxHA0aWRA6zowCXaEQe5EQnqnrdY4k"  // Your Ubidots TOKEN
#define WIFINAME "PUSPITA"                          // Your SSID
#define WIFIPASS "87654321"                          // Your Wifi Pass

Ubidots client(TOKEN);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
MAX30102 sensor;
Pulse pulseIR;
Pulse pulseRed;
MAFilter bpm;


#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  //Declaring the display name (display)


static const unsigned char PROGMEM logo2_bmp[] = { 0x03, 0xC0, 0xF0, 0x06, 0x71, 0x8C, 0x0C, 0x1B, 0x06, 0x18, 0x0E, 0x02, 0x10, 0x0C, 0x03, 0x10,
                                                   0x04, 0x01, 0x10, 0x04, 0x01, 0x10, 0x40, 0x01, 0x10, 0x40, 0x01, 0x10, 0xC0, 0x03, 0x08, 0x88,
                                                   0x02, 0x08, 0xB8, 0x04, 0xFF, 0x37, 0x08, 0x01, 0x30, 0x18, 0x01, 0x90, 0x30, 0x00, 0xC0, 0x60,
                                                   0x00, 0x60, 0xC0, 0x00, 0x31, 0x80, 0x00, 0x1B, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x04, 0x00, 0x00
                                                 };

static const unsigned char PROGMEM logo3_bmp[] = { 0x01, 0xF0, 0x0F, 0x80, 0x06, 0x1C, 0x38, 0x60, 0x18, 0x06, 0x60, 0x18, 0x10, 0x01, 0x80, 0x08,
                                                   0x20, 0x01, 0x80, 0x04, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x02, 0xC0, 0x00, 0x08, 0x03,
                                                   0x80, 0x00, 0x08, 0x01, 0x80, 0x00, 0x18, 0x01, 0x80, 0x00, 0x1C, 0x01, 0x80, 0x00, 0x14, 0x00,
                                                   0x80, 0x00, 0x14, 0x00, 0x80, 0x00, 0x14, 0x00, 0x40, 0x10, 0x12, 0x00, 0x40, 0x10, 0x12, 0x00,
                                                   0x7E, 0x1F, 0x23, 0xFE, 0x03, 0x31, 0xA0, 0x04, 0x01, 0xA0, 0xA0, 0x0C, 0x00, 0xA0, 0xA0, 0x08,
                                                   0x00, 0x60, 0xE0, 0x10, 0x00, 0x20, 0x60, 0x20, 0x06, 0x00, 0x40, 0x60, 0x03, 0x00, 0x40, 0xC0,
                                                   0x01, 0x80, 0x01, 0x80, 0x00, 0xC0, 0x03, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00, 0x30, 0x0C, 0x00,
                                                   0x00, 0x08, 0x10, 0x00, 0x00, 0x06, 0x60, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x01, 0x80, 0x00
                                                 };

/****************************************
   Auxiliar variables
 ****************************************/
#define LED D4
#define Push_Button D3

int beatAvg,
    counter = 0;
int SPO2,
    SPO2f;
float temperature,
      suhu,
      suhu_rata,
      suhu_sebelum,
      suhu_sekarang,
      i,
      j;
bool led_on = false;
long lastBeat = 0,    //Time of the last beat
     displaytime = 0,  //Time of the last display update
     uploadtime = 0;  //Time of the last update to send ke ubidots

unsigned long last_millis = 0;

bool ubidots_connected = false;



//int ThermistorPin = A7;
int Vo;
float R1 = 10000;
float logR2, R2, T, Tc, Tf;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

float Kp = 8;
float Ki = 0.1;
float Kd = 0;
float Error, dError, sum_Error;
float sp = 36.5;


/****************************************
   Auxiliar Functions
 ****************************************/


void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

float suhu_C() {
  Vo = mlx.readObjectTempC();
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
  Tc = (T - 273.15) * -1;
  Tf = (Tc * 9.0) / 5.0 + 32.0;

  return Tc;
  delay(100);
  
}

int pid (float p, float i, float d)
{
  dError = Error;
  Error = sp - suhu_C();
  sum_Error += Error;

  float P = p * Error;
  float I = i * sum_Error;
  float D = (d / 100) * (Error - dError);

  float power = P + I + D;

  if (power > 255) {
    power = 255;
  }
  else if (power < 0) {
    power = 0;
  }
  return power;
}

//void Tempetaure_MLX_Read()
//{
//  i = 1156 / mlx.readObjectTempC();   //Kalibrasi Sensor MLX
//  suhu = mlx.readObjectTempC();
//  //suhu = mlx.readObjectTempC();
//
//  if (counter <= 10) {
//    j = j + i;
//    suhu_rata = suhu_rata + suhu;
//
//    if (counter == 10) {
//      //   suhu_rata = (suhu_rata / 10) - (j / 10);
//      suhu_rata = (suhu_rata / 10);
//      suhu_sekarang = suhu_rata;
//    }
//  }
//  if (suhu_sebelum >= (suhu_sekarang + 1.2) && suhu_sebelum <= (suhu_sekarang + 1.2)) {
//    suhu_sekarang = suhu_sebelum + 0.1;
//  }
//}


/****************************************
   Main Functions
 ****************************************/
void setup(void)
{
  Serial.begin(115200);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW); // led nyala


  Serial.println("Initializing...MAX30102");
  Wire.begin();
  Wire.setClock(400000); // atur kecepatan komunikasi data I2C ke 400kHz
  delay(500);

  mlx.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  //Start the OLED display

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Koneksi");
  display.println("Ke Wifi");
  display.println(WIFINAME);
  display.display();

  // inisialisasi ubidots
  client.setDebug(true);  // Pass a true or false bool value to activate debug messages
  client.wifiConnection(WIFINAME, WIFIPASS);
  client.begin(callback);

  display.clearDisplay();


  if (!sensor.begin())  // jika inisialisasi sensor error
  {
    Serial.println("MAX30102 was not found. Please check wiring and reconect power. ");
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("MAX30102");
    display.setCursor(0, 16);
    display.println("Error!");
    display.display();

    while (1) {
      delay(10);
    }
  }

  sensor.setup();

  digitalWrite(LED, HIGH); // led mati
}


void loop()
{
  // put your main code here, to run repeatedly:
  if (!client.connected()) // jika koneksi putus maka conect ulang
  {
    client.reconnect(); // coba konek ulang ke server ubidots
  }

  long now = millis();  //start time of this cycle

  sensor.check();
  if (!sensor.available()) return;  //Check the sensor for new data // kalo belum ada data maka lakukan cek sensor lagi

  const float temp_calib = 0.5   ; // konstanta kalibrasi suhu
  uint32_t redValue = sensor.getRed();
  uint32_t irValue = sensor.getIR();
  sensor.nextSample();

  Serial.print(" irValue=");
  Serial.println(irValue);

  if (irValue < 50000)
  {
    suhu_sebelum = suhu_sekarang;
    beatAvg = 0;
    SPO2 = 0;
    SPO2f = 0;
    counter = 0;
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10, 0);
    display.println("Letakkan");
    display.setCursor(10, 16);
    display.println("Jari Anda");
    display.setCursor(10, 32);
    display.println("Pada");
    display.setCursor(10, 48);
    display.println("Sensor!");
    display.display();
  }
  else if (irValue >= 50000)  // ketika jari telah ditempel ke sensor nilai irValue diatas 5000
  {
    // remove DC element
    int16_t IR_signal, Red_signal;
    bool beatRed, beatIR;

    IR_signal = pulseIR.ma_filter(pulseIR.dc_filter(irValue));
    Red_signal = pulseRed.ma_filter(pulseRed.dc_filter(redValue));
    beatRed = pulseRed.isBeat(Red_signal);
    beatIR = pulseIR.isBeat(IR_signal);


    //    Serial.print(" IR_signal=");
    //    Serial.print(IR_signal);
    //    Serial.print(" Red_signal=");
    //    Serial.print(Red_signal);
    //    Serial.print(" beatRed=");
    //    Serial.print(beatRed);
    //    Serial.print(" beatIR=");
    //    Serial.print(beatIR);
    //    Serial.println();

    display.clearDisplay();                              //Clear the display
    display.drawBitmap(5, 5, logo2_bmp, 24, 21, WHITE);  //Draw the first bmp picture (little heart)
    display.setTextSize(2);                              //Near it display the average BPM you can display the BPM if you want
    // Tampilkan nilai BPM
    display.setTextColor(WHITE);
    display.setCursor(47, 0);
    display.print("BPM");
    display.setCursor(47, 16);
    display.print(beatAvg);
    // Tampilkan Nilai SpO2
    display.setCursor(0, 32);
    display.println("SpO2");
    display.setCursor(0, 48);
    display.print(SPO2f);
    display.print("%");
    // Tampilkan Nilai SpO2
    if (counter == 10) {
      display.setCursor(65, 32);
      display.println("Suhu");
      display.setCursor(65, 48);
      display.print((suhu_C() + Error) - (mlx.readObjectTempC() / 6.6));
      display.print("*C");
    } else {
      display.setCursor(65, 32);
      display.println("Suhu");
      display.setCursor(65, 48);
      display.print((suhu_C() + Error) - (mlx.readObjectTempC() / 6.6));
      display.print("*C");
    }
    // display.print("*C");
    display.display();


    if (beatIR == HIGH)  //If a heart beat is detected
    {
      if (counter <= 10) {
        // compute heart rate (beat per minute atau BPM)
        long btpm = 60000 / (now - lastBeat);
        if (btpm > 60  && btpm < 200) beatAvg = bpm.filter((int16_t)btpm);
        lastBeat = now;
        digitalWrite(LED, LOW); // LED ON
        led_on = true;

        // compute SpO2 ratio
        long numerator = (pulseRed.avgAC() * pulseIR.avgDC()) / 256;
        long denominator = (pulseRed.avgDC() * pulseIR.avgAC()) / 256;
        int RX100 = (denominator > 0) ? (numerator * 100) / denominator : 999;

        // using formula
        SPO2f = (10400 - RX100 * 17 + 50) / 102;  // nilai SpO2 berdasarkan perhitungan
        if (SPO2f > 100) SPO2f = 100;

        // from table
        if ((RX100 >= 0) && (RX100 < 184))
          SPO2 = pgm_read_byte_near(&spo2_table[RX100]);  // Nilai SpO2 Rata-rata berdasarkan menggunakan look up tabel
        // Serial.print(" btpm=");
        // Serial.print(btpm);

        // UBAH SUHU --------------------------------------------------------------------------------------------------------------------------------------------------//
        temperature = sensor.readTemperature();
        //        Tempetaure_MLX_Read();

        display.clearDisplay();                              //Clear the display
        display.drawBitmap(0, 0, logo3_bmp, 32, 32, WHITE);  //Draw the second picture (bigger heart)
        display.setTextSize(2);                              //And still displays the average BPM
        display.setTextColor(WHITE);
        // Tampilkan nilai BPM
        display.setCursor(47, 0);
        display.print("BPM");
        display.setCursor(47, 16);
        display.print(beatAvg);
        // Tampilkan Nilai SpO2
        display.setCursor(0, 32);
        display.println("SpO2");
        display.setCursor(0, 48);
        display.print(SPO2f);
        display.print("%");
        // Tampilkan Nilai Suhu

        if (counter == 10) {
          display.setCursor(65, 32);
          display.println("Suhu");
          display.setCursor(65, 48);
          display.print((suhu_C() + Error) - (mlx.readObjectTempC() / 6.6));
          display.print("*C");
        } else {
          display.setCursor(65, 32);
          display.println("Suhu");
          display.setCursor(65, 48);
          display.print((suhu_C() + Error) - (mlx.readObjectTempC() / 6.6));
          display.print("*C");
        }
        // Tampilkan display
        display.display();

        counter = counter + 1;
        Serial.println(counter);

        // update display every 1000 ms if fingerdown
        if (now - displaytime > 1000)
        {
          displaytime = now;
          Serial.print(" temperature=");
          Serial.print(temperature, 4);
          Serial.print(" HR AVG=");
          Serial.print(beatAvg);
          Serial.print("\t SPO2f=");
          Serial.print(SPO2f);
          Serial.print("\t SPO2 AVG=");
          Serial.print(SPO2);
          Serial.println();
        }

        if ( now - uploadtime > 1000 && client.connected() == true)
        {
          uploadtime = now;
          client.add("bpm_alat3", beatAvg);
          client.add("spo2_alat3", SPO2f);
          client.add("suhu_alat3", suhu);
          client.ubidotsPublish("alat3");
          client.loop();
        }
      }
    }
  }

  // flash led for 25 ms
  if (led_on && (now - lastBeat) > 25)
  {
    digitalWrite(LED, HIGH); // LED OFF
    led_on = false;
  }
}
