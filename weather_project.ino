#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#define bmp_adress
#define WIRE Wire
#define screen_adress 0x3C
#define conversion 0.02953
#define DHTPIN A0
#define DHTTYPE DHT11
Adafruit_BMP280 bmp;
DHT dht(DHTPIN, DHTTYPE);

float raw_x_data[128];
float raw_y_data_pressure[128];
float raw_y_data_temp[128];
float raw_y_data_hum[128];
float L = 0.0065;
float R = 8.3144598;
float M = 0.0289644;
float g = 9.8;
float elev = 64;
float T0 = 288.15;
float c = M * g / (R * L);
int datapoints = 128;
int pb = 63, pl = 0, pr = 127, pt = 0;
float data_pressure[128];
float data_temp[128];
float data_hum[128];
int cycle = 0;
Adafruit_SSD1306 oled(128, 64, &Wire, -1);
float mest =  12*60*60;
float Ymin_pressure = 28;
float Ymax_pressure = 31;
float Ymin_temp = 10;
float Ymax_temp = 50;
float Ymin_hum = 0;
float Ymax_hum = 100;
float Xmin = 0;
float Xmax = mest;
float dummy[128];

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(13, INPUT_PULLUP);
  if (!oled.begin(SSD1306_SWITCHCAPVCC, screen_adress)) Serial.println("screen error");
  if (!bmp.begin(0x76)) Serial.println("bmp error");
  oled.clearDisplay();
  oled.display();
  for (int i = 0; i < 128; i++) {
    data_pressure[i] = 0;
    raw_x_data[i] = 0;
    raw_y_data_pressure[i] = 0;
    data_temp[i] = 0;
    raw_y_data_temp[i] = 0;
    raw_y_data_hum[i] = 0;
    data_hum[i] = 0;
    raw_x_data[i] = (Xmax / 128) * i;
    dummy[i] = 0;
  }
  norm(raw_x_data, dummy, Xmin, Xmax, Ymin_pressure, Ymax_pressure);
  data_temp[0]=dht.readTemperature();
  data_pressure[0]=bmp.readPressure();
  data_hum[0]=dht.readHumidity();
}
float last_update = millis();
int t = 0;
int end = 0;
int prev = 1;
int now = 1;
void loop() {
  oled.clearDisplay();
  now = digitalRead(13);
  if (now == 0 and prev == 1) {
    cycle++;
    cycle = cycle % 3;
  }
  prev = now;
  if (millis() - last_update < 1000 * mest / 128) {
    Serial.println(end);
    data_pressure[end] = 0.05 * data_pressure[end] + conversion * 0.95 * bmp.readPressure() / (pow(1 - L * elev / T0, c) * 100);

    data_temp[end] = 0.05 * data_temp[end] + 0.95 * dht.readTemperature();

    data_hum[end] = 0.05 * data_hum[end] + 0.95 * dht.readHumidity();

  }

  else {
    for (int i = 0; i < end-1; i++) {
        
    
      data_pressure[i] = data_pressure[i + 1];
      data_temp[i] = data_temp[i + 1];
      data_hum[i] = data_hum[i + 1];
        raw_y_data_pressure[i] = data_pressure[i];
      raw_y_data_temp[i] = data_temp[i];
      raw_y_data_hum[i] = data_hum[i];
      dummy[i] = 0;
    
    }
    end = min(127, end + 1);
   

    norm(dummy, raw_y_data_pressure, Xmin, Xmax, Ymin_pressure, Ymax_pressure);
    norm(dummy, raw_y_data_temp, Xmin, Xmax, Ymin_temp, Ymax_temp);
    norm(dummy, raw_y_data_hum, Xmin, Xmax, Ymin_hum, Ymax_hum);


    if (cycle % 3 == 0) plotgr(raw_x_data, raw_y_data_pressure, data_pressure);
    else if (cycle % 3 == 1) plotgr(raw_x_data, raw_y_data_temp, data_temp);
    else if (cycle % 3 == 2) plotgr(raw_x_data, raw_y_data_hum, data_hum);

    last_update = millis();
  }
}





void norm(float raw_x[], float raw_y[], float Xmin, float Xmax, float Ymin, float Ymax) {
  float ax = (pl - pr) / (Xmin - Xmax);
  float ay = (pb - pt) / (Ymin - Ymax);
  float bx = pr - ax * Xmax;
  float by = pt - ay * Ymax;
  for (int i = 0; i < 127; i++) {

    raw_x[i] = ax * raw_x[i] + bx;
    raw_y[i] = ay * raw_y[i] + by;
  }
}

void plotgr(float raw_x_data[], float raw_y_data[], float data[]) {

  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.drawLine(pl, pb, pr, pb, WHITE);
  oled.drawLine(pl, pb, pl, pt, WHITE);

  for (int i = 0; i < end - 2; i++) {
    oled.drawLine(int(raw_x_data[i]), int(raw_y_data[i]), int(raw_x_data[i+1]), int(raw_y_data[i+1]), WHITE);
    Serial.println(String(raw_y_data[i+1]) + " " +String(raw_x_data[i+1]));
   
  }
  oled.setCursor(20, 32);
  oled.setTextSize(2);
  oled.setTextColor(WHITE);
  oled.print(data[end - 1]);
  oled.setTextSize(1);

  if (cycle == 0) {
    oled.print(" in HG");
    oled.setCursor(2, 0);
    oled.print(String(Ymax_pressure) + " in Hg");
    oled.setCursor(2, 55);
    oled.print(String(Ymin_pressure) + " in Hg");
  }
  if (cycle == 1) {
    oled.print(" C");
    oled.setCursor(2, 0);
    oled.print(String(Ymax_temp) + " C");
    oled.setCursor(2, 55);
    oled.print(String(Ymin_temp) + " C");
  }
  if (cycle == 2) {
    oled.print("%");
    oled.setCursor(2, 0);
    oled.print(String(Ymax_hum) + " %humidity");
    oled.setCursor(2, 55);
    oled.print(String(Ymin_hum) + " %");
  }
  oled.display();
}