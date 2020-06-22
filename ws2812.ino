//制作bilibili:小白菜2233，基于esp8266链接小爱同学控制ws2812彩色灯带（完整版）
//功能：调节RGB：0-255内任何颜色，调节亮度，实现灯带开关
//控制：小爱同学语音控制或手机APPblinker点击控制
//配网：实现手机智能配网，保持手机和esp在同一wifi下，输入密码实现给esp8266配网
//已经加入小爱语音控制灯带模式如 彩虹呼吸灯，跑马灯，影院追逐，单色呼吸灯，日光灯等有7种模式
//按键控制有10种模式，小爱只有7中模式可以调用加不了10种
//代码上传github

#define BLINKER_PRINT Serial
#define BLINKER_WIFI
#define BLINKER_MIOT_LIGHT
#include <Blinker.h>
#include <FastLED.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

char auth[] = "1155b30107b2";/****秘钥****/
#define PIN 15  //  DIN PIN (GPIO15, D8)
#define LED_PIN  8//DIN FASTLED PIN
#define NUMPIXELS 60  // Number of you led
CRGB leds[NUMPIXELS];
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
BlinkerRGB RGB1("RGB");
BlinkerButton Button1("BT");

int stat = 10;
int hue = 0;
bool wsState;
uint8_t wsMode = BLINKER_CMD_MIOT_DAY;
int LED_R=0,LED_G=0,LED_B=0,LED_Bright=180;// RGB和亮度
bool WIFI_Status = true;
void smartConfig()//配网函数
{
  WiFi.mode(WIFI_STA);
  Serial.println("\r\nWait for Smartconfig...");
  WiFi.beginSmartConfig();//等待手机端发出的用户名与密码
  while (1)
  {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);  
    delay(1000);                      
    digitalWrite(LED_BUILTIN, LOW);    
    delay(1000);                      
    if (WiFi.smartConfigDone())//退出等待
    {
      Serial.println("SmartConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      break;
    }
  }
}
void WIFI_Set()//
{
    //Serial.println("\r\n正在连接");
    int count = 0;
    while(WiFi.status()!=WL_CONNECTED)
    {
        if(WIFI_Status)
        {
            Serial.print(".");
            digitalWrite(LED_BUILTIN, HIGH);  
            delay(500);                       
            digitalWrite(LED_BUILTIN, LOW);    
            delay(500);                 
            count++;
            if(count>=5)//5s
            {
                WIFI_Status = false;
                Serial.println("WiFi连接失败，请用手机进行配网"); 
            }
        }
        else
        {
            smartConfig();  //微信智能配网
        }
     }  
    /* Serial.println("连接成功");  
     Serial.print("IP:");
     Serial.println(WiFi.localIP());*/
}


void SET_RGB(int R,int G,int B,int bright)
{
   
    
    LED_R = R;
    LED_G = G;
    LED_B = B;
    for (uint16_t i = 0; i < NUMPIXELS; i++) //把灯条变色
    {
        pixels.setPixelColor(i,R,G,B);
    }
    
    pixels.setBrightness(bright);//亮度
    
    pixels.show();    //送出显示
}

//APP RGB颜色设置回调
void rgb1_callback(uint8_t r_value, uint8_t g_value, 
                    uint8_t b_value, uint8_t bright_value)
{
    
    //digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    BLINKER_LOG("R value: ", r_value);
    BLINKER_LOG("G value: ", g_value);
    BLINKER_LOG("B value: ", b_value);
    BLINKER_LOG("Rrightness value: ", bright_value);
    LED_Bright = bright_value;
    SET_RGB(r_value,g_value,b_value,LED_Bright);
}

//小爱设置颜色回调
void miotColor(int32_t color)
{
    BLINKER_LOG("need set color: ", color);
    int colorR,colorG,colorB;
    colorR = color >> 16 & 0xFF;
    colorG = color >>  8 & 0xFF;
    colorB = color       & 0xFF;

    BLINKER_LOG("colorR: ", colorR, ", colorG: ", colorG, ", colorB: ", colorB);
    SET_RGB(colorR,colorG,colorB,LED_Bright);
    //pixel.Show();

    BlinkerMIOT.color(color);//反馈小爱控制状态
    BlinkerMIOT.print();
}
//小爱设置亮度回调
void miotBright(const String & bright)
{
    BLINKER_LOG("need set brightness: ", bright);

    int colorW = bright.toInt();

    BLINKER_LOG("now set brightness: ", colorW);
    LED_Bright = colorW*2.55;
    SET_RGB(LED_R,LED_G,LED_B,LED_Bright);
    Serial.printf("亮度调节中...%d",colorW);
    
    BlinkerMIOT.brightness(colorW);//反馈小爱控制状态
    BlinkerMIOT.print();
}

//小爱电源类回调
void miotPowerState(const String & state)
{
    BLINKER_LOG("need set power state: ", state);

    if (state == BLINKER_CMD_ON) {
        //digitalWrite(LED_BUILTIN, LOW);
        SET_RGB(255,255,255,255);        
        BlinkerMIOT.powerState("on");
        BlinkerMIOT.print();
    }
    else if (state == BLINKER_CMD_OFF) {
        //digitalWrite(LED_BUILTIN, HIGH);
        SET_RGB(0,0,0,0);
        BlinkerMIOT.powerState("off");
        BlinkerMIOT.print();
    }
}

