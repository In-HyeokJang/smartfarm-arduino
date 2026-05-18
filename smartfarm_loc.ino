#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// 1. 부품 핀 설정
#define DHTPIN 2          // 온습도 센서(DHT11)를 확장 쉴드 디지털 2번(D2)에 연결
#define DHTTYPE DHT11     // 센서 종류를 DHT11로 지정
#define SOIL_PIN A0       // 토양 수분 센서를 확장 쉴드 아날로그 0번(A0)에 연결

// 2. 부품 객체 초기화 (LCD 주소는 보통 0x27 또는 0x3F입니다)
LiquidCrystal_I2C lcd(0x27, 16, 2); 
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // PC 모니터(시리얼 모니터 콘솔) 출력 시작 설정
  Serial.begin(9600);
  
  // 온습도 센서 및 LCD 화면 시작
  dht.begin();
  lcd.init();
  lcd.backlight(); // LCD 파란색 백라이트 켜기
  
  // 첫 오프닝 화면 출력
  lcd.setCursor(0, 0);
  lcd.print("   IoT MAKER   ");
  lcd.setCursor(0, 1);
  lcd.print("  STATION OPEN  ");
  delay(2000); // 2초 동안 환영 문구 유지
  lcd.clear();
}

void loop() {
  // 데이터 읽어오기 (감각 기관 작동)
  float humidity = dht.readHumidity();       // 공기 중 습도 읽기
  float temperature = dht.readTemperature(); // 현재 온도 읽기
  int soilMoisture = analogRead(SOIL_PIN);   // 흙 속 수분 값 읽기 (0 ~ 1023)

  // 센서 오류 값 체크
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("센서 연결을 확인하세요!");
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error!!  ");
    return;
  }

  // ----------------------------------------------------
  // [1단계] 개발자 시점: 노트북 PC 콘솔(시리얼 모니터)에 찍기
  // ----------------------------------------------------
  Serial.print("Temp: "); Serial.print(temperature); Serial.print("C | ");
  Serial.print("Humid: "); Serial.print(humidity); Serial.print("% | ");
  Serial.print("Soil: "); Serial.println(soilMoisture);

  // ----------------------------------------------------
  // [2단계] 참여자 시점(PEAK): 물리적인 보드판 LCD 화면에 찍기
  // ----------------------------------------------------
  // LCD 첫 번째 줄 (현재 온도와 습도 표시)
  lcd.setCursor(0, 0);
  lcd.print("T: ");
  lcd.print((int)temperature);
  lcd.print("C  ");
  
  lcd.print("H: ");
  lcd.print((int)humidity);
  lcd.print("%   ");

  // LCD 두 번째 줄 (토양 수분 상태 표시)
  lcd.setCursor(0, 1);
  lcd.print("Soil Moisture:");
  lcd.print(soilMoisture);
  lcd.print("   "); // 이전 글자 잔상 지우기용 공백

  delay(1500); // 1.5초마다 데이터를 새로고침하며 화면을 갱신합니다.
}