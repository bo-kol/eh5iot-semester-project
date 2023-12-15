//https://web.archive.org/web/20171206220525/https://docs.particle.io/tutorials/integrations/webhooks/#what-39-s-the-weather-like-
//OFFLINE_TEST_MODE forhindrer API-kald. Udkommentér for at kalde API'en
//#define OFFLINE_TEST_MODE

#include <map>

//DOKUMENTATION AF DE TIDSRELATEREDE FUNKTIONER: https://docs.particle.io/reference/device-os/api/time/delaymicroseconds/#hour-
struct priceTime {
  int day;
  int hour;

  bool operator<(struct priceTime const& other) const { //https://stackoverflow.com/questions/8016880/c-less-operator-overload-which-way-to-use
  //const'ET I MIDTEN AF DET HELE VAR LØSNINGEN PÅ DINE PROBLEMER I OG FØR PROJEKTUGEN. https://stackoverflow.com/questions/5511482/help-with-c-stl-map-and-operator-overloading
    if (day < other.day) {
      return true;
    }
    else if ((day == other.day) && (hour < other.hour)) {
      return true;
    }
    else {
      return false;
    }
  }

  void increment() {
    if (hour < 23) {
      this->hour++;
    }
    else if (day >= 7) {
      this->day = 1;
      this->hour = 0;
    }
    else {
      this->day++;
      this->hour = 0;
    }
  }

  void setToNow() {
    this->day = Time.weekday();
    this->hour = Time.hour();
  }
};
#define TIME struct priceTime
struct priceType {
  double price;
  bool active;
};
std::map<TIME, priceType> prices;

#include <string>
#include <algorithm>

#ifndef OFFLINE_TEST_MODE
//Ventetid imellem iterationer af den overordnede programløkke (i ms):
#define WAIT_TIME 120000
//Ventetid imellem tjek af værdien af ready (i ms):
#define POLL_RETRY_TIME 1000
#else
#define WAIT_TIME 10000
#endif

#define PRINT_TIME Serial.println("The current time is " + String(Time.now()) + ". (" + String(Time.hour()) + ":" + String(Time.minute()) + ":" + String(Time.second()) + " on day " + String(Time.weekday()) + ")")

//Til strtok:
#define SEPARATOR ","
//Til zone()
#define TIMEZONE +1

double pv; //Procesværdi, altså aktuel temperatur
//Herfra læses pv:
#define SENSOR_PIN A0
//Setpunktets værdi repræsenterer en temperatur i grader Celsius. For nu er setpunktet statisk:
#define SP 18

//D2 er dét PWM-kapable ben, der er længst ude på Argon'en, altså længst fra enden med Micro USB-porten (https://docs.particle.io/assets/images/argon/argon-pinout-v1.0.pdf)
#define OUTPUT_PIN D2

bool ready = false; //For at forhindre, at programmet går videre, imens handleData eksekveres
bool currentDataUpToDate = false;
TIME currentDataTime;

void setup() {
  //Til temperatursensoren:
  pinMode(SENSOR_PIN, INPUT);
  //Til outputkredsløbet:
  pinMode(OUTPUT_PIN, OUTPUT);

  //Så programmet ikke tror, at det er en time længere tilbage i tiden, end det er:
  Time.zone(TIMEZONE); //Dette vil vare ved i resten af programmets levetid

  // Begin serial comm.
  Serial.begin(115200);

  #ifndef OFFLINE_TEST_MODE
  Serial.println("THE PROGRAM WILL NOW RUN NORMALLY");
  
  // Subscribe to the webhook response event
  Particle.subscribe("hook-response/power-prices", handleData);

  // Give ourselves 10 seconds before we actually start the
  // program.  This will open the serial monitor before
  // the program sends the request
  for(int i=0;i<10;i++) {
      Serial.println("waiting " + String(10-i) + " seconds before we publish");
      delay(1000);
  }
  #else
  Serial.println("THE PROGRAM WILL NOW RUN IN OFFLINE TEST MODE");
  #endif
}

#define SELECTION_PROPORTION_BIAS 0.02
#ifdef OFFLINE_TEST_MODE
#include <iostream>
#include <fstream>
void handleData() {
  //Test af LM35 (kode baseret på den herfra: https://community.particle.io/t/lm35-temperature-sensor/23246)
  int rawTemperature = analogRead(SENSOR_PIN);
  Serial.println("Raw temperature value: " + String(rawTemperature));
  double celsiusTemperature = rawTemperature / 12.4; //Værdien fra ADC'en stiger med 12,4 for hver stigning på 10 mV (DET ER NOK EN GOD IDÉ AT FINDE NOGET I SYSTEM ON CHIP-BESKRIVELSEN ELLER SÅDAN NOGET, DER BAKKER DETTE OP)
  Serial.println("Temperature in °C: " + String(celsiusTemperature));
}
#else
std::vector<double> priceArray; //STL-arrays kan under ingen omstændigheder optimeres til at have en variabel længde, selvom lortet defineres indenfor et scope

double readTemperature() {
  //Test af LM35 (kode baseret på den herfra: https://community.particle.io/t/lm35-temperature-sensor/23246)
  int rawTemperature = analogRead(SENSOR_PIN);
  Serial.println("Raw temperature value: " + String(rawTemperature));
  double celsiusTemperature = rawTemperature / 12.4; //Værdien fra ADC'en stiger med 12,4 for hver stigning på 10 mV (DET ER NOK EN GOD IDÉ AT FINDE NOGET I SYSTEM ON CHIP-BESKRIVELSEN ELLER SÅDAN NOGET, DER BAKKER DETTE OP)
  Serial.println("Temperature in °C: " + String(celsiusTemperature));

  return celsiusTemperature;
}

