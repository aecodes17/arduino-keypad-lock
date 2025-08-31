#include <Arduino.h>
#include <Keypad.h>

// ---------- CONFIG ----------
const char* USER_PIN   = "519204";   // change me
const char* DURESS_PIN = "940031";   // change me
const uint8_t PIN_LEN  = 6;

const uint8_t RELAY_PIN  = 10;       // drives relay or LED
const uint8_t BUZZER_PIN = 11;       // ACTIVE buzzer (+ to this pin, - to GND)
const uint16_t UNLOCK_MS = 3000;     // relay pulse

// Lockout policy
const uint8_t  MAX_TRIES        = 5;
const uint32_t BASE_BACKOFF_MS  = 30000; // 30s; grows after each extra fail

// ---------- KEYPAD SETUP ----------
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
// Adjust to your wiring
byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8, 9};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ---------- STATE ----------
uint8_t failedTries = 0;
uint32_t lockedUntil = 0;

// ---------- HELPERS ----------
bool isLockedOut() {
  return millis() < lockedUntil;
}

void applyBackoff() {
  failedTries++;
  if (failedTries >= MAX_TRIES) {
    uint8_t over = failedTries - (MAX_TRIES - 1); // 1,2,3...
    uint32_t duration = BASE_BACKOFF_MS * over;   // linear growth; simple
    lockedUntil = millis() + duration;
    Serial.print("[LOCKOUT] Backoff ms: ");
    Serial.println(duration);
  }
}

void pulseRelay() {
  digitalWrite(RELAY_PIN, HIGH);
  delay(UNLOCK_MS);
  digitalWrite(RELAY_PIN, LOW);
}

void logEvent(const char* evt, const char* detail = "") {
  Serial.print("["); Serial.print(evt); Serial.print("] ");
  Serial.println(detail);
}

String readDigits(uint8_t digitsNeeded) {
  String s = "";
  logEvent("INPUT", "Enter digits (auto-submit at length)");
  while (s.length() < digitsNeeded) {
    char k = keypad.getKey();
    if (!k) continue;

    if (k == '*') { // backspace
      if (s.length() > 0) s.remove(s.length()-1);
      Serial.println("* (backspace)");
    } else if (k == '#') {
      // ignore # in this simple flow (no submit key needed)
    } else if (k >= '0' && k <= '9') {
      s += k;
      Serial.print("*"); // don’t echo digits
      Serial.println();
    } else {
      // ignore A/B/C/D in this minimal build
    }
  }
  return s;
}

// ---------- BUZZER PATTERNS (ACTIVE BUZZER: HIGH = ON) ----------
void beep(uint16_t on_ms, uint16_t off_ms, uint8_t times) {
  for (uint8_t i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(on_ms);
    digitalWrite(BUZZER_PIN, LOW);
    if (i + 1 < times) delay(off_ms);
  }
}

// Success: quick beep-beep
void successChirp() {
  beep(120, 100, 2); // 120ms ON, 100ms gap, twice
}

// Duress: same as success (you can change if you want it distinct)
void duressChirp() {
  beep(120, 100, 2);
}

// Error: long buzz
void errorBuzz() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(700); // long buzz
  digitalWrite(BUZZER_PIN, LOW);
  delay(120);
}

// Lockout: repeating short chirps while locked
void lockoutChirpOnce() {
  beep(60, 0, 1);  // one short chirp
}

// ---------- SETUP & LOOP ----------
void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  logEvent("BOOT", "Ready");
}

void loop() {
  if (isLockedOut()) {
    // Visual + audible "locked" heartbeat
    digitalWrite(RELAY_PIN, HIGH); delay(50);
    digitalWrite(RELAY_PIN, LOW);  delay(150);
    lockoutChirpOnce();            delay(150);
    return;
  }

  // 1) Read PIN
  String pin = readDigits(PIN_LEN);

  // 2) Duress PIN: looks like success but logs an alert
  if (pin.equals(DURESS_PIN)) {
    logEvent("ACCESS", "DURESS");
    duressChirp();      // beep-beep
    pulseRelay();       // looks like normal unlock
    failedTries = 0;    // reset counter
    delay(250);
    return;
  }

  // 3) Check user PIN
  if (pin.equals(USER_PIN)) {
    logEvent("ACCESS", "OK");
    successChirp();     // beep-beep
    pulseRelay();
    failedTries = 0;
  } else {
    logEvent("FAIL", "Bad PIN");
    errorBuzz();        // long buzz
    applyBackoff();
    // brief LED blink for error (optional)
    digitalWrite(RELAY_PIN, HIGH); delay(120);
    digitalWrite(RELAY_PIN, LOW);  delay(120);
  }

  delay(200); // small idle delay
}
