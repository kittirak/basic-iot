/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME ""
#define BLYNK_AUTH_TOKEN ""

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <TridentTD_LineNotify.h>

#define DHTPIN D4
#define DHTTYPE DHT11

#define LINE_TOKEN  ""

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
float TemperatureThreshold;

char ssid[] = "";
char pass[] = "";

int timezone = 7 * 3600; //ตั้งค่า TimeZone ตามเวลาประเทศไทย
int dst = 0; //กำหนดค่า Daylight saving time

void sendSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V5, h);
  Blynk.virtualWrite(V6, t);

/*
  if (t>TemperatureThreshold) {
    digitalWrite(D1, LOW); // enable pull up resistor
    Blynk.virtualWrite(V7, LOW);
  } else {
    digitalWrite(D1, HIGH); // enable pull up resistor
    Blynk.virtualWrite(V7, HIGH);
  }
*/
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V4);  
  Blynk.syncVirtual(V7);  
  Blynk.syncVirtual(V8);  
}

// V4 is Slider Temperature Threshold
BLYNK_WRITE(V4) {
  //reads the slider value when it changes in the app
  TemperatureThreshold = param.asFloat();
}

// V7 is Button On/Off
BLYNK_WRITE(V7) {
  if(param.asInt() == 1){
    digitalWrite(D1,HIGH);  // Set digital pin D1 HIGH
  } else {
    digitalWrite(D1,LOW);  // Set digital pin D1 LOW    
  }
  Serial.print("Blynk.Cloud is writing V1 is ");
  Serial.println(param.asInt());
  Serial.println();
}

// V8 Time Input 
BLYNK_WRITE(V8) {
  long startTimeInSecs = param[0].asLong();
  long stopTimeInSecs = param[1].asLong();

  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  long presentTimeInSecs = timeinfo->tm_hour*3600+(timeinfo->tm_min*60);

  Serial.println(startTimeInSecs);
  Serial.println(stopTimeInSecs);
  Serial.println(presentTimeInSecs);
  Serial.println(asctime(timeinfo));

  if((presentTimeInSecs >= startTimeInSecs) && (presentTimeInSecs < stopTimeInSecs)) {   
    digitalWrite(D1, LOW);// code here to switch the relay ON
    Blynk.virtualWrite(V7, LOW);
    LINE.notify("เปิดก๊อก");         

  } else {
    digitalWrite(D1, HIGH); // code here to switch the relay OFF 
    Blynk.virtualWrite(V7, HIGH);
  }
  Serial.println();
}

void setup() {
  // put your setup code here, to run once:
  pinMode(D1, OUTPUT); // then set pin to output
  digitalWrite(D1, HIGH); // enable pull up resistor
  Serial.begin(9600);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  dht.begin();

  // NTP Sync
  //  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov"); //ดึงเวลาจาก Server  
  configTime(timezone, dst, "192.168.123.10"); //ดึงเวลาจาก Server  

  LINE.setToken(LINE_TOKEN);
  Serial.println(LINE.getVersion());
  LINE.notify("nodeMCU is up");         
  //LINE.notifySticker("Hello",1,2);  // ส่ง Line Sticker ด้วย PackageID 1 , StickerID 2  พร้อมข้อความ

  // Setup a function to be called every second
  timer.setInterval(1000L, sendSensor);
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  timer.run();
}
