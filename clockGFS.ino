// Inkludierung von Bounce2
#include <Bounce2.h>

// Definition der Taster-Pins
const byte TASTER_PINS[] = { A1, A2, A3 };
const byte LATCH_DIO = 4;
const byte CLK_DIO = 7;
const byte DATA_DIO = 8;

// Segment-Byte-Mapping für Zahlen von 0 bis 9
const byte SEGMENT_MAP[] = { 0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0X80, 0X90 };
const byte SEGMENT_SELECT[] = { 0xF1, 0xF2, 0xF4, 0xF8 };

// Struktur zur Speicherung der aktuellen Zeit
struct Time {
    byte hours = 0;
    byte minutes = 0;
    byte seconds = 0;
} currentTime;

unsigned long lastMillis = 0;
unsigned long lastButtonPress = 0;
bool showHoursMinutes = true;

// Debounce-Objekte für die Taster
Bounce debouncers[3];

// Funktion zum Schreiben von Zahlen auf das Segment
void writeNumberToSegment(byte segment, byte value) {
    digitalWrite(LATCH_DIO, LOW);
    shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, SEGMENT_MAP[value]);
    shiftOut(DATA_DIO, CLK_DIO, MSBFIRST, SEGMENT_SELECT[segment]);
    digitalWrite(LATCH_DIO, HIGH);
}

// Funktion zur Behandlung von Tastendrücken
void handleButtonPress(Bounce &debouncer, byte &value, byte maxVal) {
    if (debouncer.fell() || (debouncer.read() == LOW && millis() - lastButtonPress > 200)) {
        value = (value + 1) % maxVal;
        lastButtonPress = millis();
    }
}

void setup() {
    // Setzen der Pin-Modi
    pinMode(LATCH_DIO, OUTPUT);
    pinMode(CLK_DIO, OUTPUT);
    pinMode(DATA_DIO, OUTPUT);

    // Initialisierung der Taster und Debounce-Objekte
    for (int i = 0; i < 3; i++) {
        pinMode(TASTER_PINS[i], INPUT_PULLUP);
        debouncers[i].attach(TASTER_PINS[i]);
        debouncers[i].interval(5);
    }
}

void loop() {
    // Aktualisierung des Zustands der Taster
    for (int i = 0; i < 3; i++) {
        debouncers[i].update();
    }

    // Behandlung der Tastendrücke für Stunden und Minuten
    handleButtonPress(debouncers[0], currentTime.hours, 24);
    handleButtonPress(debouncers[1], currentTime.minutes, 60);

    // Umschalten zwischen Stunden:Minuten und Minuten:Sekunden
    if (debouncers[2].fell()) {
        showHoursMinutes = !showHoursMinutes;
    }

    // Zurücksetzen der Zeit, wenn der dritte Taster lange gedrückt wird
    if (debouncers[2].read() == LOW && millis() - lastButtonPress > 3000) {
        currentTime.hours = currentTime.minutes = currentTime.seconds = 0;
        lastButtonPress = millis();
    }

    // Inkrementieren der Sekunden und gegebenenfalls der Minuten und Stunden
    if (millis() - lastMillis >= 1000) {
        currentTime.seconds = (currentTime.seconds + 1) % 60;
        if (currentTime.seconds == 0 && ++currentTime.minutes == 60) {
            currentTime.minutes = 0;
            currentTime.hours = (currentTime.hours + 1) % 24;
        }
        lastMillis = millis();
    }

    // Anzeige der aktuellen Zeit
    byte first = showHoursMinutes ? currentTime.hours : currentTime.minutes;
    byte second = showHoursMinutes ? currentTime.minutes : currentTime.seconds;

    writeNumberToSegment(0, first / 10);
    writeNumberToSegment(1, first % 10);
    writeNumberToSegment(2, second / 10);
    writeNumberToSegment(3, second % 10);
}
