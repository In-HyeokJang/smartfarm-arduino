#include <DHT.h>

// ---------- 핀 (기존 배선 유지) ----------
const int PIN_RELAY_LIGHT = 14;  // 릴레이 IN1 - 조명/상시 (Low-trigger)
const int PIN_RELAY_PUMP  = 26;  // 릴레이 IN2 - 펌프 (다음주 연결)
#define DHTPIN        4          // DHT22 OUT
#define DHTTYPE       DHT22
#define PIN_SOIL_AO   34         // HW-103 AO (ADC1 전용)
#define PIN_SOIL_PWR  25         // HW-103 VCC (측정 시에만 ON)

DHT dht(DHTPIN, DHTTYPE);

// ---------- 운영 스위치 ----------
const bool CALIBRATION_MODE = false;  // 재캘리브 필요할 때만 true
const bool LIGHT_ALWAYS_ON  = true;   // CH1 상시 ON (나중에 스케줄 붙이면 false)
const bool PUMP_DRY_RUN     = true;   // 펌프 미연결 → 로그만, 신호 안 보냄
                                      //  ★ 다음주 펌프 연결하면 false로

// ---------- 캘리브레이션 ----------
int RAW_DRY = 4095;   // 공기 중 (측정 완료)
int RAW_WET = 1450;   // 물속 (측정 완료)
// ※ 흙에 꽂은 실제 값 확인 후 아래 임계치 재조정 필요

// ---------- 급수 정책 ----------
const int MOISTURE_ON  = 30;   // 이 % 미만이면 급수
const int MOISTURE_OFF = 55;   // 이 % 넘으면 정지
const unsigned long PUMP_MAX_MS   = 8UL * 1000;         // 1회 최대 8초
const unsigned long PUMP_COOLDOWN = 30UL * 60 * 1000;   // 급수 후 30분 대기
const unsigned long READ_INTERVAL = 5000;

// ---------- 상태 ----------
unsigned long lastRead = 0, pumpStartAt = 0, pumpEndedAt = 0;
bool pumpOn = false;

// ================= 릴레이 =================
void relayWrite(int pin, bool on) { digitalWrite(pin, on ? LOW : HIGH); }

// CH1 조명 — 지금은 상시 ON. 나중에 여기에 스케줄 로직 추가
void updateLight() {
  if (LIGHT_ALWAYS_ON) return;   // setup에서 켠 상태 유지
  // TODO: NTP 시간 기반 작물별 점등 스케줄
}

// ================= 펌프 =================
void pumpStart() {
  if (pumpOn) return;
  pumpOn = true; pumpStartAt = millis();
  if (!PUMP_DRY_RUN) relayWrite(PIN_RELAY_PUMP, true);
  Serial.printf(">> 펌프 ON%s\n", PUMP_DRY_RUN ? " (모의 - 미연결)" : "");
}

void pumpStop(const char* reason) {
  if (!pumpOn) return;
  pumpOn = false; pumpEndedAt = millis();
  relayWrite(PIN_RELAY_PUMP, false);   // 정지는 dry-run이어도 무조건 실행
  Serial.printf(">> 펌프 OFF (%s)%s\n", reason, PUMP_DRY_RUN ? " (모의)" : "");
}

// ================= 토양수분 =================
int readSoilRaw() {
  const int N = 11;
  int s[N];
  digitalWrite(PIN_SOIL_PWR, HIGH); delay(300);      // 전원 ON + 안정화
  for (int i = 0; i < N; i++) { s[i] = analogRead(PIN_SOIL_AO); delay(20); }
  digitalWrite(PIN_SOIL_PWR, LOW);                   // 전원 OFF (부식 방지)

  for (int i = 1; i < N; i++) {                      // 중앙값 필터
    int k = s[i], j = i - 1;
    while (j >= 0 && s[j] > k) { s[j + 1] = s[j]; j--; }
    s[j + 1] = k;
  }
  return s[N / 2];
}

int rawToPercent(int raw) {
  return (int)constrain(map(raw, RAW_DRY, RAW_WET, 0, 100), 0, 100);
}

bool isSoilSane(int raw) { return raw > 20; }   // 합선/단선만 필터

// ================= 급수 판단 =================
void updatePump(int percent, bool soilOk) {
  if (!soilOk) { pumpStop("센서 이상"); return; }          // 안전장치 1

  if (pumpOn) {
    if (millis() - pumpStartAt >= PUMP_MAX_MS) { pumpStop("최대시간 도달"); return; }  // 안전장치 2
    if (percent >= MOISTURE_OFF) { pumpStop("목표 도달"); return; }
  } else {
    if (pumpEndedAt != 0 && millis() - pumpEndedAt < PUMP_COOLDOWN) return;            // 안전장치 3
    if (percent < MOISTURE_ON) pumpStart();
  }
}

// ================= 메인 =================
void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(PIN_RELAY_LIGHT, OUTPUT);
  pinMode(PIN_RELAY_PUMP,  OUTPUT);
  relayWrite(PIN_RELAY_PUMP,  false);            // 펌프는 항상 OFF로 시작
  relayWrite(PIN_RELAY_LIGHT, LIGHT_ALWAYS_ON);  // CH1 상시 ON

  pinMode(PIN_SOIL_PWR, OUTPUT);
  digitalWrite(PIN_SOIL_PWR, LOW);

  analogSetPinAttenuation(PIN_SOIL_AO, ADC_11db);   // 0~3.3V
  analogReadResolution(12);                         // 0~4095

  dht.begin();
  Serial.println("== Nook 노드 시작 ==");
  Serial.printf("CH1(조명/P14): %s\n", LIGHT_ALWAYS_ON ? "상시 ON" : "스케줄");
  Serial.printf("CH2(펌프/P26): %s\n", PUMP_DRY_RUN ? "모의 모드 (미연결)" : "실제 동작");
  Serial.printf("급수 기준: %d%% 미만 -> ON / %d%% 이상 -> OFF\n", MOISTURE_ON, MOISTURE_OFF);
  Serial.println("--------------------------");
}

void loop() {
  // --- 재캘리브레이션용 ---
  if (CALIBRATION_MODE) {
    Serial.printf("soil raw = %4d\n", readSoilRaw());
    delay(1000);
    return;
  }

  // 펌프 타임아웃은 매 루프 체크 (5초 주기에 갇히면 안 됨)
  if (pumpOn && millis() - pumpStartAt >= PUMP_MAX_MS) pumpStop("최대시간 도달");
  updateLight();

  if (millis() - lastRead < READ_INTERVAL) { delay(50); return; }
  lastRead = millis();

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int  raw     = readSoilRaw();
  bool soilOk  = isSoilSane(raw);
  int  percent = soilOk ? rawToPercent(raw) : -1;

  if (isnan(h) || isnan(t)) Serial.print("[DHT22] 읽기 실패   ");
  else                      Serial.printf("[대기] %.1f°C / %.1f%%   ", t, h);

  if (soilOk) Serial.printf("[토양] %d%% (raw %d)   ", percent, raw);
  else        Serial.printf("[토양] 이상 (raw %d)   ", raw);

  Serial.printf("[CH1] %s   [펌프] %s\n",
                digitalRead(PIN_RELAY_LIGHT) == LOW ? "ON" : "OFF",
                pumpOn ? "ON" : "OFF");

  updatePump(percent, soilOk);
}
