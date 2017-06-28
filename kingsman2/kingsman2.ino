#include <SparkFunESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>
 
const char SERVER[] = "api.openweathermap.org";
const char OWM_API_KEY[] = "1e7af0f682b81cf5d2ae1522e726c347";
const char WIFI_SSID[] = "cha"; // "olleh_WiFi_A38D";
const char WIFI_PASS[] = "cksgml12"; // "0000000383";
const int TIME_WAITING = 5000;
const int RECV_MAX = 32;
const char LAT[] = "37.55825";//"37.532600";// "37.566535";
const char LON[] = "127.000194";//"127.024612";//"126.97796919999996";

ESP8266Client client;
bool g_is_reqcved = false;
int buttonApin = 11; //버튼 11번핀
bool isButton = false; //버튼이 눌렸는지를 확인하기 위한 bool변수
int trig = 12; // 초음파센서의 송신부 8번핀
int echo = 13; // 초음파센서의 수신부 9번핀
int speakerpin = 3; //스피커가 연결된 디지털핀 설정

 Adafruit_NeoPixel strip = Adafruit_NeoPixel(9, 5, NEO_GRB + NEO_KHZ800);

int WeatherFrame();
void HTTPRequest();
void colorWipe(uint32_t c, uint8_t wait);
void ledOn();

void setup()
{
  
  Serial.begin(9600);
//  Serial.println(F("start!"));
  pinMode (speakerpin, OUTPUT);   
  pinMode(trig, OUTPUT); // 초음파센서의 송신부로 연결된 핀을 OUTPUT으로 설정
  pinMode(echo, INPUT); // 초음파센서의 수신부로 연결된 핀을 INPUT으로 설정
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  pinMode(buttonApin, INPUT_PULLUP);

tone(speakerpin, 1568, 500);   
  if (esp8266.begin())
  {
//    Serial.println(F("ESP8266 ready to go!"));
  }
  else
  {
//    Serial.println(F("Unable to communicate with the ESP8266 :("));
  }

  if (esp8266.connect(WIFI_SSID, WIFI_PASS) < 0)
  {
//    Serial.print(F("Error connecting: "));
  }
}

void loop()
{  
  if (digitalRead(buttonApin) == LOW)
  {
    Serial.println("ButtonOn");
    //버튼 눌리면 초음파센서ㄱㄱ
    isButton = true;
  }

  //초음파센서 실행
   while(isButton){      
    if (digitalRead(buttonApin) == HIGH){
        Serial.println("ButtonOFF");
        g_is_reqcved = false;
        isButton = false;
        colorWipe(strip.Color(0, 0, 0), 100); // Re
    }
      digitalWrite(trig, LOW);
      digitalWrite(echo, LOW);
      delayMicroseconds(2);
      digitalWrite(trig, HIGH);
      delayMicroseconds(10);
      digitalWrite(trig, LOW);
      unsigned long duration = pulseIn(echo, HIGH);
      //초음파 이동거리(cm)
      float distance = duration / 29.0 / 2.0;
      //거리가 70cm이하면 작동
      if(distance < 70)
      {
        switch(WeatherFrame())
        {
          case 1: // rain
          while(isButton){
            colorWipe(strip.Color(0, 0, 255), 600);
            colorRain(strip.Color(0, 0, 255), 800);
            if (digitalRead(buttonApin) == HIGH){
            Serial.println("ButtonOFF");
             g_is_reqcved = false;
            isButton = false;
            colorWipe(strip.Color(0, 0, 0), 100); // Re
    }}
          break;
          case 2: // clouds
           while(isButton){
            colorWipe(strip.Color(160, 43, 174), 400);
            colorRain(strip.Color(0, 0, 255), 50);
            colorWipe(strip.Color(160, 43, 174), 400);
            if (digitalRead(buttonApin) == HIGH){
            Serial.println("ButtonOFF");
             g_is_reqcved = false;
            isButton = false;
            colorWipe(strip.Color(0, 0, 0), 100); // Re
    }}
          break;
          case 3: // wind
          while(isButton){
            colorWipe(strip.Color(0, 255, 0), 50);
            colorWind(strip.Color(0, 255, 0), 30);
            if (digitalRead(buttonApin) == HIGH){
            Serial.println("ButtonOFF");
             g_is_reqcved = false;
            isButton = false;
            colorWipe(strip.Color(0, 0, 0), 100); // Re
    }}
          break;                
          case 4: // clear
          while(isButton){
            colorWipe(strip.Color(255, 255, 255), 100);
            if (digitalRead(buttonApin) == HIGH){
            Serial.println("ButtonOFF");
             g_is_reqcved = false;
            isButton = false;
            colorWipe(strip.Color(0, 0, 0), 100); // Re
    }}
          break;
          case 5:
            colorWipe(strip.Color(255, 0, 0), 100);
          break;
        }        
      }
   }   
}

int WeatherFrame()
{
  static unsigned long s_timer = 0;

  if (client.available())
  {
    bool is_json = false;
    String recvbuf;
    while (client.available())
    {
      char c = client.read();
      if (c == ' ' || c == 10)
        continue;

      if (c == '{')
        is_json = true;

      if (is_json)
        recvbuf += c;
    }
    
    short last = recvbuf.lastIndexOf('}');
    recvbuf.remove(last + 1, recvbuf.length() - last - 1);
//    Serial.println(recvbuf);

    short s = recvbuf.indexOf(F("\"weather\":"));
    short sv = recvbuf.indexOf(F("\"main\":"), s);
    short ev = recvbuf.indexOf(F(","), sv);

    String weather = recvbuf.substring(sv + 8, ev -1);
    weather.toLowerCase();

    Serial.println("---");
    Serial.print("weather : ");
    Serial.println(weather);
    Serial.println("---");
    g_is_reqcved = true;

        tone(speakerpin, 784, 500);
    if (weather.equals("rain")) return 1;
    else if (weather.equals("clouds")) return 2;
    else if (weather.equals("wind")) return 3;
    else if (weather.equals("clear")||weather.equals("haze")) return 4;
    else return 5;     
  }
  else if (g_is_reqcved == false && 
    (millis() - s_timer) >= TIME_WAITING)
  {
    HTTPRequest();
    s_timer = millis();
  }

  return 0;
}

void HTTPRequest()
{
  Serial.println(F("send http request"));
  
  client.stop();
  if (client.connect(SERVER, 80))
  {
    client.print(F("GET /data/2.5/weather?lat="));
    client.print(LAT);
    client.print(F("&lon="));
    client.print(LON);
    client.print(F("&APPID="));
    client.print(OWM_API_KEY);
    client.println(F(" HTTP/1.1"));
    client.println(F("Host: api.openweathermap.org"));
    client.println(F("Connection: close"));
    client.println();
  }
  else
  {
    Serial.println(F("Failed to connect"));
  }
}

//LED 스트립 제어 함수
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}
void colorRain(uint32_t c, uint8_t wait) {
delay(wait);
    for(uint16_t i=0; i<strip.numPixels(); i++) {
      for(int j=255;j>0;j--){
          strip.setPixelColor(i, (0,0,j));
          strip.show();
        delay(1);
      }
    }
}

void colorWind(uint32_t c, uint8_t wait) {
delay(wait);
    for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, (0,0,0));
        strip.show();
        delay(wait);
      }
}
