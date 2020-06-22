//制作bilibili:小白菜2233，基于esp8266链接小爱同学控制ws2812彩色灯带
//功能：调节RGB：0-255内任何颜色，调节亮度，实现灯带开关
//控制：小爱同学语音控制或手机APPblinker点击控制
//配网：实现手机智能配网，保持手机和esp在同一wifi下，输入密码实现给esp8266配网
//后期准备加入语音控制灯带模式如 呼吸灯，跑马灯，已写好6种模式
//代码上传github

#define BLINKER_PRINT Serial
#define BLINKER_WIFI
#define BLINKER_MIOT_LIGHT
#include <Blinker.h>
#include <Adafruit_NeoPixel.h>



char auth[] = "1155b30107b2";/****秘钥****/
#define PIN 15  //  DIN PIN (GPIO15, D8)
#define NUMPIXELS 60  // Number of you led
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
BlinkerRGB RGB1("RGB");



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
//小爱设置颜色回调
void miotBright(const String & bright)
{
    BLINKER_LOG("need set brightness: ", bright);

    int colorW = bright.toInt();

    BLINKER_LOG("now set brightness: ", colorW);
    LED_Bright = colorW;
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


void setup() {
  
Serial.begin(115200);
 pixels.begin();//WS2812初始化
    pixels.show();
    pinMode(LED_BUILTIN, OUTPUT);
    #if defined(BLINKER_PRINT)
        BLINKER_DEBUG.stream(BLINKER_PRINT);
    #endif
  // 初始化blinker
    Blinker.begin(auth, WiFi.SSID().c_str(), WiFi.psk().c_str());
    RGB1.attach(rgb1_callback);//注册调节颜色的回调函数
    BlinkerMIOT.attachPowerState(miotPowerState);
    BlinkerMIOT.attachColor(miotColor);//小爱调节颜色
    BlinkerMIOT.attachBrightness(miotBright);//小爱调节RGB亮度
}

void loop() {
  
Blinker.run();
}
