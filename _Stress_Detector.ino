#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS  1000
#define STRESS_DISPLAY_TIME  10000

LiquidCrystal_I2C lcd(0x27, 16, 2);

byte smile[] = {
  B00000,
  B00000,
  B01010,
  B00000,
  B10001,
  B01110,
  B00000,
  B00000
};
byte mod[] = {
  B00000,
  B00000,
  B01010,
  B00000,
  B11111,
  B00000,
  B00000,
  B00000
};
byte sad[] = {
  B00000,
  B00000,
  B01010,
  B00000,
  B01110,
  B10001,
  B00000,
  B00000
};

PulseOximeter pox;
uint32_t tsLastReport = 0;
uint32_t lastDisplaySwitch = 0;
bool showStressLevel = false;

void onBeatDetected()
{
  Serial.println("Beat!!!");
}

void setup()
{
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.createChar(1 , smile);
  lcd.createChar(2 , mod);
  lcd.createChar(3 , sad);
  lcd.setCursor(0, 0);
  lcd.print("    Stress Level");
  lcd.setCursor(0, 1);
  lcd.print("    Detector");
  delay(2000);

  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop()
{
  pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    lcd.clear();
    if (!showStressLevel) {
      lcd.setCursor(0 , 0);
      lcd.print("BPM : ");
      lcd.print(pox.getHeartRate());
      lcd.setCursor(0 , 1);
      lcd.print("SpO2: ");
      lcd.print(pox.getSpO2());
      lcd.print("%");

      if (pox.getSpO2() >= 96) {
        lcd.setCursor(15 , 1);
        lcd.write(1);                 
      }
      else if (pox.getSpO2() <= 95 && pox.getSpO2() >= 91) {
        lcd.setCursor(15 , 1);
        lcd.write(2);                 
      }
      else if (pox.getSpO2() <= 90) {
        lcd.setCursor(15 , 1);
        lcd.write(3);
      }
    } else {
      int stressLevel = calculateStressLevel(pox.getHeartRate(), pox.getSpO2());
      lcd.setCursor(0, 0);
      lcd.print("Stress Level: ");
      lcd.print(stressLevel);
      lcd.setCursor(0, 1);
      lcd.print("Relax: ");
      lcd.print(getRelaxationTechnique(stressLevel));
    }
    
    tsLastReport = millis();
    if (millis() - lastDisplaySwitch >= STRESS_DISPLAY_TIME) {
      showStressLevel = !showStressLevel;
      lastDisplaySwitch = millis();
    }
  }
}

int calculateStressLevel(float bpm, float spo2) {
  if (spo2 >= 96 && bpm <= 80) return 1;
  if ((spo2 >= 94 && spo2 < 96) || (bpm > 80 && bpm <= 90)) return 2;
  if ((spo2 >= 91 && spo2 < 94) || (bpm > 90 && bpm <= 100)) return 3;
  if ((spo2 >= 88 && spo2 < 91) || (bpm > 100 && bpm <= 110)) return 4;
  return 5;
}

String getRelaxationTechnique(int stressLevel) {
  switch (stressLevel) {
    case 1: return "Breath";
    case 2: return "Music";
    case 3: return "Walk";
    case 4: return "Meditate";
    case 5: return "Rest";
    default: return "None";
  }
}
