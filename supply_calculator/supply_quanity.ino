#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);  // RS, E, D4, D5, D6, D7

const int BTN_NEXT = 6;
const int BTN_UP = 7;
const int BTN_DOWN = 8;

int quantity = 60;     // tablets in the bottle
int perDose = 1;       // tablets per dose
int timesPerDay = 2;   // doses per day
int field = 0;         // 0 = quantity, 1 = per dose, 2 = times per day

struct Button {
  int pin;
  bool stable;
  bool lastReading;
  unsigned long lastChange;
  unsigned long pressedAt;
  unsigned long lastRepeat;
};

Button btnNext = {BTN_NEXT, HIGH, HIGH, 0, 0, 0};
Button btnUp   = {BTN_UP,   HIGH, HIGH, 0, 0, 0};
Button btnDown = {BTN_DOWN, HIGH, HIGH, 0, 0, 0};

// true on a debounced press; with repeat, also fires while held down
bool buttonTriggered(Button &b, unsigned long now, bool repeat) {
  bool reading = digitalRead(b.pin);
  if (reading != b.lastReading) {
    b.lastChange = now;
    b.lastReading = reading;
  }
  if (now - b.lastChange > 30 && reading != b.stable) {
    b.stable = reading;
    if (b.stable == LOW) {
      b.pressedAt = now;
      b.lastRepeat = now;
      return true;
    }
  }
  if (repeat && b.stable == LOW && now - b.pressedAt > 500 && now - b.lastRepeat > 100) {
    b.lastRepeat = now;
    return true;
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

void adjust(int delta) {
  if (field == 0)      quantity    = constrain(quantity + delta, 1, 999);
  else if (field == 1) perDose     = constrain(perDose + delta, 1, 9);
  else                 timesPerDay = constrain(timesPerDay + delta, 1, 9);
}

void draw() {
  int dailyUse = perDose * timesPerDay;  // never 0, every value starts at 1
  int days = quantity / dailyUse;        // integer division rounds down
  int leftover = quantity % dailyUse;

  char line[24];
  snprintf(line, sizeof(line), "Qty%03d Dose%d x%d", quantity, perDose, timesPerDay);
  printLine(0, line);

  if (leftover > 0) snprintf(line, sizeof(line), "%d days +%d tab", days, leftover);
  else              snprintf(line, sizeof(line), "%d days", days);
  printLine(1, line);

  // blinking cursor shows which number you're editing
  const int cols[3] = {3, 11, 14};
  lcd.setCursor(cols[field], 0);

  Serial.print(quantity);   Serial.print(",");
  Serial.print(perDose);    Serial.print(",");
  Serial.print(timesPerDay); Serial.print(",");
  Serial.print(days);       Serial.print(",");
  Serial.println(leftover);
}

void setup() {
  Serial.begin(9600);
  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  lcd.begin(16, 2);
  lcd.blink();
  Serial.println("quantity,per_dose,times_per_day,days,leftover");
  draw();
}

void loop() {
  unsigned long now = millis();
  bool changed = false;

  if (buttonTriggered(btnNext, now, false)) { field = (field + 1) % 3; changed = true; }
  if (buttonTriggered(btnUp, now, true))    { adjust(+1); changed = true; }
  if (buttonTriggered(btnDown, now, true))  { adjust(-1); changed = true; }

  if (changed) draw();
}