// =============================================
// 스마트팜 자동 급수 시스템
// 토양 수분 감지 → 임계값 이하 → 펌프 ON
// =============================================

// 핀 설정
const int SOIL_SENSOR_PIN = A0;  // 토양수분 센서 아날로그 핀
const int RELAY_PIN = 7;         // 릴레이 디지털 핀

// 임계값 설정 (0 ~ 1023)
// 숫자가 높을수록 = 흙이 건조한 상태
// 숫자가 낮을수록 = 흙이 촉촉한 상태
const int DRY_THRESHOLD = 600;   // 이 값 이상이면 건조 → 펌프 ON
const int WET_THRESHOLD = 400;   // 이 값 이하면 충분 → 펌프 OFF

// 펌프 동작 시간 설정
const int PUMP_ON_TIME = 3000;  // 펌프 작동 시간 (3초)
const int CHECK_INTERVAL = 5000; // 센서 체크 주기 (5초)

bool isPumping = false;
unsigned long lastCheckTime = 0;

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // 릴레이 초기값 OFF (HIGH = OFF)
  Serial.println("=== 스마트팜 시스템 시작 ===");
}

void loop() {
  unsigned long currentTime = millis();

  // 설정한 주기마다 센서 체크
  if (currentTime - lastCheckTime >= CHECK_INTERVAL) {
    lastCheckTime = currentTime;

    int soilValue = analogRead(SOIL_SENSOR_PIN);

    // 시리얼로 데이터 전송 (대시보드 연동용)
    Serial.print("SOIL:");
    Serial.print(soilValue);
    Serial.print(",PUMP:");
    Serial.println(isPumping ? "ON" : "OFF");

    // 상태 출력
    Serial.print("토양 수분값: ");
    Serial.print(soilValue);

    if (soilValue >= DRY_THRESHOLD && !isPumping) {
      // 건조 → 펌프 ON
      Serial.println(" → 건조! 펌프 가동");
      pumpOn();

    } else if (soilValue <= WET_THRESHOLD && isPumping) {
      // 충분히 촉촉 → 펌프 OFF
      Serial.println(" → 충분! 펌프 중지");
      pumpOff();

    } else {
      Serial.println(" → 정상 범위");
    }
  }
}

void pumpOn() {
  digitalWrite(RELAY_PIN, LOW);  // LOW = 릴레이 ON
  isPumping = true;
  Serial.println("[펌프 ON]");
}

void pumpOff() {
  digitalWrite(RELAY_PIN, HIGH); // HIGH = 릴레이 OFF
  isPumping = false;
  Serial.println("[펌프 OFF]");
}