#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);

#define VT_PIN A0
#define AT_PIN A1

#define GREEN_PIN 5
#define RED_PIN 4
#define BLUE_PIN 6

double voltageReading = 0;
double currentReading = 0;
double wattageReading = 0;

long lastPowerReading = 0;
long powerReadingInterval = 1000;

long lastDisplayRefresh = 0;
long displayRefreshInterval = 500;

double totalWattHours = 0;
double totalAmpHours = 0;

void setup()
{
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  
  Serial.begin(9600);

  lcd.init();

  lcd.backlight();

  lcd.setCursor(0,0);
  lcd.print("Starting meter");
  
  delay(500);
}

void loop() 
{
  if (lastPowerReading + powerReadingInterval < millis())
  {
    getPower();

    lastPowerReading = millis();
  }

  if (lastDisplayRefresh + displayRefreshInterval < millis())
  {
    displayResult();
    
    lastDisplayRefresh = millis();
  }

  setLED();
}

void displayResult()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(voltageReading);
  lcd.print("v");
  lcd.print(currentReading);
  lcd.print("a");
  lcd.print(totalAmpHours);
  lcd.print("ah");

  lcd.setCursor(0,1);
  lcd.print(wattageReading);
  lcd.print("w  ");
  lcd.print(totalWattHours);
  lcd.print("wh");

}

void setLED()
{
  // TODO: Implement additional LED functionality
  if (voltageReading == 0)
  {
    digitalWrite(BLUE_PIN, HIGH);
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
  }
  else
  {
    digitalWrite(BLUE_PIN, LOW);
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, HIGH);
  }
}

void getPower()
{  
  int vt_temp = analogRead(VT_PIN);
  int at_temp = analogRead(AT_PIN);
  
  long result;
  
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  
  delay(2); // Wait for Vref to settle
  
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result;
  result = result / 1000;
  
  voltageReading = vt_temp * (result / 1023.0) * 5;
  currentReading = at_temp * (result / 1023.0);
  
  Serial.print("A0: ");
  Serial.print(vt_temp);
  Serial.print(" / ");
  Serial.print("A1: ");
  Serial.print(at_temp);
  
  Serial.print(" ----- "); 
  
  Serial.print(voltageReading);
  Serial.print(" ");
  Serial.println(currentReading);

  wattageReading = (voltageReading*currentReading);

  double timeSinceLastReading = millis() - lastPowerReading;
  double secondsSinceLastReading = timeSinceLastReading / 1000;
  
  double hoursSinceLastReading = secondsSinceLastReading / 60 / 60;

  double wattHourIncrease = wattageReading * hoursSinceLastReading;
  double ampHourIncrease = currentReading * hoursSinceLastReading;

  totalWattHours += wattHourIncrease;

  totalAmpHours += ampHourIncrease;
  
}

