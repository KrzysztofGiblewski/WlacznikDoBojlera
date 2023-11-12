
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Ds1302.h>    // zegar
#include <IRremote.h>  // biblioteka

IRrecv irrecv(6);     // D9
Ds1302 rtc(9, 7, 8);  //RST CLK DAT

decode_results results;  //kod odebrany przez czujnik
int godziny = 12;        // ta zmienna bedzie przechowywac godzine
int minuty = 10;
int sekundy=15;
int dlugoscTablicyGodzin=8;
 int tablicaGodzin[] = { 13, 14, 22, 23, 01, 02, 03, 04 };

boolean kontrolkaWlaczonegoWentylatora = false;  // kontrolka wlaczonego (true) lub wylaczonego (false) stanu wentylatora

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Ustawienie adresu ukladu na 0x27         A4 SDA        A5 SCL

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);
  lcd.print("0");
  // pinMode(A0, INPUT_PULLUP);  // Przycisk dodawania sztuki A0
  //pinMode(A1, INPUT_PULLUP);  // Przycisk odejmowania A1
  //pinMode(A2, INPUT_PULLUP);  // Przycisk dodawania opuznienia
  // pinMode(A3, OUTPUT);  // Konfiguracja A3 jako wyjście dla buzzera
  pinMode(2, OUTPUT);      // Przekaznik jako wyjście dla ladowarek D2
  digitalWrite(2, false);  // Na start wylaczony przekaznik ladowarek D2
                           // pinMode(3, OUTPUT);        // Przekaznik jako wyjście dla wentylatora D3
  //digitalWrite(3, true);   // Na start wylaczony przekaznik wentylatora D3
  irrecv.enableIRIn();  // uruchamia odbiornik podczerwieni

  rtc.init();

  boolean ustawGodzine = false;  //trzeba zrobić na true żeby można ustawić date i czas
    ////////////ustawianie godziny
  if (ustawGodzine) {
    Ds1302::DateTime dt = {
      .year = 23,
      .month = Ds1302::MONTH_OCT,
      .day = 1,
      .hour = 16,
      .minute = 55,
      .second = 30,
      .dow = Ds1302::DOW_TUE
    };

    rtc.setDateTime(&dt);
  }
  //////////////
}

void loop() {
  Ds1302::DateTime now;
  rtc.getDateTime(&now);
  godziny = now.hour;
  minuty = now.minute;
  //sekundy = now.second;


  sprawdz();
  wyswietl();




  if (digitalRead(A1) == LOW)  //przycisk uruchamia wentylator na interwaCzasulWlaczeniaWentylatora minut
  {
    kliknij();
  }
  if (irrecv.decode(&results))  // sprawdza, czy otrzymano sygnał IR
  {
    if (results.value == 551486205) {  ////////////////////////////////// Tu mnusze podmienic wartosc ktora odczytam z pilota
      Serial.println("oooooo mam sygnal");
      kliknij();
    }
    Serial.print(results.value);
    Serial.println(" ");
    irrecv.resume();  // odbiera następną wartość
    delay(100);
  }
}


void kliknij() {

  kontrolkaWlaczonegoWentylatora = true;
}



void wyswietl() {
  lcd.setCursor(0, 0);
  if (kontrolkaWlaczonegoWentylatora == true)  // tu sprawdzam ktora wersje wyswietlic
  {
    lcd.print("Wentyla ON ");
   
  } else if (kontrolkaWlaczonegoWentylatora == false) {
    lcd.print("Wentylator OFF   ");
  }

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
  lcd.print("          ");  // koniec bierzacej godziny
}


void sprawdz() {

 
  if (kontrolkaWlaczonegoWentylatora == true)  // kontrolka zmiana stanu na wylaczony

    for (int i = 0; i < sizeof(tablicaGodzin); i++) {
      if (godziny == tablicaGodzin[i]) {
        Serial.println("godziny sa takie same");
        kontrolkaWlaczonegoWentylatora = true;
        Serial.println("klik ");
        kliknij();
      } else kontrolkaWlaczonegoWentylatora = false;
    }
}