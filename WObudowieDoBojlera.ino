
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Ds1302.h>   // zegar
Ds1302 rtc(9, 7, 8);  //RST CLK DAT

float temperatura = 60;
int godziny = 12;  // ta zmienna bedzie przechowywac godzine
int minuty = 10;
int sekundy = 15;
char dzien = 0;
char dzienTygodnia = 2;
const static char* DniTygodnia[] = {
  "Poniedz",
  "Wtorek ",
  "Sroda  ",
  "Czwarte",
  "Piatek ",
  "Sobota ",
  "Niedzie"
};

char pinBojler = 6;
char pinWentylator = 5;
char pinCzujnikaTemperatury = 15;  //czyli analogowy A1 środkowa nóżka

boolean kontrolkaWlaczeniaBojlera = false;  // kontrolka wlaczonego (true) lub wylaczonego (false) stanu Bojlera

//LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Ustawienie adresu ukladu na 0x27         A4 SDA        A5 SCL
LiquidCrystal_I2C lcd(0x27, 20, 4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.begin(16, 2);
  Serial.begin(9600);
  lcd.print("Bojler");
  pinMode(pinBojler, OUTPUT);         // Przekaznik jako wyjście dla bojlera D6
  digitalWrite(pinBojler, true);      // Na start wylaczony przekaznik bojler D6
  pinMode(pinWentylator, OUTPUT);     // Przekaznik jako wyjście dla wentylator D5
  digitalWrite(pinWentylator, true);  // Na start wylaczony przekaznik wentylator D5
  rtc.init();

  boolean ustawGodzine = false;  //trzeba zrobić na true żeby można ustawić date i czas
    ////////////ustawianie godziny
  if (ustawGodzine) {
    Ds1302::DateTime dt = {
      .year = 23,
      .month = Ds1302::MONTH_NOV,
      .day = 27,
      .hour = 11,
      .minute = 54,
      .second = 03,
      .dow = Ds1302::DOW_MON
    };

    rtc.setDateTime(&dt);
  }
}

void loop() {
  Ds1302::DateTime now;
  rtc.getDateTime(&now);
  godziny = now.hour;
  minuty = now.minute;
  sekundy = now.second;
  dzien = now.dow;
  dzienTygodnia = dzien - 1;

  sprawdz();
  wyswietl();

  temperatura = ((analogRead(pinCzujnikaTemperatury) * 5.0) / 1024.0) * 100;
  Serial.print("temperatura:");
  Serial.println(temperatura);
}
void uruchomPrzekaznikNr(char pinPrzekaznika) {
  digitalWrite(pinPrzekaznika, true);
}
void wylaczPrzekaznikNr(char pinPrzekaznika) {
  digitalWrite(pinPrzekaznika, false);
}

void wyswietl() {
  //////////////    tu wyswietlam bierzaca godzine   ////////////////////////
  lcd.setCursor(0, 0);
  if (godziny < 10)  // jak godziny od 0 do 9 to trzeba zero dopisac zeby ładnie było
    lcd.print(0);
  lcd.print(godziny);
  lcd.print(":");
  if (minuty < 10)  // jak minuty od 0 do 9 to trzeba zero dopisac
    lcd.print(0);
  lcd.print(minuty);
  lcd.print(":");
  if (sekundy < 10)  // jak sekundy od 0 do 9 to trzeba zero dopisac
    lcd.print(0);
  lcd.print(sekundy);
  lcd.print(" ");
  lcd.print(DniTygodnia[dzienTygodnia]);
   lcd.setCursor(0, 1);
  if (kontrolkaWlaczeniaBojlera == true)  // tu sprawdzam ktora wersje wyswietlic
  {
    lcd.print("Boj ON temp");    

  } else if (kontrolkaWlaczeniaBojlera == false) {
    lcd.print("Boj OFF temp");
  }
lcd.print(temperatura);


  Serial.print(godziny);
  Serial.print(":");
  Serial.print(minuty);
  Serial.print(":");
  Serial.println(sekundy);
 
}


void sprawdz() {
  boolean kontrolkaTemp = false;
  delay(1000);
  temperatura = ((analogRead(pinCzujnikaTemperatury) * 5.0) / 1024.0) * 100;
  Serial.print("temperatura:");
  Serial.println(temperatura);

  if (godziny >= 22 || godziny <= 5) {
    kontrolkaTemp = true;
    Serial.println("godziny sa takie same nocne");
  }
  if (godziny >= 13 && godziny < 15) {
    kontrolkaTemp = true;
    Serial.println("godziny sa takie same dzienna");
  }
  if (dzienTygodnia >= 6) {
    kontrolkaTemp = true;
    Serial.print("godziny weekendowe ");
    Serial.println(dzienTygodnia);
  }
  if (kontrolkaTemp) {
    Serial.print(godziny);
    Serial.print(":");
    Serial.print(minuty);
    Serial.print("   ...");
    Serial.println(" wlaczam bojler");
    kontrolkaWlaczeniaBojlera = true;
    uruchomPrzekaznikNr(pinBojler);
    //  wylaczPrzekaznikNr(pinBojler);
  } else if (!kontrolkaTemp) {
    kontrolkaWlaczeniaBojlera = false;
    wylaczPrzekaznikNr(pinBojler);
    uruchomPrzekaznikNr(pinWentylator);
    Serial.println("    Wyłaczony   OFF >>>   ");
  }
}