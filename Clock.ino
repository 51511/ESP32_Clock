#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>

// WiFi連接信息
const char* ssid     = "wifi名稱";
const char* password = "密碼";

// NTP伺服器地址
const char* ntpServer = "asia.pool.ntp.org";
const long  utcOffsetInSeconds = 8 * 3600; // 台灣標準時間 +8小時

// 設置UDP連接
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, utcOffsetInSeconds);

// LED設置
#define LED_PIN     5    // LED 数据引脚
#define NUM_LEDS    60   // LED 数量
#define LED_BRIGHTNESS 5
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// 時間變數
unsigned long previousMillis = 0; // 上次更新时间
const long interval = 1000; // 更新间隔（毫秒）
int hour = 0;   // 初始值, 最後會從NTP獲取
int minute = 0; // 初始值
int second = 0; // 初始值

void setup() {
    // 啟動串列監控
    Serial.begin(115200);
    // 初始化LED
    strip.begin();
    strip.show(); // 顯示初始狀態
    // 連接WiFi
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startAttemptTime) < 40000) {
        WiFi.begin(ssid, password);
        delay(1000);
        Serial.println("正在連接WiFi...");
    }
    // 如果超過40秒未連接
    if (WiFi.status() != WL_CONNECTED) {
        // LED全亮紅色2秒
        for (int i = 0; i < NUM_LEDS; i++) {
            strip.setPixelColor(i, strip.Color(255, 0, 0)); // 紅色
        }
        strip.show();
        delay(2000);
        // 設置時間為早上8點
        hour = 8;
        minute = 0;
        second = 0;
    } else {
        // 否則啟動NTPClient並獲取時間
        timeClient.begin();
        timeClient.update();
        // 將NTP獲取的時間存入hour, minute, second
        hour = timeClient.getHours();
        minute = timeClient.getMinutes();
        second = timeClient.getSeconds();
    }
}

void loop() {
    unsigned long currentMillis = millis();
    // 檢查是否到了更新時間
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        // 如果連接WiFi，更新NTP時間
        if (WiFi.status() == WL_CONNECTED) {
            timeClient.update();
            hour = timeClient.getHours();
            minute = timeClient.getMinutes();
            second = timeClient.getSeconds();
        } else {
            // 每秒更新時間
            second = (second + 1) % 60;
            if (second == 0) {
                minute = (minute + 1) % 60;
                if (minute == 0) {
                    hour = (hour + 1) % 24;
                }
            }
        }
        // 持續顯示 LED 時鐘
        updateLEDs();
    }
}

void updateLEDs() {
    // 計算 LED 索引
    int hourIndex = (hour % 24) * 2.5; // 每小時對應 2.5 個 LED
    int minuteIndex = minute; // 每分鐘對應 1 個 LED
    int secondIndex = second; // 每秒對應 1 個 LED

    // 清空所有 LED
    strip.clear();

    // 設置小時 LED（紅色）
    for (int i = hourIndex; i < hourIndex + 5; i++) {
        int ledIndex = (NUM_LEDS - 1 - i); // 反向索引
        if (ledIndex >= 0 && ledIndex < NUM_LEDS) {
            strip.setPixelColor(ledIndex, strip.Color(255, 0, 0)); // 紅色
        }
    }

    // 設置分鐘 LED（綠色）
    int minuteLedIndex = (NUM_LEDS - 1 - minuteIndex); // 反向索引
    if (minuteLedIndex >= 0 && minuteLedIndex < NUM_LEDS) {
        strip.setPixelColor(minuteLedIndex, strip.Color(0, 255, 0)); // 綠色
    }

    // 設置秒 LED（藍色）
    int secondLedIndex = (NUM_LEDS - 1 - secondIndex); // 反向索引
    if (secondLedIndex >= 0 && secondLedIndex < NUM_LEDS) {
        strip.setPixelColor(secondLedIndex, strip.Color(0, 0, 255)); // 藍色
    }

    // 更新 LED 顯示
    strip.show();
}

