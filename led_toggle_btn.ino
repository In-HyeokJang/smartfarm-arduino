const int LED_PIN = 13;
const int BTN_PIN = 2;   // 버튼 → 2번 핀

bool ledState = false;
bool lastBtnState = HIGH;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);  // 내부 풀업 저항 사용
}

void loop() {
  bool currentBtnState = digitalRead(BTN_PIN);

  // 눌리는 순간(HIGH → LOW)에만 토글
  if (lastBtnState == HIGH && currentBtnState == LOW) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  }

  lastBtnState = currentBtnState;
  delay(50);  // 채터링 방지
}