// =============================================
// 스마트팜 자동 급수 시스템
// Spring Boot 대시보드 연동 버전
//
// [송신] Arduino → PC (3초마다)
//   형식: SOIL:850,PUMP:OFF
//
// [수신] PC → Arduino (대시보드 수동 제어)
//   형식: PUMP_ON 또는 PUMP_OFF
// =============================================

// 핀 설정
const int SOIL_SENSOR_PIN = A0;  // 토양수분 센서 아날로그 핀
const int RELAY_PIN = 7;         // 릴레이 디지털 핀

// 자동 제어 임계값 (analogRead 범위: 0 ~ 1023, 높을수록 건조)
const int DRY_THRESHOLD = 600;   // 이상이면 건조 → 펌프 자동 ON
const int WET_THRESHOLD = 400;   // 이하면 충분 → 펌프 자동 OFF

// 서버 시뮬레이션 주기(3초)와 동일하게 설정
const unsigned long SEND_INTERVAL = 3000;

bool isPumping = false;
unsigned long lastSendTime = 0;

// 수동 제어 모드: 대시보드 버튼 누르면 30초간 자동 제어 억제
bool manualMode = false;
unsigned long manualModeUntil = 0;

void setup() {
  // baud rate는 application.yaml serial.baud-rate: 9600 과 반드시 일치해야 함
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  // 릴레이 모듈은 LOW가 ON이므로 초기값 HIGH(OFF)로 설정
  digitalWrite(RELAY_PIN, HIGH);
}

void loop() {
  // 서버(대시보드)에서 수동 제어 명령 수신
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "PUMP_ON") {
      manualMode = true;
      manualModeUntil = millis() + 30000;
      pumpOn();
    } else if (cmd == "PUMP_OFF") {
      manualMode = true;
      manualModeUntil = millis() + 30000;
      pumpOff();
    }
  }

  unsigned long now = millis();
  if (now - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = now;

    int soilRaw = analogRead(SOIL_SENSOR_PIN);

    // 수동 모드 타임아웃 해제
    if (manualMode && millis() > manualModeUntil) {
      manualMode = false;
    }

    // 자동 제어: 수동 모드일 땐 스킵
    if (!manualMode) {
      if (soilRaw >= DRY_THRESHOLD && !isPumping) {
        pumpOn();
      } else if (soilRaw <= WET_THRESHOLD && isPumping) {
        pumpOff();
      }
    }

    // 서버로 데이터 전송 (이 줄 외에 다른 Serial.print 금지 — 서버 파싱 오류 원인)
    Serial.print("SOIL:");
    Serial.print(soilRaw);
    Serial.print(",PUMP:");
    Serial.println(isPumping ? "ON" : "OFF");
  }
}

/**
 * 릴레이를 켜서 펌프를 작동시킵니다.
 * 릴레이 모듈은 LOW 신호에서 ON 동작합니다.
 */
void pumpOn() {
  digitalWrite(RELAY_PIN, LOW);
  isPumping = true;
}

/**
 * 릴레이를 꺼서 펌프를 정지시킵니다.
 */
void pumpOff() {
  digitalWrite(RELAY_PIN, HIGH);
  isPumping = false;
}
