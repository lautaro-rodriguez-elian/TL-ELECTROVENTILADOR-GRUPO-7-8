#include "controlSiNo_sca.h"

// --------- PINES ---------
#define PIN_SENSOR 2
#define PIN_MOTOR  5
#define PIN_POT    A0
#define PIN_BOTON  7
#define PIN_LED    13

// --------- PARÁMETROS ---------
#define DELTA_T 100      // ms
#define HISTERESIS 100

// --------- OBJETO CONTROL ---------
controlSiNo PLANTA(PIN_MOTOR);

// --------- VARIABLES ---------
volatile int contador = 0;

int rpm = 0;
int objetivo = 1000;

unsigned long tiempo_actual = 0;
unsigned long tiempo_anterior = 0;

//----------------------------------
// --------- PROTOTIPOS ---------
bool debo_muestrear();
void medir();
void mostrar_datos(bool accion);
void parada();
bool boton_parada_presionado();
void imprimir_encabezado();

//----------------------------------

void setup() {
  pinMode(PIN_SENSOR, INPUT);
  pinMode(PIN_BOTON, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(PIN_SENSOR), contar, CHANGE);

  Serial.begin(9600);

  // Configurar controlador
  PLANTA.Configurar(objetivo, HISTERESIS, SALIDA_NORMAL);

  imprimir_encabezado();
}

//----------------------------------

void loop() {

  if (boton_parada_presionado()) {
    parada();
  }

  if (debo_muestrear()) {
    medir();

    bool accion = PLANTA.Controlar(rpm);

    mostrar_datos(accion);
  }
}

//----------------------------------
// --------- FUNCIONES ---------

bool debo_muestrear() {
  tiempo_actual = millis();

  if (tiempo_actual - tiempo_anterior >= DELTA_T) {
    tiempo_anterior = tiempo_actual;
    return true;
  }
  return false;
}

//----------------------------------

void medir() {

  // Leer potenciómetro y actualizar objetivo
  int valor = analogRead(PIN_POT);

  if (valor < 341) {
    objetivo = 300;
  }
  else if (valor < 682) {
    objetivo = 600;
  }
  else {
    objetivo = 1000;
  }

  PLANTA.Configurar(objetivo); // actualiza objetivo manteniendo histéresis

  // Calcular RPM (zona crítica)
  noInterrupts();
  int cuentas = contador;
  contador = 0;
  interrupts();

  rpm = cuentas * (15000 / DELTA_T);
}

//----------------------------------

void mostrar_datos(bool accion) {

  Serial.print(tiempo_actual);
  Serial.print("\t");
  Serial.print(rpm);
  Serial.print("\t");
  Serial.print(objetivo);
  Serial.print("\t");
  Serial.println(accion);
}

//----------------------------------

bool boton_parada_presionado() {
  return digitalRead(PIN_BOTON) == LOW;
}

//----------------------------------

void parada() {

  PLANTA.Apagar(); // apagar actuador
  Serial.println("PARADA DE EMERGENCIA");

  detachInterrupt(digitalPinToInterrupt(PIN_SENSOR));

  while (true) {
    digitalWrite(PIN_LED, HIGH); // LED encendido (estado seguro)
  }
}

//----------------------------------

void imprimir_encabezado() {

  Serial.println("=== CONTROL SI-NO ===");
  Serial.print("Histeresis: ");
  Serial.println(HISTERESIS);
  Serial.println("Tiempo\tRPM\tOBJ\tACCION");
}

//----------------------------------

// --------- INTERRUPCIÓN ---------
void contar() {
  contador++;
}
