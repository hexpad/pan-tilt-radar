#include <ESP32Servo.h>
#include <LiquidCrystal.h>


class Sensor {
public:

  void begin() {
    pinMode(5, OUTPUT);
    pinMode(4, INPUT);
  }

  float read() {
    float total = 0;

    for (int i = 0; i < 5; i++) {
      digitalWrite(5, HIGH);
      delayMicroseconds(10);
      digitalWrite(5, LOW);
      long t = pulseIn(4, HIGH, 25000);
      if (t == 0) {
        total += 400;
      }
      else {
        total += t * 0.0343 / 2;
      }
      delay(10);
    }

    return total / 5;
  }
};


class Screen {
  LiquidCrystal lcd{ 32, 33, 21, 22, 23, 15 };
  String lastTop = "";

public:

  void begin() {
    lcd.begin(16, 2);
    delay(200);
    lcd.begin(16, 2);
  }

  void show(const char* top, String bottom) {
    lastTop = String(top);

    while (bottom.length() < 16) {
      bottom += ' ';
    }

    lcd.setCursor(0, 0);
    lcd.print(top);

    lcd.setCursor(0, 1);
    lcd.print(bottom);
  }

  void updateDistance(String bottom) {
    while (bottom.length() < 16) {
      bottom += ' ';
    }

    lcd.setCursor(0, 1);
    lcd.print(bottom);
  }
};


class Gimbal {
  Servo panServo;
  Servo tiltServo;

public:

  int pan  = 30;
  int tilt = 65;

  void begin() {
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    panServo.attach(18, 500, 2400);
    tiltServo.attach(19, 500, 2400);
    panServo.write(pan);
    tiltServo.write(tilt);
    delay(500);
  }

  void move(int newPan, int newTilt) {
    pan  = constrain(newPan,  30, 150);
    tilt = constrain(newTilt, 65, 115);
    panServo.write(pan);
    tiltServo.write(tilt);
  }
};


class Radar {
  Sensor sensor;
  Screen screen;
  Gimbal gimbal;

  int  panDir    = 1;
  bool locked    = false;
  int  lostCount = 0;
  int  killCount = 0;


  void scan(float distance) {

    int newPan  = gimbal.pan + panDir * 5;
    int newTilt = gimbal.tilt;

    bool hitEdge = (newPan >= 150 || newPan <= 30);

    if (hitEdge) {
      newPan  = constrain(newPan, 30, 150);
      panDir  = panDir * -1;

      bool tiltAtBottom = (gimbal.tilt + 10 > 115);

      if (tiltAtBottom) {
        newTilt = 65;
      }
      else {
        newTilt = gimbal.tilt + 10;
      }
    }

    gimbal.move(newPan, newTilt);
    screen.show("** SCANNING **  ",
      "P:" + String(gimbal.pan) + " T:" + String(gimbal.tilt));

    bool targetFound = (distance < 35);

    if (targetFound) {
      locked    = true;
      lostCount = 0;
      killCount = 0;
    }
  }


  void track(float distance) {

    if (distance > 45) {
      lostCount++;
    }
    else {
      lostCount = 0;
    }

    if (distance < 15) {
      killCount++;
    }
    else {
      killCount = 0;
    }

    if (lostCount > 8) {
      screen.show("  TARGET LOST   ", "Scanning...     ");
      delay(800);
      reset();
      return;
    }

    if (killCount > 5) {
      screen.show("###NEUTRALIZED##", "Restarting...   ");
      delay(2000);
      reset();
      return;
    }

    String dist = "Dist: " + String(distance, 1) + " cm";


    if (distance < 15) {
      screen.show("!!! KILL ZONE !!", dist);
    }
    else if (distance < 25) {
      screen.show(">>  APPROACHING ", dist);
    }
    else {
      screen.show("    TARGET LOCK ", dist);
    }
  }


  void reset() {
    locked    = false;
    lostCount = 0;
    killCount = 0;
    panDir    = 1;
    gimbal.move(30, 65);
  }


public:

  void begin() {
    sensor.begin();
    gimbal.begin();
    screen.begin();
    screen.show("  RADAR SYSTEM  ", "  Starting...   ");
    delay(1000);
  }

  void update() {
    float distance = sensor.read();

    if (!locked) {
      scan(distance);
    }
    else {
      track(distance);
    }
  }
};


Radar radar;

void setup() {
  radar.begin();
}

void loop() {
  radar.update();
  delay(200);
}
