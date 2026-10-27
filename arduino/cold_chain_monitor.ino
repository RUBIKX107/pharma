const int SENSOR_PIN = A0;
const int GREEN_LED = 8;
const int RED_LED = 9;
const int BUZZER = 10;

const float TEMP_MIN = 2.0;
const float TEMP_MAX = 8.0;

const unsigned long SAMPLE_INTERVAL_MS = 1000;
const unsigned long ALARM_DELAY_MS = 10000UL;  // demo: 10 s. Real fridge: ~600000UL (10 min)
const unsigned long SIREN_STEP_MS = 250;

unsigned long lastSample = 0;
unsigned long outOfRangeSince = 0;
bool wasOutOfRange = false;
bool alarmActive = false;

unsigned long lastSirenStep = 0;
bool sirenHigh = false;

void setup() {
  Serial.begin(9600);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  Serial.println("ms,temp_c,status,alarm");
}

float readTempC() {
  int raw = analogRead(SENSOR_PIN);
  float voltage = raw * (5.0 / 1023.0);
  return (voltage - 0.5) * 100.0;  // TMP36
}

void updateSiren(unsigned long now) {
  if (!alarmActive) {
    noTone(BUZZER);
    return;
  }
  if (now - lastSirenStep >= SIREN_STEP_MS) {
    lastSirenStep = now;
    sirenHigh = !sirenHigh;
    tone(BUZZER, sirenHigh ? 1200 : 800);
  }
}

void loop() {
  unsigned long now = millis();

  if (now - lastSample >= SAMPLE_INTERVAL_MS) {
    lastSample = now;

    float tempC = readTempC();
    bool outOfRange = (tempC < TEMP_MIN || tempC > TEMP_MAX);

    // start the timer the moment we leave the safe range
    if (outOfRange && !wasOutOfRange) outOfRangeSince = now;
    wasOutOfRange = outOfRange;

    alarmActive = outOfRange && (now - outOfRangeSince >= ALARM_DELAY_MS);

    digitalWrite(GREEN_LED, !outOfRange);
    digitalWrite(RED_LED, outOfRange);

    const char* status = "OK";
    if (tempC > TEMP_MAX) status = "TOO_HOT";
    else if (tempC < TEMP_MIN) status = "TOO_COLD";

    Serial.print(now);
    Serial.print(",");
    Serial.print(tempC);
    Serial.print(",");
    Serial.print(status);
    Serial.print(",");
    Serial.println(alarmActive ? 1 : 0);
  }

  updateSiren(now);  // runs every pass, so the siren timing isn't tied to the 1 s sampling
}