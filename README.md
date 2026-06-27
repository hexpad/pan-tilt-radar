
<img width="200" height="113" alt="gif" src="https://github.com/user-attachments/assets/53fb322f-d567-46c2-ad02-43e8b688adf4" />


## What it does

Pan servo sweeps left to right while tilt steps down one row after each pass — raster pattern. When something shows up within 35 cm, servos freeze on that position and it switches to tracking mode. Target gets under 15 cm five times in a row and it triggers the neutralize sequence. Disappears for eight consecutive reads and it unlocks and starts scanning again.

---

## Parts

- ESP32
- HC-SR04 ultrasonic sensor
- 2x SG90 servo (pan + tilt on a 2-axis gimbal)
- 1602 LCD display
- YwRobot breadboard power module

---

## Wiring

| Part | Pin | GPIO |
|---|---|---|
| HC-SR04 | TRIG | 5 |
| HC-SR04 | ECHO | 4 |
| Pan servo | signal | 18 |
| Tilt servo | signal | 19 |
| LCD | RS | 32 |
| LCD | EN | 33 |
| LCD | D4 | 21 |
| LCD | D5 | 22 |
| LCD | D6 | 23 |
| LCD | D7 | 15 |

Servos and LCD run off the YwRobot 5V rail, not the ESP32. Powering servos from the ESP32 causes brownout resets every time they move.

LCD V0 goes straight to GND. No potentiometer needed, contrast is fine.

---

## Libraries

- ESP32Servo — install from Library Manager
- LiquidCrystal — already included in Arduino IDE

---

## Notes

Had two main issues during development. First, moving the tilt servo during tracking was causing the sensor readings to bounce — servo vibration was feeding back into the distance measurements and making the LCD flicker between messages. Fixed it by keeping tilt frozen once a target is locked. Second, single readings were too noisy near threshold boundaries. Averaging 5 readings per loop cycle smoothed it out completely. Also using a counter for both lock and neutralize so one bad reading doesn't throw off the whole system — target has to be out of range for 8 consecutive reads before it gives up.
