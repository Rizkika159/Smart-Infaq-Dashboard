#include <WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>

// Konfigurasi WiFi Wokwi
const char* ssid = "Wokwi-GUEST";
const char* password = "";
// Menggunakan Broker Publik Gratis (Broker HiveMQ)
const char* mqtt_server = "broker.hivemq.com"; 

WiFiClient espClient;
PubSubClient client(espClient);
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int trigPin = 5;
const int echoPin = 18;
const int buzzerPin = 19;
const int tinggiKotak = 20; // Asumsi tinggi kotak amal 20cm

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  lcd.init();
  lcd.backlight();
  
  // Koneksi WiFi
  lcd.setCursor(0, 0);
  lcd.print("Menghubungkan...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  lcd.clear();
  lcd.print("WiFi Terkoneksi!");
  
  client.setServer(mqtt_server, 1883);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Mencoba MQTT...");
    // ID Client unik untuk lomba
    if (client.connect("IoT_Ramadhan_Charity_001")) {
      Serial.println("Terhubung!");
    } else {
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // Baca Sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  int jarak = duration * 0.034 / 2;
  
  // Hitung Persentase (map: jika jarak 20cm = 0%, jika jarak 2cm = 100%)
  int isiPersen = map(jarak, tinggiKotak, 2, 0, 100);
  isiPersen = constrain(isiPersen, 0, 100);

  // Tampilan LCD
  lcd.setCursor(0, 0);
  lcd.print("SMART-INFAQ");
  lcd.setCursor(0, 1);
  lcd.print("Isi: " + String(isiPersen) + "%   ");

  // Publish ke MQTT
  String payload = String(isiPersen);
  client.publish("masjid/charity/kapasitas", payload.c_str());

  // Logika Buzzer (Alarm jika penuh)
  if (isiPersen > 90) {
    tone(buzzerPin, 1000, 200);
  }

  delay(2000); // Kirim data tiap 2 detik
}