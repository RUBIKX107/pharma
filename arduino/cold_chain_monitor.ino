const int SENSOR_PIN = A0;
const int GREEN_LED = 8;
const int RED_LED = 9;
const int BUZZER = 10;

const float TEMP_MIN = 2.0;  // °C, safe range for vaccines/insulin
const float TEMP_MAX = 8.0;

void setup() {
  Serial.begin(9600);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  Serial.println("ms,temp_c,status");  // CSV header
}

void loop() {
  int raw = analogRead(SENSOR_PIN);
  float voltage = raw * (5.0 / 1023.0);
  float tempC = (voltage - 0.5) * 100.0;  // TMP36 formula

  bool inRange = (tempC >= TEMP_MIN && tempC <= TEMP_MAX);

  digitalWrite(GREEN_LED, inRange);
  digitalWrite(RED_LED, !inRange);
  if (inRange) noTone(BUZZER); else tone(BUZZER, 1000);

  Serial.print(millis());
  Serial.print(",");
  Serial.print(tempC);
  Serial.print(",");
  Serial.println(inRange ? "OK" : "ALERT");

  delay(1000);
}