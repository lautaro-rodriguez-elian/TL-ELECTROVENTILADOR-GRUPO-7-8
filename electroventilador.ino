// ===============================================
// Trabajo Práctico: Control SI-NO
// Alumno: Rodriguez Lautaro , Tomas Muraoka , Juan Leroux , Hugo Marchesin 
// Materia: Sistema de control automatico
// Fecha: 26/4/26

// Descripción: Este programa implementa un sistema de control de velocidad basado en un controlador sí-no (on/off). 
//El sistema mide la velocidad de un motor mediante un sensor conectado a una interrupción y calcula las RPM en intervalos de 
//tiempo definidos. A partir de esta medición y de un valor de referencia (objetivo) determinado por un potenciómetro, el controlador 
//decide encender o apagar el actuador.
//El muestreo se realiza de forma periódica utilizando la función millis(), evitando el uso de retardos bloqueantes. En cada instante de muestreo, 
//el sistema envía por el puerto serie el tiempo de la muestra y el estado de la acción de control, facilitando el registro y análisis de los datos.
//Además, se implementa una parada de emergencia mediante un pulsador. Al activarse, el sistema apaga el actuador, desactiva las interrupciones, envía 
//un mensaje por el puerto serie y queda en un estado seguro de ejecución infinita con un LED encendido.

//El programa está estructurado en funciones independientes para mejorar la claridad, mantenimiento y reutilización del código.
// Control de velocidad con controlador SI-NO.
// Incluye parada de emergencia, muestreo periódico
// y envío de datos por puerto serie.
// ===============================================

#include "controlSiNo_sca.h"

// --------- PINES ---------
#define PIN_SENSOR 2
#define PIN_MOTOR  5
#define PIN_POT    A0
#define PIN_BOTON  7
#define PIN_LED    13

// --------- PARÁMETROS ---------
#define DELTA_T 500      // ms (corregido)
#define HISTERESIS 100

// --------- OBJETO CONTROL ---------
controlSiNo PLANTA(PIN_MOTOR);

// --------- VARIABLES ---------
volatile int contador = 0;

int rpm = 0;
int objetivo = 0;

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
void contar();

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

  // Leer potenciómetro y actualizar objetivo (incluye 0 RPM)
  int valor = analogRead(PIN_POT);

  if (valor < 256) {
    objetivo = 0;
  }
  else if (valor < 512) {
    objetivo = 300;
  }
  else if (valor < 768) {
    objetivo = 600;
  }
  else {
    objetivo = 1000;
  }

  PLANTA.Configurar(objetivo); // mantiene histéresis

  // Calcular RPM (baja precisión intencional)
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
  Serial.println(accion ? "ON" : "OFF");
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
    digitalWrite(PIN_LED, HIGH); // estado seguro
  }
}

//----------------------------------

void imprimir_encabezado() {

  Serial.println("=== CONTROL SI-NO ===");
  Serial.print("Histeresis: ");
  Serial.println(HISTERESIS);
  Serial.println("Tiempo(ms)\tACCION");
}

//----------------------------------

// --------- INTERRUPCIÓN ---------
void contar() {
  contador++;
}
