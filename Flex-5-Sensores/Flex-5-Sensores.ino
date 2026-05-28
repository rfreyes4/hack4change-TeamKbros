const int PIN_AZUL_VERDE = 34;
const int PIN_MARRON_BLANCO = 35;
const int PIN_ROJO_NARANJA = 32;
const int PIN_MORADO_AZUL = 33;
const int PIN_VERDE_AMARILLO = 25;

// ================= SENSOR MORADO/AZUL =================
float filtradoMoradoAzul = 0;
const float alphaMoradoAzul = 0.35;
const int muestrasMoradoAzul = 15;
int estiradoMoradoAzul = 2485;
int dobladoMoradoAzul = 2600;

// ================= SENSOR MARRON/BLANCO =================
float filtradoMarronBlanco = 0;
const float alphaMarronBlanco = 0.2;
const int muestrasMarronBlanco = 20;
int estiradoMarronBlanco = 1823;
int dobladoMarronBlanco = 1950;

// ================= SENSOR ROJO/NARANJA =================
float filtradoRojoNaranja = 0;
const float alphaRojoNaranja = 0.2;
const int muestrasRojoNaranja = 20;
int estiradoRojoNaranja = 1986;
int dobladoRojoNaranja = 2210;

// ================= SENSOR VERDE/AMARILLO =================
float filtradoVerdeAmarillo = 0;
const float alphaVerdeAmarillo = 0.2;
const int muestrasVerdeAmarillo = 20;
int estiradoVerdeAmarillo = 1949;
int dobladoVerdeAmarillo = 2200;

// ================= SENSOR AZUL/VERDE =================
float filtradoAzulVerde = 0;
const float alphaAzulVerde = 0.12;
const int muestrasAzulVerde = 20;
int estiradoAzulVerde = 2400;
int dobladoAzulVerde = 2510;

float leerPromedio(int pin, int numMuestras, int esperaMicrosegundos) {
  long suma = 0;

  for (int i = 0; i < numMuestras; i++) {
    suma += analogRead(pin);
    delayMicroseconds(esperaMicrosegundos);
  }

  return (float)suma / numMuestras;
}

int procesarSensorNormal(int pin, float &valorFiltrado, float alpha, int numMuestras, int esperaMicrosegundos, int valorEstirado, int valorDoblado, int zonaBaja, int zonaAlta, bool usarZonaAlta) {
  float valorBrutoPromedio = leerPromedio(pin, numMuestras, esperaMicrosegundos);

  valorFiltrado = alpha * valorBrutoPromedio + (1.0 - alpha) * valorFiltrado;

  int porcentajeFlexion = map((int)valorFiltrado, valorEstirado, valorDoblado, 0, 100);
  porcentajeFlexion = constrain(porcentajeFlexion, 0, 100);

  if (porcentajeFlexion < zonaBaja) {
    porcentajeFlexion = 0;
  }

  if (usarZonaAlta && porcentajeFlexion > zonaAlta) {
    porcentajeFlexion = 100;
  }

  return porcentajeFlexion;
}

int procesarSensorAzulVerde() {
  float valorBrutoPromedio = leerPromedio(PIN_AZUL_VERDE, muestrasAzulVerde, 300);

  if (valorBrutoPromedio < filtradoAzulVerde) {
    filtradoAzulVerde = valorBrutoPromedio;
  } else if (valorBrutoPromedio <= (estiradoAzulVerde + 30)) {
    filtradoAzulVerde = estiradoAzulVerde;
  } else {
    filtradoAzulVerde = alphaAzulVerde * valorBrutoPromedio + (1.0 - alphaAzulVerde) * filtradoAzulVerde;
  }

  int porcentajeFlexion = map((int)filtradoAzulVerde, estiradoAzulVerde, dobladoAzulVerde, 0, 100);
  porcentajeFlexion = constrain(porcentajeFlexion, 0, 100);

  if (porcentajeFlexion <= 30) {
    porcentajeFlexion = 0;
  }

  if (porcentajeFlexion > 85) {
    porcentajeFlexion = 100;
  }

  return porcentajeFlexion;
}

void setup() {
  Serial.begin(115200);

  analogReadResolution(12);

  analogSetPinAttenuation(PIN_AZUL_VERDE, ADC_11db);
  analogSetPinAttenuation(PIN_MARRON_BLANCO, ADC_11db);
  analogSetPinAttenuation(PIN_ROJO_NARANJA, ADC_11db);
  analogSetPinAttenuation(PIN_MORADO_AZUL, ADC_11db);
  analogSetPinAttenuation(PIN_VERDE_AMARILLO, ADC_11db);

  filtradoAzulVerde = analogRead(PIN_AZUL_VERDE);
  filtradoMarronBlanco = analogRead(PIN_MARRON_BLANCO);
  filtradoRojoNaranja = analogRead(PIN_ROJO_NARANJA);
  filtradoMoradoAzul = analogRead(PIN_MORADO_AZUL);
  filtradoVerdeAmarillo = analogRead(PIN_VERDE_AMARILLO);
}

void loop() {
  int flexAzulVerde = procesarSensorAzulVerde();

  int flexMarronBlanco = procesarSensorNormal(
    PIN_MARRON_BLANCO,
    filtradoMarronBlanco,
    alphaMarronBlanco,
    muestrasMarronBlanco,
    300,
    estiradoMarronBlanco,
    dobladoMarronBlanco,
    20,
    85,
    true
  );

  int flexRojoNaranja = procesarSensorNormal(
    PIN_ROJO_NARANJA,
    filtradoRojoNaranja,
    alphaRojoNaranja,
    muestrasRojoNaranja,
    300,
    estiradoRojoNaranja,
    dobladoRojoNaranja,
    23,
    85,
    true
  );

  int flexMoradoAzul = procesarSensorNormal(
    PIN_MORADO_AZUL,
    filtradoMoradoAzul,
    alphaMoradoAzul,
    muestrasMoradoAzul,
    100,
    estiradoMoradoAzul,
    dobladoMoradoAzul,
    25,
    85,
    false
  );

  int flexVerdeAmarillo = procesarSensorNormal(
    PIN_VERDE_AMARILLO,
    filtradoVerdeAmarillo,
    alphaVerdeAmarillo,
    muestrasVerdeAmarillo,
    300,
    estiradoVerdeAmarillo,
    dobladoVerdeAmarillo,
    20,
    85,
    true
  );

  Serial.print("Min:0");
  Serial.print(",");

  Serial.print("Azul_Verde:");
  Serial.print(flexAzulVerde);
  Serial.print(",");

  Serial.print("Marron_Blanco:");
  Serial.print(flexMarronBlanco);
  Serial.print(",");

  Serial.print("Rojo_Naranja:");
  Serial.print(flexRojoNaranja);
  Serial.print(",");

  Serial.print("Morado_Azul:");
  Serial.print(flexMoradoAzul);
  Serial.print(",");

  Serial.print("Verde_Amarillo:");
  Serial.print(flexVerdeAmarillo);
  Serial.print(",");

  Serial.println("Max:100");

  delay(50);
}