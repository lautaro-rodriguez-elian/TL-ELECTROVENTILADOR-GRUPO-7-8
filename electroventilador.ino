// --------- PINES ---------
const int pinSensor = 2;
const int pinMotor = 5;
const int pinPot = A0;
const int pinBoton = 7;   // <-- botón de emergencia

// --------- VARIABLES ---------
volatile int contador = 0;

int rpm = 0;
int objetivo = 0;

const int deltaT = 100;     // tiempo de muestreo (ms)
const int histeresis = 100;  // margen

unsigned long tiempo = 0;

//----------------------------------

void setup() {
  pinMode(pinSensor, INPUT);
  pinMode(pinMotor, OUTPUT);
  pinMode(pinBoton, INPUT_PULLUP); // <-- botón

  attachInterrupt(digitalPinToInterrupt(pinSensor), contar, CHANGE);

  Serial.begin(9600);
}

//----------------------------------

void loop() {

  // --------- PARADA DE EMERGENCIA ---------
  if (digitalRead(pinBoton) == LOW) {
    parada();
  }

  if (millis() - tiempo >= deltaT) {
    tiempo = millis();

    // --------- LEER POTENCIOMETRO ---------
    int valor = analogRead(pinPot);

    if (valor < 341) {
      objetivo = 300;   // 30%
    }
    else if (valor < 682) {
      objetivo = 600;   // 60%
    }
    else {
      objetivo = 1000;  // 100%
    }

    // --------- CALCULAR RPM ---------
    rpm = contador * (15000 / deltaT);
    contador = 0;

    // --------- CONTROL ON-OFF ---------
    if (rpm < (objetivo - histeresis)) {
      digitalWrite(pinMotor, HIGH); // prender
    }
    else if (rpm > (objetivo + histeresis)) {
      digitalWrite(pinMotor, LOW);  // apagar
    }

    // --------- MONITOR SERIAL ---------
    Serial.print("RPM: ");
    Serial.print(rpm);
    Serial.print(" | OBJ: ");
    Serial.println(objetivo);
  }
}

//----------------------------------

// --------- INTERRUPCIÓN ---------
void contar() {
  contador++;
}

//----------------------------------

// --------- PARADA DE EMERGENCIA ---------
void parada() {

  digitalWrite(pinMotor, LOW); // apagar motor
  Serial.println("PARADA DE EMERGENCIA");

  detachInterrupt(digitalPinToInterrupt(pinSensor)); // desactiva interrupciones

  while (true) {
    // queda bloqueado (estado seguro)
  }
}
