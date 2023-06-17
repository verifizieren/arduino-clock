// Bibliothek einbinden
#include <Bounce2.h>

// Definition der Taster-Pins
#define Taster1 A1
#define Taster2 A2
#define Taster3 A3
#define LATCH_DIO 4
#define CLK_DIO 7
#define DATA_DIO 8

// Segment-Byte-Mapping für die Zahlen 0 bis 9
const byte SEGMENT_MAP[] = { 0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0X80, 0X90 };
const byte SEGMENT_SELECT[] = { 0xF1, 0xF2, 0xF4, 0xF8 };

// Variablen für Stunden, Minuten und Sekunden
byte hours = 0;
byte minutes = 0;
byte seconds = 0;
unsigned long lastMillis = 0;
unsigned long lastButtonPress = 0;

bool showHoursMinutes = true;

// Debounce-Objekte für die Taster
Bounce debouncer1 = Bounce();
Bounce debouncer2 = Bounce();
Bounce debouncer3 = Bounce();

void writeNumberToSegment(byte segment, byte value) {
  // Daten an die 7-Segment-Anzeige schreiben
  digitalWrite(LATCH_DIO, LOW);
  shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, SEGMENT_MAP[value]);
  shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, SEGMENT_SELECT[segment]);
  digitalWrite(LATCH_DIO, HIGH);
}

void setup() {
  // Pin-Modi setzen
  pinMode(LATCH_DIO, OUTPUT);
  pinMode(CLK_DIO, OUTPUT);
  pinMode(DATA_DIO, OUTPUT);

  pinMode(Taster1, INPUT_PULLUP);
  pinMode(Taster2, INPUT_PULLUP);
  pinMode(Taster3, INPUT_PULLUP);

  // Debounce-Objekte initialisieren
  debouncer1.attach(Taster1);
  debouncer1.interval(5);
  debouncer2.attach(Taster2);
  debouncer2.interval(5);
  debouncer3.attach(Taster3);
  debouncer3.interval(5);
}

void loop() {
  // Zustand der Taster aktualisieren
  debouncer1.update();
  debouncer2.update();
  debouncer3.update();

  // Bei Tastendruck oder -haltung
  if (debouncer1.fell() || (debouncer1.read() == LOW && millis() - lastButtonPress > 200)) {
    hours = (hours + 1) % 24;  // Stunden inkrementieren und nach 24 zurücksetzen
    lastButtonPress = millis();
  }
  if (debouncer2.fell() || (debouncer2.read() == LOW && millis() - lastButtonPress > 200)) {
    minutes = (minutes + 1) % 60;  // Minuten inkrementieren und nach 60 zurücksetzen
    lastButtonPress = millis();
  }
  if (debouncer3.fell()) {
    showHoursMinutes = !showHoursMinutes;  // Zwischen HH:MM und MM:SS wechseln
  }
  if (debouncer3.read() == LOW && millis() - lastButtonPress > 3000) {
    hours = 0;    // Stunden zurücksetzen
    minutes = 0;  // Minuten zurücksetzen
    seconds = 0;  // Sekunden zurücksetzen
    lastButtonPress = millis();
  }

  // Sekunden inkrementieren
  if (millis() - lastMillis >= 1000) {
    seconds = (seconds + 1) % 60;
    if (seconds == 0) {  // Minuten alle 60 Sekunden erhöhen
      minutes = (minutes + 1) % 60;
      if (minutes == 0) {  // Stunden alle 60 Minuten erhöhen
        hours = (hours + 1) % 24;
      }
    }
    lastMillis = millis();
  }

  if (showHoursMinutes) {
    // Stunden und Minuten anzeigen
    writeNumberToSegment(0, hours / 10);
    writeNumberToSegment(1, hours % 10);
    writeNumberToSegment(2, minutes / 10);
    writeNumberToSegment(3, minutes % 10);
  } else {
    // Minuten und Sekunden anzeigen
    writeNumberToSegment(0, minutes / 10);
    writeNumberToSegment(1, minutes % 10);
    writeNumberToSegment(2, seconds / 10);
    writeNumberToSegment(3, seconds % 10);
  }
}