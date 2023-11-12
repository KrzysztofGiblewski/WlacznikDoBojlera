
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Ds1302.h>    // zegar
#include <IRremote.h>  // biblioteka

IRrecv irrecv(6);     // D9
Ds1302 rtc(9, 7, 8);  //RST CLK DAT

decode_results results;  //kod odebrany przez czujnik
int godziny = 12;        // ta zmienna bedzie przechowywac godzine
int minuty = 15;         // ta zmienna bedzie przechowywac minuty
int sekundy = 20;        // ta zmienna bedzie przechowywac sekundy

int minutyOKtorychWylaczySieWentylator = 0;      // to znaczy ze jak bedzie ustawione 10:07 na 3min to ta zmienna bedzie miala wartosc 10 bo minuty10 (7+3)
int limitCzasuWlaczeniaWent = 30;                // maksymalny czas wlaczenia wentylatora
int sumaCzasuWlaczeniaWentylatora = 0;           // sumujemy interwaly wlaczenia wentylatora (max patrz limitCzasuWlaczeniaWentylatora)
int interwaCzasulWlaczeniaWentylatora = 5;       // interwal dodawania czasu dzialania wentylatora w minutach
int minutyPoprzednie = 0;                        // taka wartosc tymczasowa zeby mozna bylo zobaczyc czy bierzaca minuta nie jest rowna poprzedniej minucie
int godzinaWentylator;                           // tylko po to zeby muc wyswietlic godzinke po przekroczeniu kolejnej godziny
boolean kontrolkaWlaczonegoWentylatora = false;  // kontrolka wlaczonego (true) lub wylaczonego (false) stanu wentylatora
boolean kontrolkaWlaczonegoLadowania = false;    // kontrolka wlaczonego ladowania

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Ustawienie adresu ukladu na 0x27         A4 SDA        A5 SCL
int dlugoscTablicyGodzin = 12;
int dlugoscTablicyMinut = 2;

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);
  lcd.print("0");
  pinMode(A0, INPUT_PULLUP);  // Przycisk dodawania sztuki A0
  pinMode(A1, INPUT_PULLUP);  // Przycisk odejmowania A1
  pinMode(A2, INPUT_PULLUP);  // Przycisk dodawania opuznienia
   // pinMode(A3, OUTPUT);  // Konfiguracja A3 jako wyjście dla buzzera
  pinMode(2, OUTPUT);         // Przekaznik jako wyjście dla ladowarek D2
  digitalWrite(2, false);     // Na start wylaczony przekaznik ladowarek D2
   // pinMode(3, OUTPUT);        // Przekaznik jako wyjście dla wentylatora D3
  //digitalWrite(3, true);   // Na start wylaczony przekaznik wentylatora D3
  irrecv.enableIRIn();  // uruchamia odbiornik podczerwieni

  rtc.init();

 boolean ustawGodzine=false;  //trzeba zrobić na true żeby można ustawić date i czas
  ////////////ustawianie godziny
if(ustawGodzine){
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
  sekundy = now.second;


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

  sumaCzasuWlaczeniaWentylatora += interwaCzasulWlaczeniaWentylatora;  //to dodaj do minut wylaczenia czas
  kontrolkaWlaczonegoWentylatora = true;
  godzinaWentylator = godziny;
  if (limitCzasuWlaczeniaWent < sumaCzasuWlaczeniaWentylatora)  // a jak przekroczy limit czasu dlugosci dzialania
  {
    sumaCzasuWlaczeniaWentylatora = 0;  // to wyzerowanie przez nastawienie minut wylaczenia na biezace minuty
  }
  minutyOKtorychWylaczySieWentylator = minuty + sumaCzasuWlaczeniaWentylatora;  // ustawienie koncowej minuty
  if (minutyOKtorychWylaczySieWentylator > 59) {
    minutyOKtorychWylaczySieWentylator = minutyOKtorychWylaczySieWentylator - 60;
    godzinaWentylator++;
  }
  if (godzinaWentylator > 23) {
    godzinaWentylator = 0;
  }
  delay(800);  // pauza zeby klikniecia nie byly zbyt szybkie bo wtedy ciezko cos ustawic
}



void wyswietl() {
  lcd.setCursor(0, 0);
  if (kontrolkaWlaczonegoWentylatora == true)  // tu sprawdzam ktora wersje wyswietlic
  {
    lcd.print("Wentyla ON ");
    lcd.print(godzinaWentylator);
    lcd.print(":");
    if (minutyOKtorychWylaczySieWentylator < 10)  // jak minutyOKtorychWylaczySieWentylator od 0 do 9 to trzeba zero dopisac
      lcd.print(0);
    lcd.print(minutyOKtorychWylaczySieWentylator);

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

  ////////   Wentylator    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  if (sumaCzasuWlaczeniaWentylatora > 0 && kontrolkaWlaczonegoWentylatora == true)  // jesli kontrolka wylaczenia wentylatora wylaczona
  {
    digitalWrite(6, false);  // przekaznik wentylatora wlaczony
  }
  if (minutyOKtorychWylaczySieWentylator == minuty && kontrolkaWlaczonegoWentylatora == true)  // jak minutyOKtorychWylaczySieWentylator czyli minuty rowne  nastawione minuty to koniec
  {
    digitalWrite(6, true);                   // przekaznik wylaczony wentylator nie dziala
    kontrolkaWlaczonegoWentylatora = false;  // kontrolka zmiana stanu na wylaczony
  }



  int tablicaGodzin[] = { 13, 22 };
  int tablicaMinut[] = { 01 };

  if (kontrolkaWlaczonegoWentylatora == false)  // kontrolka zmiana stanu na wylaczony

    for (int j = 0; j < dlugoscTablicyMinut; j++)
      if (minuty == tablicaMinut[j]) {
        Serial.println("minuty sa takie same");
        for (int i = 0; i < dlugoscTablicyGodzin; i++) {
          if (godziny == tablicaGodzin[i]) {
            Serial.println("godziny sa takie same");

            kontrolkaWlaczonegoWentylatora = true;
            Serial.println("klik ");
            Serial.print(j);
            kliknij();
          }
        }
      }
}
