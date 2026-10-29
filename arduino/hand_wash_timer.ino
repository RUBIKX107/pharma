#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);  // RS, E, D4, D5, D6, D7

const int BUTTON_PIN = 7;
const int GREEN_LED = 8;

const unsigned long WASH_MS = 20000UL;     // 20-second scrub
const unsigned long DONE_SHOW_MS = 3000UL;
const unsigned long DRAW_INTERVAL_MS = 100;

const int STATE_IDLE = 0;
const int STATE_RUNNING = 1;
const int STATE_DONE = 2;

int state = STATE_IDLE;
unsigned long startedAt = 0;
unsigned long doneAt = 0;
unsigned long lastDraw = 0;
int washCount = 0;

bool btnStable = HIGH;
bool btnLastReading = HIGH;
unsigned long btnLastChange = 0;

// true once per debounced press
bool buttonPressed(unsigned long now) {
  bool reading = digitalRead(BUTTON_PIN);
  if (reading != btnLastReading) {
    btnLastChange = now;
    btnLastReading = reading;
  }
  if (now - btnLastChange > 30 && reading != btnStable) {
    btnStable = reading;
    if (btnStable == LOW) return true;
  }
  return false;
}

void printLine(int row, const char* text) {
  lcd.setCursor(0, row);
  int n = 0;
  while (text[n] != '\0' && n < 16) {
    lcd.print(text[n]);
    n++;
  }
  while (n < 16) {
    lcd.print(' ');
    n++;
  }
}

void printBar(int row, int filled) {
  lcd.setCursor(0, row);
  for (int i = 0; i < 16; i++) {
    if (i < filled) lcd.write((uint8_t)255);  // solid block
    else lcd.print(' ');
  }
}

void draw(unsigned long now) {
  char line[24];

  if (state == STATE_IDLE) {
    printLine(0, "Press to start");
    snprintf(line, sizeof(line), "Washes done: %d", washCount);
    printLine(1, line);

  } else if (state == STATE_RUNNING) {
    unsigned long elapsed = now - startedAt;
    int remaining = (WASH_MS - elapsed + 999) / 1000;  // round up
    snprintf(line, sizeof(line), "Scrub!  %2d sec", remaining);
    printLine(0, line);
    printBar(1, (int)(elapsed * 16UL / WASH_MS));

  } else {
    printLine(0, "Done! Rinse");
    printLine(1, "Well done");
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(GREEN_LED, OUTPUT);
  lcd.begin(16, 2);
  Serial.println("ms,event,wash_count");
}

void loop() {
  unsigned long now = millis();
  bool pressed = buttonPressed(now);

  if (state == STATE_IDLE) {
    if (pressed) {
      state = STATE_RUNNING;
      startedAt = now;
      digitalWrite(GREEN_LED, LOW);
      Serial.print(now); Serial.print(",start,"); Serial.println(washCount);
    }

  } else if (state == STATE_RUNNING) {
    if (pressed) {  // cancel
      state = STATE_IDLE;
      Serial.print(now); Serial.print(",cancel,"); Serial.println(washCount);
    } else if (now - startedAt >= WASH_MS) {
      state = STATE_DONE;
      doneAt = now;
      washCount++;
      digitalWrite(GREEN_LED, HIGH);
      Serial.print(now); Serial.print(",complete,"); Serial.println(washCount);
    }

  } else {  // DONE
    if (pressed || now - doneAt >= DONE_SHOW_MS) {
      state = STATE_IDLE;
      digitalWrite(GREEN_LED, LOW);
    }
  }

  if (now - lastDraw >= DRAW_INTERVAL_MS) {
    lastDraw = now;
    draw(now);
  }
}