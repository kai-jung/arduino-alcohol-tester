/* MQ-3 Alcohol Sensor Circuit and Bluetooth Control with Arduino */
/* 정경진(건설환경공학부) 2020-2 창의공학설계 */

#include <SoftwareSerial.h>
SoftwareSerial BT_Serial(7,8);

// Warmup variables
int WARMUP_SECONDS = 3;
unsigned long time;

// Measurement variables
int MEASURE_SECONDS = 5;
unsigned long start_at;

// Flags
bool is_ready = false;
bool is_done = false;

// Sensor readiness
int analogPin = A0; // MQ-3 센서 아날로그 입력
int val = 0;

const int buzzerPin = 10;
const int buttonPin = 2;

// LED digital pin 11 and 12
const int redPin = 12;
const int greenPin = 11;

void setup() {
  BT_Serial.begin(9600);
  Serial.begin(9600);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
}

void loop()
{
  int button_state = digitalRead(buttonPin); // Push 버튼 입력시
  
  if (BT_Serial.available()) {
    button_state = BT_Serial.read(); // App 버튼 입력시
  }
  
  if (button_state && !is_ready) {
    is_ready = true;
    start_at = millis()/1000;
    is_done = false;
  }
  delay(333);

  time = millis()/1000;

  // Warmup
  if (time<=WARMUP_SECONDS)
  {
    int progress = map(millis(), 0, WARMUP_SECONDS*1000, 0, 100);
    if (progress >= 100) {
      progress = 100;
    }
    Serial.print("Warming up: ");
    Serial.print(progress);
    Serial.println("%");
  }
  else
  {
    // idle 상태
    if (!is_ready && !is_done) {
      printReady();
    }

    // 측정 중 상태
    if (is_ready && !is_done) {
      printMeasure();
      val = readAlcohol();
    }

    // 측정 종료
    if (is_ready && !is_done && ((time - start_at)>MEASURE_SECONDS)) {
      tone(buzzerPin, 5000);
      delay(1000);
      noTone(buzzerPin);
      is_done = true;
      is_ready = false;
    }

    // 측정 결과 출력
    if (is_done) {
      printAlcohol(val);
      if (val > 300) {
        digitalWrite(redPin, HIGH);
      } else {
        digitalWrite(greenPin, HIGH);
      }
      is_done = false;
      is_ready = false;
    }
  }
}

void printReady() {
  Serial.println("Press to start ...");
}
void printMeasure() {
  Serial.println("Breathe until the sound beeps");
}
void printAlcohol(int value) {
  Serial.print("Sensor reading: ");
  Serial.println(val);
//  val = val + 500;
  char *info = "";
  if (val>300) {
    info = "면허 정지 수준입니다.";
  }
  if (val>500) {
    info = "면허 취소 수준입니다.";
  }
  BT_Serial.println(String(val)+","+info); // payload: "368,면허 정지 수준입니다."
  BT_Serial.write('>'); // delimeter
}

int readAlcohol() {
  // 5회 측정하여 평균
  int repeat = 5;
  int sum = 0;
  // Measure
  for (int i = 0; i < repeat; i++) {
    sum += analogRead(analogPin);
  }
  sum = sum / repeat;
  return sum;
}
