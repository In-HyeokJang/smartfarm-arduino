const int switchPin = 2; // 스위치 핀 번호
const int ledPin = 13;   // LED 핀 번호
int switchState = 0;     // 스위치 현재 읽기 값
int lastSwitchState = 0; // 스위치 이전 상태 값
bool ledState = false;   // LED의 현재 상태 (켜짐/꺼짐)
unsigned long lastDebounceTime = 0;  // 마지막으로 스위치 상태가 바뀐 시간
unsigned long debounceDelay = 50;    // 채터링 방지 딜레이(50ms)

void setup() {
  pinMode(switchPin, INPUT);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // 스위치의 현재 상태를 읽어옵니다.
  int reading = digitalRead(switchPin);

  // 스위치 상태가 이전과 달라졌다면 (눌리거나 떼어짐)
  if (reading != lastSwitchState) {
    lastDebounceTime = millis(); // 타이머 리셋
  }

  // 50ms 동안 상태가 안정적으로 유지되었다면
  if ((millis() - lastDebounceTime) > debounceDelay) {
    
    // 스위치 상태가 실제로 변경된 경우
    if (reading != switchState) {
      switchState = reading;

      // 스위치를 새로 누른 상태(HIGH)일 때만 토글 실행
      if (switchState == HIGH) {
        ledState = !ledState; // 현재 LED 상태를 반전 (!) 시킴
      }
    }
  }

  // 변경된 LED 상태를 핀에 출력합니다.
  digitalWrite(ledPin, ledState);

  // 현재 읽은 값을 이전 상태 값으로 저장합니다.
  lastSwitchState = reading;
}
