#define Czujnik_LM35 14  // pin A0
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Ds1302.h>   // zegar
Ds1302 rtc(9, 7, 8);  //RST CLK DAT

float temperatura = 21;
float sredniaTempDoWyswietlenia = 20;
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
String lewaGoraGodzina, prawaGoraDzien, lewyDol_ON_OFF, prawyDol_Temperatura;
bool kontrolaGodzinDniDrugiejTRaryfy = false;
float sreredniaTemperatyr[] = { 30.1, 30.2, 30.3, 30.4, 30.5 };  // tablica do zbierania kolejnych odczytow
unsigned long kroczkiBierzace = millis();
unsigned long kroczkiPoSpr = 1;
unsigned long kroczkiPoOdczycie = 1;
unsigned long kroczkiPoWyswietleniu = 1;
unsigned long przerwaSpr = 20000;
unsigned long przerwaPoOdczycie = 20000;
unsigned long przerwaWyswietleniu = 200;
unsigned long kroczkiKlawiszy = 1;
char pinBojler = 16;      //A2
char pinWentylator = 17;  //A3

char pinPrzyciskSet = 2;           //D2 biały
char pinPrzyciskPlus = 3;          //D3 czerwony
char pinPrzyciskMinus = 4;         //D4 zielony
char pinPrzyciskWlaczPlus = 13;    //D13 czarny
char pinPrzyciskWylaczMinus = 12;  //D12 biały/biały


boolean stanOdczytany, stanBierzacy;
boolean priorytetUruhom, stanPoprzedni = false;




boolean kontrolkaWlaczeniaBojlera = false;  // kontrolka wlaczonego (true) lub wylaczonego (false) stanu Bojlera

//LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Ustawienie adresu ukladu na 0x27         A4 SDA        A5 SCL
LiquidCrystal_I2C lcd(0x27, 20, 4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.begin(16, 2);
  Serial.begin(9600);
  lcd.print("Bojler");
  pinMode(Czujnik_LM35, INPUT);  //czujnik temperatury

  pinMode(pinPrzyciskSet, INPUT);    //przycisk "SET"
  pinMode(pinPrzyciskPlus, INPUT);   //przycisk "plus"
  pinMode(pinPrzyciskMinus, INPUT);  //przycisk "minus"

  pinMode(pinPrzyciskWlaczPlus, INPUT);    //przycisk "Włącz natychmiast i plus"
  pinMode(pinPrzyciskWylaczMinus, INPUT);  //przycisk "Wyłącz przez odejmowanie czasu włączenia"

  pinMode(pinBojler, OUTPUT);         // Przekaznik jako wyjście dla bojlera D6
  digitalWrite(pinBojler, true);      // Na start wylaczony przekaznik bojler D6
  pinMode(pinWentylator, OUTPUT);     // Przekaznik jako wyjście dla wentylator D5
  digitalWrite(pinWentylator, true);  // Na start wylaczony przekaznik wentylator D5
  rtc.init();

  boolean ustawGodzine = false;  //trzeba zrobić na true żeby można ustawić date i czas
    ////////////ustawianie godziny
  if (ustawGodzine) {
    Ds1302::DateTime dt = {
      .year = 25,
      .month = Ds1302::MONTH_JAN,
      .day = 7,
      .hour = 16,
      .minute = 2,
      .second = 03,
      .dow = Ds1302::DOW_TUE
    };

    rtc.setDateTime(&dt);
  }
}

void loop() {
  kroczkiBierzace = millis();
  Ds1302::DateTime now;
  rtc.getDateTime(&now);
  godziny = now.hour;
  minuty = now.minute;
  sekundy = now.second;
  dzien = now.dow;
  dzienTygodnia = dzien - 1;

  odczytajTemperature();
  if (priorytetUruhom == false)
    sprawdzTaryfe();
  wyswietl();
  ustawGodzine();
  wlaczNatychmiast();
}

