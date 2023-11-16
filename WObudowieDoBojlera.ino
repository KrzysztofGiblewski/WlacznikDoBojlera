
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Ds1302.h>   // zegar
Ds1302 rtc(9, 7, 8);  //RST CLK DAT

int godziny = 12;  // ta zmienna bedzie przechowywac godzine
int minuty = 10;
int sekundy = 15;
int dzien = 7;
int tablicaGodzin[] = { 13, 14,21, 22,  23, 00, 01, 02, 03, 04, 05 };
int dlugoscTablicyGodzin = sizeof(tablicaGodzin) / sizeof(int);

boolean kontrolkaWlaczeniaBojlera = false;  // kontrolka wlaczonego (true) lub wylaczonego (false) stanu Bojlera

//LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Ustawienie adresu ukladu na 0x27         A4 SDA        A5 SCL
LiquidCrystal_I2C lcd(0x27, 20, 4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.begin(16, 2);
  Serial.begin(9600);
  lcd.print("Bojler");
  pinMode(6, OUTPUT);     // Przekaznik jako wyjście dla ladowarek D2
  digitalWrite(6, true);  // Na start wylaczony przekaznik ladowarek D2
  rtc.init();

  boolean ustawGodzine = false;  //trzeba zrobić na true żeby można ustawić date i czas
    ////////////ustawianie godziny
  if (ustawGodzine) {
    Ds1302::DateTime dt = {
      .year = 23,
      .month = Ds1302::MONTH_NOV,
      .day = 16,
      .hour = 19,
      .minute = 19,
      .second = 03,
      .dow = Ds1302::DOW_TUE
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


  wyswietl();
  sprawdz();
}


void uruchomBojlej() {
  digitalWrite(2, true);  // uruchamiamy pin D2
}

void wylaczBojlej() {
  digitalWrite(2, false);  // wyłączamy pin D2
}

void wyswietl() {
  lcd.setCursor(0, 0);
  if (kontrolkaWlaczeniaBojlera == true)  // tu sprawdzam ktora wersje wyswietlic
  {
    lcd.print("ON Bojler wlaczony ON ...");

  } else if (kontrolkaWlaczeniaBojlera == false) {
    lcd.print("Bojler OFF");
  }
  Serial.println(godziny + " " + minuty);
  //////////////    tu wyswietlam bierzaca godzine   ////////////////////////
  lcd.setCursor(0, 1);
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
  lcd.print(" dzie-");
  int dzienTygodnia=dzien+2;
  lcd.print(dzienTygodnia);
}


void sprawdz() {
  boolean kontrolkaTemp = false;
  for (int i = 0; i < dlugoscTablicyGodzin; i++) {
    delay(50);
    if (godziny == tablicaGodzin[i]) {
      kontrolkaTemp = true;
    }
    if (godziny != tablicaGodzin[i] && kontrolkaTemp == false)
      kontrolkaTemp = false;
  }if(dzien==5)kontrolkaTemp=true;
  if(dzien==6)kontrolkaTemp=true;
  
  if (kontrolkaTemp) {
    Serial.println("godziny sa takie same ");
    Serial.print(godziny);Serial.print(":");Serial.print(minuty);Serial.print("   ...");
    Serial.println(" wlaczam bojler");
    kontrolkaWlaczeniaBojlera = true;
    uruchomBojlej();

  } else {
    kontrolkaWlaczeniaBojlera = false;
    wylaczBojlej();
    Serial.println("Wyłaczony>>>>>>>>>>>>>>");
  }
  kontrolkaTemp = false;
}