//小爱模式回调
void miotMode(uint8_t mode)
{
    BLINKER_LOG("need set mode: ", mode);

    if (mode == BLINKER_CMD_MIOT_DAY) {
       stat = 10;
    }
    else if (mode == BLINKER_CMD_MIOT_NIGHT) {
     
       stat = 6;
      
    }
    else if (mode == BLINKER_CMD_MIOT_COLOR) {
        
        stat = 7;
        
    }
    else if (mode == BLINKER_CMD_MIOT_WARMTH) {
        stat = 9;
          
    }
    else if (mode == BLINKER_CMD_MIOT_TV) {
     
       stat = 2;
      
    }
    else if (mode == BLINKER_CMD_MIOT_READING) {
      stat = 0;
    }
    else if (mode == BLINKER_CMD_MIOT_COMPUTER) {
       
        stat = 1;
       
    }

    wsMode = mode;

    BlinkerMIOT.mode(mode);
    BlinkerMIOT.print();
}



按键回调
void button1_callback(const String & state)
{
    BLINKER_LOG("get button state: ", state);
    stat = stat + 1;
  if (stat > 10) stat = 0;
}



void LSD() {
  for (int i = 0; i < NUMPIXELS; i++) {
    leds[i] = CRGB::Red;
    FastLED.show();
    delay(2);
    leds[i] = CRGB::Black;
  }
}

void CHHXD() {
  for (int i = 0; i < NUMPIXELS; i++) {
    leds[i] = CHSV( (hue + (255 / NUMPIXELS) * i), 255, 255); //用HSV色彩空间，不断改变H即可
    FastLED.show();
  }
  delay(2);
  hue = (hue + 3) % 255;
}
void doLedStep() {
 for (int i = 0; i < NUMPIXELS; i++) {
    leds[i] = CHSV( hue, 255, 255); //用HSV色彩空间，不断改变H即可
    FastLED.show();
  }
  delay(2);
  hue = (hue + 3) % 255;
}
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j = 0; j < 10; j++) { //do 10 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < pixels.numPixels(); i = i + 3) {
        pixels.setPixelColor(i + q, c);  //turn every third pixel on
      }
      pixels.show();
      delay(wait);

      for (uint16_t i = 0; i < pixels.numPixels(); i = i + 3) {
        pixels.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, c);
    pixels.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256; j++) {
    if (stat != 3) break;
    Blinker.run();
    for (i = 0; i < pixels.numPixels(); i++) {
      if (stat != 3) break;
      Blinker.run();
      pixels.setPixelColor(i, Wheel((i + j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}

void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    if (stat != 4) break;
    Blinker.run();
    for (i = 0; i < pixels.numPixels(); i++) {
      if (stat != 4) break;
      Blinker.run();
      pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}

void theaterChaseRainbow(uint8_t wait) {
  for (int j = 0; j < 256; j++) {   // cycle all 256 colors in the wheel
    if (stat != 5) break;
    Blinker.run();
    for (int q = 0; q < 3; q++) {
      if (stat != 5) break;
      Blinker.run();
      for (uint16_t i = 0; i < pixels.numPixels(); i = i + 3) {
        if (stat != 5) break;
        Blinker.run();
        pixels.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
      }
      pixels.show();
      delay(wait);

      for (uint16_t i = 0; i < pixels.numPixels(); i = i + 3) {
        if (stat != 5) break;
        Blinker.run();
        pixels.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}


void setup() {
  
    Serial.begin(115200);
    FastLED.addLeds<WS2812,LED_PIN, GRB>(leds, NUMPIXELS);
    pixels.begin();//WS2812初始化
    pixels.show();
    pinMode(LED_BUILTIN, OUTPUT);
    #if defined(BLINKER_PRINT)
        BLINKER_DEBUG.stream(BLINKER_PRINT);
    #endif
  
  WIFI_Set();
  // 初始化blinker
    Blinker.begin(auth, WiFi.SSID().c_str(), WiFi.psk().c_str());
    RGB1.attach(rgb1_callback);//注册调节颜色的回调函数
    Button1.attach(button1_callback);
    BlinkerMIOT.attachPowerState(miotPowerState);
    BlinkerMIOT.attachColor(miotColor);//小爱调节颜色
    BlinkerMIOT.attachBrightness(miotBright);//小爱调节RGB亮度
    BlinkerMIOT.attachMode(miotMode);//小爱调节模式
}

void loop() {
  //Serial.println(stat);
 if (stat == 0 || stat == 1  || stat == 2 || stat == 6 || stat == 7 || stat == 8 || stat == 9 || stat == 10 ) Blinker.run();
  switch (stat) {
    case 0:
      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(255, 255, 255));
        pixels.show();
      }
      break;
    case 1: theaterChase(pixels.Color(0, 236, 255), 100);
      break;
    case 2:
      colorWipe(pixels.Color(255, 0, 0), 30);// Red
      colorWipe(pixels.Color(0, 255, 0), 30);// Green
      colorWipe(pixels.Color(0, 0, 255), 30);// Blue
      colorWipe(pixels.Color(255, 255, 255), 30);// 白
      colorWipe(pixels.Color(255, 255, 0), 30);// 黄
      colorWipe(pixels.Color(0, 255, 255), 30);// 青
      colorWipe(pixels.Color(255, 0, 255), 30);// Blue
      break;
    case 3: rainbow(20);
      break;
    case 4: rainbowCycle(20);
      break;
    case 5: theaterChaseRainbow(20);
      break;
      case 6:
      theaterChase(pixels.Color(127, 127, 127), 50); // White
      theaterChase(pixels.Color(255, 0, 0), 50); // Red
      theaterChase(pixels.Color(0, 0, 255), 50); // Blue剧院追逐
      theaterChase(pixels.Color(0, 255, 0), 50);
      theaterChase(pixels.Color(255, 0, 255), 50);
      theaterChase(pixels.Color(255, 255, 0), 50);
      theaterChase(pixels.Color(0, 255, 255), 50);
      break;
      case 7:doLedStep(); 
      break;
      case 8:LSD(); 
      break;
      case 9:CHHXD(); 
      break;
      case 10:pixels.begin(); 
      break;
  }
}