void ustawGodzine() {

  if (kroczkiBierzace >= kroczkiKlawiszy) {

    stanOdczytany = digitalRead(pinPrzyciskSet);
    int plusGodzina;
    plusGodzina = digitalRead(pinPrzyciskPlus);
    int plusMinut;
    plusMinut = digitalRead(pinPrzyciskMinus);
    if (stanOdczytany) {

      switch (stanPoprzedni) {
        case false:
          Serial.println("przestaw godzine");
          stanPoprzedni = true;
          stanBierzacy = true;
          break;

        case true:
          Serial.println("Nie przestaiwaj nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn");
          stanPoprzedni = false;
          stanBierzacy = false;
          break;
      }
    }
    if (stanBierzacy > 0) {
      Ds1302::DateTime now;
      rtc.getDateTime(&now);
      Serial.println("SET.SET.SET.SET.SET");
      if (plusGodzina > 0) {
        Serial.println("PluS+Godziny+++++++++++++");
        uint8_t godzinaTemp = now.hour;
        godzinaTemp++;
        Ds1302::DateTime dt = {
          .year = now.year,
          .month = now.month,
          .day = now.day,
          .hour = godzinaTemp,
          .minute = now.minute,
          .second = now.second,
          .dow = now.dow
        };
        rtc.setDateTime(&dt);
      }
      if (plusMinut > 0) {
        Serial.println("MINUTY+++++++PLUS");
        uint8_t minutaTemp = now.minute;
        minutaTemp++;
        Ds1302::DateTime dt = {
          .year = now.year,
          .month = now.month,
          .day = now.day,
          .hour = now.hour,
          .minute = minutaTemp,
          .second = now.second,
          .dow = now.dow
        };
        rtc.setDateTime(&dt);
      }
    }

    kroczkiKlawiszy = kroczkiBierzace + 300;
  }
}

void wlaczNatychmiast() {
  if (digitalRead(pinPrzyciskWlaczPlus)) {
    Serial.println("WŁĄCZ_natychmiast_____ON");
    uruchomPrzekaznikNr(pinBojler);
    priorytetUruhom = true;
    kontrolkaWlaczeniaBojlera = true;
  }

  if (digitalRead(pinPrzyciskWylaczMinus)) {
    Serial.println("Wyłącz-------OFFFFF");
    wylaczPrzekaznikNr(pinBojler);
    priorytetUruhom = false;
    kontrolkaWlaczeniaBojlera = false;
  }
}

void uruchomPrzekaznikNr(char pinPrzekaznika) {
  digitalWrite(pinPrzekaznika, true);
}
void wylaczPrzekaznikNr(char pinPrzekaznika) {
  digitalWrite(pinPrzekaznika, false);
}

void wyswietl() {
  //////////////    tu wyswietlam bierzaca godzine   ////////////////////////
  if (kroczkiBierzace - kroczkiPoWyswietleniu > przerwaWyswietleniu) {
    String godzina, minuta, sekunda;
    if (godziny < 10)  // jak godziny od 0 do 9 to trzeba zero dopisac zeby ładnie było
      godzina = "0" + godziny;
    else godzina = godziny;
    if (minuty < 10)  // jak minuty od 0 do 9 to trzeba zero dopisac
      minuta = "0" + minuty;
    else minuta = minuty;
    if (sekundy < 10)  // jak sekundy od 0 do 9 to trzeba zero dopisac
      sekunda = "0" + sekundy;
    else sekunda = sekundy;
    lewaGoraGodzina = godzina + ":" + minuta + ":" + sekunda;
    prawaGoraDzien = DniTygodnia[dzienTygodnia];

    if (kontrolkaWlaczeniaBojlera == true)  // tu sprawdzam ktora wersje wyswietlic
    {
      lewyDol_ON_OFF = "Boj ON  ";
    } else if (kontrolkaWlaczeniaBojlera == false) {
      lewyDol_ON_OFF = "Boj OFF ";
    }
    if (stanBierzacy == true) {
      prawyDol_Temperatura = "ZMIANA";
    } else {
      prawyDol_Temperatura = sredniaTempDoWyswietlenia;
      prawyDol_Temperatura = prawyDol_Temperatura + " ";
    }
    lcd.setCursor(0, 0);
    lcd.print(lewaGoraGodzina);
    lcd.setCursor(9, 0);
    lcd.print(prawaGoraDzien);
    lcd.setCursor(0, 1);
    lcd.print(lewyDol_ON_OFF);
    lcd.setCursor(10, 1);
    lcd.print(prawyDol_Temperatura);
    Serial.print(godziny);
    Serial.print(":");
    Serial.print(minuty);
    Serial.print(":");
    Serial.println(sekundy);

    kroczkiPoWyswietleniu = kroczkiBierzace;
  }
}

