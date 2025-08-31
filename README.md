# 🔐 Arduino Keypad Lock System

This project is a **cybersecurity-inspired Arduino keypad lock**. It simulates an electronic access control system with **PIN entry**, **duress alarm code**, **lockout protection**, and **visual + audible feedback** using an LED (or relay) and a buzzer.  

It’s designed for learning about Arduino, electronics, and basic security principles while building a fun and practical prototype.

---

## ✨ Features

- **6-digit user PIN** – grants access when entered correctly  
- **Duress PIN** – looks like access granted but logs an alert  
- **Lockout & backoff** – after too many failed attempts, the system enforces delays that grow with each mistake (to frustrate brute force attempts)  
- **Buzzer feedback**:  
  - Success → short *beep-beep*  
  - Error → long *buzz*  
  - Lockout → periodic chirps  
- **LED/Relay output** – simulates unlocking a door or enabling a device  
- **Event logging (Serial Monitor)** – shows access granted, denied, duress, and lockout events  

---

## 🛠️ Hardware Required

- Arduino Nano, Uno, or ESP32 (adjust pins if needed)  
- **4x4 Matrix Keypad**  
- **Active buzzer** (5V)  
- **LED + 220Ω resistor** *(or a relay module if you want to drive a lock)*  
- Jumper wires & breadboard  

---

## 🔌 Wiring

| Component     | Arduino Pin | Notes                                      |
|---------------|-------------|--------------------------------------------|
| Keypad Rows   | 2, 3, 4, 5  | Connected row-wise                         |
| Keypad Cols   | 6, 7, 8, 9  | Connected column-wise                      |
| LED/Relay +   | 10          | Use a 220Ω resistor if LED; relay IN if relay |
| LED/Relay –   | GND         |                                            |
| Buzzer +      | 11          | Active buzzer                              |
| Buzzer –      | GND         |                                            |
| Common Ground | GND         | All components share Arduino GND           |

> 📝 **Note**: All GND pins on Arduino are connected internally, so you can use any GND pin that’s convenient.  

---
## 🚀 How It Works

1. The keypad is wired in a matrix (rows + columns). The Arduino scans which row and column get connected when a button is pressed to figure out the digit.  
2. The Arduino waits until **6 digits** are entered.  
3. If the PIN matches the stored **user PIN**, the LED/relay activates for 3 seconds and the buzzer plays a **success beep-beep**.  
4. If the PIN matches the **duress PIN**, the LED/relay also activates, but the Serial Monitor logs `ACCESS: DURESS` (silent alarm).  
5. If the PIN is wrong, the buzzer gives a **long buzz** and failed attempts are counted. After 5 failed attempts, the system enforces **lockout with growing delays**.  
6. During lockout, the buzzer chirps and the LED blinks to show the system is unavailable.  

---

## 💻 Example Serial Output

[BOOT] Ready
[INPUT] Enter digits (auto-submit at length)

[ACCESS] OK
[INPUT] Enter digits (auto-submit at length)

[FAIL] Bad PIN
[LOCKOUT] Backoff ms: 30000


## 🧩 Customization

- Change user PIN and duress PIN at the top of the sketch:
  ```cpp
  const char* USER_PIN   = "519204";
  const char* DURESS_PIN = "940031";

  Adjust the number of attempts before lockout:

const uint8_t MAX_TRIES = 5;


Change lockout timing:

const uint32_t BASE_BACKOFF_MS = 30000; // 30s


Switch to a relay module if you want to control higher-power devices instead of an LED.

🛡️ Security Notes

This is a prototype for learning. A real security system would also include:

Encrypted storage of PINs (not plain text in code)
1. Tamper sensors (lid switch, light sensor)
2. Secure bootloader and firmware protection
3. Event logs stored in non-volatile memory or sent to a server

📸 Future Improvements

1. Add an I²C LCD screen for user feedback
2. Store multiple PINs in EEPROM for different users
3.Use a secure element chip (like ATECC608) to protect keys
4. Add a web dashboard or mobile app for logs/alerts
5. Include a tamper alarm switch