int length;
void reactivate(struct priceTime selectedTime) {
  double selectionProportion = 0.5 + (pv - SP) * SELECTION_PROPORTION_BIAS; //Bør ikke overstige 1. pv - SP giver afstanden fra setpunktet
  Serial.println("The following selection proportion has been chosen: " + String(selectionProportion));
  if (selectionProportion > 1) { //Nedenfor (i erklæringen af target) SKAL der ikke refereres til out of bounds-elementer i priceArray
    selectionProportion = 1;
  } else if (selectionProportion < 0) {
    selectionProportion = 0;
  }
  double target = priceArray[round(length * selectionProportion)]; //Tilnærmelsesvist den midterste værdi i de sorterede elpriser
  Serial.println("Target price for activation: " + String(target));

  selectedTime.setToNow();
  for (int i = 0; i <= length - 1; i++) {
    if (prices[selectedTime].price <= target) { //Med denne implementation vil flere timer end ønsket gives det grønne lys, hvis mere end én er præcis lig med target? Hvis alle priser var lige høje/lave, ville alle være aktive
      prices[selectedTime].active = true;
      Serial.println("Price " + String(i) + " is ACTIVE.");
    }
    else {
      prices[selectedTime].active = false;
      Serial.println("Price " + String(i) + " is INACTIVE.");
    }

    selectedTime.increment();
  }
}

void handleData(const char *event, const char *data) { //NY VERSION, HVOR JEG PRØVER AT BRUGE strtok
  // Handle the webhook response
  Serial.println("Event name: " + String(event));
  Serial.println("Event data: " + String(data));

  TIME selectedTime;
  selectedTime.setToNow();

  char priceList[strlen(data) + 1];
  strcpy(priceList, data);
  char * thisPrice = strtok(priceList, SEPARATOR);
  
  int i = 0;
  while (thisPrice != NULL && i <= 34) { //i-betingelsen burde ikke være nødvendig, men bruges for at forhindre, at Argon'en forsøger at assign'e tusindvis af gange i tilfælde af en fejl
    prices[selectedTime].price = atof(thisPrice);
    Serial.println("Price " + String(i) + " for the time " + String(selectedTime.day) + "-" + String(selectedTime.hour) + " has been assigned the value " + String(prices[selectedTime].price) + "."); //Dobbelt typecast sikrer, at den intenderede variabel har antaget den rigtige værdi, ikke bare den midlertidige værdi thisPrice
    i++;
    selectedTime.increment(); //Er dette positioneret korrekt???
    thisPrice = strtok(NULL, SEPARATOR); //FORMÅL???????????? DET ER I KODEEKSEMPLET FRA https://cplusplus.com/reference/cstring/strtok/ AF EN ELLER ANDEN ÅRSAG
  }

  //De timer, hvor strømgennemløb skal tillades, bestemmes:
  length = i + 1;
  //double priceArray[length];
  selectedTime.setToNow();
  for (i = 0; i <= length - 1; i++) {
    priceArray.push_back(prices[selectedTime].price);
    selectedTime.increment();
  }
  //std::sort(priceArray[0], priceArray[length - 1]); //Burde sortere i rækkefølge fra mindst til størst
  //qsort(priceArray, length, sizeof(double), NOGET UNDERLIGT LORT);
  std::sort(priceArray.begin(), priceArray.end()); //ER DET DENNE FUNKTION, DER FORÅRSAGER DEN UNDERLIGE COMPILER-FEJL??? 29-10-2023
  
  reactivate(selectedTime);

  currentDataUpToDate = true;
  currentDataTime.setToNow();
  ready = true;
}
#endif

//Fra og med dette minuttal opdateres prisene (for at vide sig mere sikker på, at API'en sender nogle nye data i stedet for at sende de gamle, selvom timetallet er 13):
#define MINIMUM_MINUTES 15

void loop()
{
  PRINT_TIME;
  #ifndef OFFLINE_TEST_MODE
  ready = false;
  TIME now;

  pv = readTemperature(); //Læs den aktuelle temperatur
  Serial.println("Current process value: " + String(pv));

  if (!currentDataUpToDate) {
    Serial.println("Requesting power prices...");
    Particle.publish("power-prices", NULL);
  
    now.setToNow();

    while (!ready) {
      delay(POLL_RETRY_TIME); //Poll'er ready
    }
  } else {
    now.setToNow();
    if (((currentDataTime.day != now.day) || (currentDataTime.hour < 13)) && (((now.hour >= 13) && (Time.minute() >= MINIMUM_MINUTES)) || (now.hour > 13))) { //Venstresiden af det første AND er til for at sikre, at de opringelige data enten blev modtaget enten dagen før el. før kl. 13 den nuværende dag. Højresiden er til for at sikre, at klokken er fyldt mindst 13. ER DET ET PROBLEM, AT JEG BRUGER Time.minute() PÅ ET ANDET TIDSPUNKT END DE ANDRE FEATURES, JEG BRUGER DERAF?
      currentDataUpToDate = false;
    }

    reactivate(now);
  }

  if (prices[now].active == true) {
    Serial.println("ENABLING OUTPUT CIRCUIT");
    analogWrite(OUTPUT_PIN, 255); //Skriv outputbenets maksimumværdi i default-8-bittilstanden til outputbenet
  } else {
    Serial.println("DISABLING OUTPUT CIRCUIT");
    analogWrite(OUTPUT_PIN, 0); //Skriv 0 til outputbenet
  }

  delay(WAIT_TIME);
  #else
  handleData();
  Serial.println("The process will be repeated in " + String(WAIT_TIME / 1000) + " seconds...");
  delay(WAIT_TIME);
  #endif
}