void odczytajTemperature() {
  if (kroczkiBierzace - kroczkiPoOdczycie > przerwaPoOdczycie) {
    temperatura = ((analogRead(Czujnik_LM35) * 5.0) / 1024.0) * 100;
    Serial.print("temperatura:");
    Serial.println(temperatura);
    Serial.println(analogRead(Czujnik_LM35));
    // if (temperatura > (sredniaTempDoWyswietlenia - (sredniaTempDoWyswietlenia * 0.1))
    //     && temperatura < (sredniaTempDoWyswietlenia + (sredniaTempDoWyswietlenia * 0.1))) {
    wyciagnijSredniaTemperature(temperatura);
    // }
    kroczkiPoOdczycie = kroczkiBierzace;
  }
}

void sprawdzTaryfe() {

  if (kroczkiBierzace - kroczkiPoSpr > przerwaSpr) {  // zamiast delay
    kontrolaGodzinDniDrugiejTRaryfy = false;          // zeruje kontrolke stanu bierzacej taryfy
    if (godziny >= 22 || godziny <= 5) {
      kontrolaGodzinDniDrugiejTRaryfy = true;  // nadaje kontrolce stan true bo to druga taryfa
      Serial.println("godziny sa takie same nocne");
    }
    if (godziny >= 13 && godziny < 15) {
      kontrolaGodzinDniDrugiejTRaryfy = true;
      Serial.println("godziny sa takie same dzienna");
    }
    if (dzien >= 6) {
      kontrolaGodzinDniDrugiejTRaryfy = true;
      Serial.print("godziny weekendowe ");
      Serial.println(dzienTygodnia);
    }
    if (kontrolaGodzinDniDrugiejTRaryfy) {
      Serial.print(godziny);
      Serial.print(":");
      Serial.print(minuty);
      Serial.print("   ...");
      Serial.println(" wlaczam bojler");
      kontrolkaWlaczeniaBojlera = true;  // kontrolka stanu wlaczenia bojlera
      uruchomPrzekaznikNr(pinBojler);
      uruchomPrzekaznikNr(pinWentylator);
    } else if (!kontrolaGodzinDniDrugiejTRaryfy) {
      kontrolkaWlaczeniaBojlera = false;
      wylaczPrzekaznikNr(pinBojler);
      wylaczPrzekaznikNr(pinWentylator);
      Serial.println("    Wyłaczony   OFF >>>   ");
    }
    bezpiecznikTermiczny(sredniaTempDoWyswietlenia);
    kroczkiPoSpr = kroczkiBierzace;
  }
}


void bezpiecznikTermiczny(float sredniaTempDoWyswietlenia) {
  if (sredniaTempDoWyswietlenia > 70.0) {
    kontrolkaWlaczeniaBojlera = false;
  }
}

void wyciagnijSredniaTemperature(float temperatura) {
  if (sreredniaTemperatyr[4] < 1) {
    sreredniaTemperatyr[0] = 10;
    sreredniaTemperatyr[1] = 50;
    sreredniaTemperatyr[2] = 40;
    sreredniaTemperatyr[3] = 30;
    sreredniaTemperatyr[4] = 20;
  }
  sreredniaTemperatyr[4] = sreredniaTemperatyr[3];
  sreredniaTemperatyr[3] = sreredniaTemperatyr[2];
  sreredniaTemperatyr[2] = sreredniaTemperatyr[1];
  sreredniaTemperatyr[1] = sreredniaTemperatyr[0];
  sreredniaTemperatyr[0] = temperatura;
  float sumaTemp = 0;
  sumaTemp = sreredniaTemperatyr[0]
             + sreredniaTemperatyr[1]
             + sreredniaTemperatyr[2]
             + sreredniaTemperatyr[3]
             + sreredniaTemperatyr[4];
  float sredniaTemp = 0;
  sredniaTemp = sumaTemp * 0.2;  // zamiast dzielic przez 5 mnoze przez 0,2
  sredniaTempDoWyswietlenia = sredniaTemp;
  Serial.print("suma temp ");
  Serial.println(sumaTemp);
  Serial.print("srednia ");
  Serial.println(sredniaTempDoWyswietlenia);
  Serial.println(temperatura);
}