#include <Wire.h>
#include <math.h>


// ===== CONFIGURACIÓN GENERAL =====
const bool MODO_DEBUG = false;


// ===== PINES Y MPU6050 =====
const int PIN_PULGAR  = 34;
const int PIN_INDICE  = 35;
const int PIN_CORAZON = 32;
const int PIN_ANULAR  = 25;
const int PIN_MENIQUE = 33;

const int NUM_DEDOS = 5;

const int pines[NUM_DEDOS] = {
  PIN_PULGAR,
  PIN_INDICE,
  PIN_CORAZON,
  PIN_ANULAR,
  PIN_MENIQUE
};

const char* nombres[NUM_DEDOS] = {
  "Pulgar",
  "Indice",
  "Corazon",
  "Anular",
  "Menique"
};

const int SDA_MPU = 22;
const int SCL_MPU = 21;
const int MPU_ADDR = 0x68;


// ===== FILTROS Y RANGOS =====
const int MAX_VENTANA = 25;

const int ventanaPorDedo[NUM_DEDOS] = {
  10,
  10,
  25,
  25,
  25
};

const int NUM_MUESTRAS_MEDIANA = 5;
const int ADC_MIN_VALIDO = 150;
const int DELAY_LOOP = 50;

const float ZONA_MUERTA_FINAL = 0.5;

float adcReposoIndice = 0.0;
float flexIndiceFiltrada = 0.0;

float adcIndiceEstable = 4095.0;
float adcCandidatoIndice = 4095.0;

bool indiceActivo = false;

int contadorBajadaIndice = 0;
int contadorReposoIndice = 0;
int contadorSaltoIndice = 0;

const float INDICE_ADC_REPOSO   = 3900.0;
const float INDICE_ADC_POCA     = 3000.0;
const float INDICE_ADC_MEDIA    = 1700.0;
const float INDICE_ADC_FUERTE   = 950.0;
const float INDICE_ADC_COMPLETA = 250.0;

const float INDICE_ADC_PICO_ALTO = 4090.0;
const float INDICE_SALTO_BRUSCO_ADC = 850.0;
const float INDICE_CANDIDATO_CERCANO_ADC = 220.0;
const int   INDICE_CONFIRMAR_SALTO = 3;

const int INDICE_CONFIRMAR_REPOSO = 8;

const float INDICE_ALPHA_SUBE = 0.55;
const float INDICE_ALPHA_BAJA = 0.16;

const float INDICE_BAJON_TOLERANCIA = 9.0;
const int   INDICE_CONFIRMAR_BAJADA = 3;
const float INDICE_MAX_BAJA_POR_CICLO = 3.5;

const float CURVA_INDICE  = 1.2;
const float CURVA_CORAZON = 1.2;
const float CURVA_ANULAR  = 1.2;
const float CURVA_MENIQUE = 1.2;

const float MINIMO_VISIBLE_INDICE  = 1.0;
const float MINIMO_VISIBLE_CORAZON = 1.0;
const float MINIMO_VISIBLE_ANULAR  = 1.0;
const float MINIMO_VISIBLE_MENIQUE = 1.0;

const float PULGAR_RECTO   = 3954.7;
const float PULGAR_POCO    = 3541.0;
const float PULGAR_MEDIO   = 3127.4;
const float PULGAR_FUERTE  = 2713.7;
const float PULGAR_DOBLADO = 2300.0;
const bool  PULGAR_BAJA_AL_DOBLAR = true;

const float PULGAR_ZONA_REPOSO_ADC = 8.0;

const float INDICE_RECTO   = 1563.5;
const float INDICE_POCO    = 2196.4;
const float INDICE_MEDIO   = 2829.3;
const float INDICE_FUERTE  = 3462.1;
const float INDICE_DOBLADO = 4095.0;
const bool  INDICE_BAJA_AL_DOBLAR = false;

const float INDICE_ZONA_REPOSO_ADC = 8.0;

const float CORAZON_RECTO   = 4095.0;
const float CORAZON_POCO    = 3811.5;
const float CORAZON_MEDIO   = 3528.0;
const float CORAZON_FUERTE  = 3244.4;
const float CORAZON_DOBLADO = 2960.9;
const bool  CORAZON_BAJA_AL_DOBLAR = true;

const float ANULAR_RECTO   = 4095.0;
const float ANULAR_POCO    = 3628.6;
const float ANULAR_MEDIO   = 3162.2;
const float ANULAR_FUERTE  = 2695.8;
const float ANULAR_DOBLADO = 2229.4;
const bool  ANULAR_BAJA_AL_DOBLAR = true;

const float MENIQUE_RECTO   = 4095.0;
const float MENIQUE_POCO    = 3540.0;
const float MENIQUE_MEDIO   = 2984.9;
const float MENIQUE_FUERTE  = 2429.9;
const float MENIQUE_DOBLADO = 1874.8;
const bool  MENIQUE_BAJA_AL_DOBLAR = true;

const float CORAZON_REPOSO_REAL_FINAL = 0.5;
const float CORAZON_MAX_REAL_FINAL    = 78.4;

const float ANULAR_REPOSO_REAL_FINAL = 0.5;
const float ANULAR_MAX_REAL_FINAL    = 100.0;

const float MENIQUE_REPOSO_REAL_FINAL = 0.5;
const float MENIQUE_MAX_REAL_FINAL    = 100.0;


// ===== VARIABLES GLOBALES =====
int bufferADC[NUM_DEDOS][MAX_VENTANA];
long sumaADC[NUM_DEDOS] = {0, 0, 0, 0, 0};
int indiceBuffer[NUM_DEDOS] = {0, 0, 0, 0, 0};

float adcFiltrado[NUM_DEDOS] = {0, 0, 0, 0, 0};

int ultimaLecturaValida[NUM_DEDOS] = {
  4095,
  4095,
  4095,
  4095,
  4095
};

float ax_g = 0, ay_g = 0, az_g = 0;
float gx_dps = 0, gy_dps = 0, gz_dps = 0;

float gyro_offset_x = 0;
float gyro_offset_y = 0;
float gyro_offset_z = 0;

float ax_filtrado = 0, ay_filtrado = 0, az_filtrado = 1.0;
float gx_filtrado = 0, gy_filtrado = 0, gz_filtrado = 0;

const float alpha_accel = 0.15;
const float alpha_gyro  = 0.10;


// ===== FUNCIONES AUXILIARES =====
float limitar(float valor, float minimo, float maximo) {
  if (valor < minimo) return minimo;
  if (valor > maximo) return maximo;
  return valor;
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  if (fabs(in_max - in_min) < 0.1) return out_min;
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float valorDefaultDedo(int dedo) {
  if (dedo == 0) return PULGAR_RECTO;
  if (dedo == 1) return INDICE_DOBLADO;
  if (dedo == 2) return CORAZON_RECTO;
  if (dedo == 3) return ANULAR_RECTO;
  if (dedo == 4) return MENIQUE_RECTO;
  return 4095.0;
}


// ===== LECTURA Y FILTRADO ADC =====
int leerADCMedianaPin(int pin) {
  int lecturas[NUM_MUESTRAS_MEDIANA];

  for (int i = 0; i < NUM_MUESTRAS_MEDIANA; i++) {
    lecturas[i] = analogRead(pin);
    delayMicroseconds(200);
  }

  for (int i = 0; i < NUM_MUESTRAS_MEDIANA - 1; i++) {
    for (int j = i + 1; j < NUM_MUESTRAS_MEDIANA; j++) {
      if (lecturas[j] < lecturas[i]) {
        int temp = lecturas[i];
        lecturas[i] = lecturas[j];
        lecturas[j] = temp;
      }
    }
  }

  return lecturas[NUM_MUESTRAS_MEDIANA / 2];
}

int leerADCMedianaDedo(int dedo) {
  return leerADCMedianaPin(pines[dedo]);
}

int leerADCValido(int dedo) {
  int lectura = leerADCMedianaDedo(dedo);

  if (dedo != 1 && lectura < ADC_MIN_VALIDO) {
    return ultimaLecturaValida[dedo];
  }

  ultimaLecturaValida[dedo] = lectura;
  return lectura;
}

void inicializarMediaMovil() {
  for (int dedo = 0; dedo < NUM_DEDOS; dedo++) {
    int lecturaInicial = leerADCMedianaDedo(dedo);

    if (dedo != 1 && lecturaInicial < ADC_MIN_VALIDO) {
      lecturaInicial = round(valorDefaultDedo(dedo));
    }

    ultimaLecturaValida[dedo] = lecturaInicial;
    sumaADC[dedo] = 0;
    indiceBuffer[dedo] = 0;

    int ventana = ventanaPorDedo[dedo];

    for (int i = 0; i < ventana; i++) {
      bufferADC[dedo][i] = lecturaInicial;
      sumaADC[dedo] += lecturaInicial;
    }

    adcFiltrado[dedo] = lecturaInicial;
  }
}

void actualizarMediaMovil() {
  for (int dedo = 0; dedo < NUM_DEDOS; dedo++) {
    int nuevaLectura = leerADCValido(dedo);
    int ventana = ventanaPorDedo[dedo];

    sumaADC[dedo] -= bufferADC[dedo][indiceBuffer[dedo]];
    bufferADC[dedo][indiceBuffer[dedo]] = nuevaLectura;
    sumaADC[dedo] += nuevaLectura;

    adcFiltrado[dedo] = (float)sumaADC[dedo] / ventana;

    indiceBuffer[dedo]++;

    if (indiceBuffer[dedo] >= ventana) {
      indiceBuffer[dedo] = 0;
    }
  }
}


// ===== CALIBRACIÓN DEL ÍNDICE =====
void calibrarReposoIndice() {
  const int muestras = 120;
  float lecturas[muestras];

  int n = 0;

  for (int i = 0; i < muestras; i++) {
    int v = leerADCMedianaDedo(1);

    if (v >= ADC_MIN_VALIDO) {
      lecturas[n] = v;
      n++;
    }

    delay(5);
  }

  if (n > 0) {
    for (int i = 0; i < n - 1; i++) {
      for (int j = i + 1; j < n; j++) {
        if (lecturas[j] < lecturas[i]) {
          float temp = lecturas[i];
          lecturas[i] = lecturas[j];
          lecturas[j] = temp;
        }
      }
    }

    adcReposoIndice = lecturas[n / 2];
  } else {
    adcReposoIndice = INDICE_ADC_REPOSO;
  }

  flexIndiceFiltrada = 0.0;
  adcIndiceEstable = adcReposoIndice;
  adcCandidatoIndice = adcReposoIndice;

  indiceActivo = false;

  contadorBajadaIndice = 0;
  contadorReposoIndice = 0;
  contadorSaltoIndice = 0;

  if (MODO_DEBUG) {
    Serial.println();
    Serial.print("Reposo indice ADC calibrado: ");
    Serial.println(adcReposoIndice, 1);
  }
}


// ===== CONVERSIÓN ADC A FLEXIÓN =====
float adcAFlexion100Direccion(
  float adc,
  float recto,
  float poco,
  float medio,
  float fuerte,
  float doblado,
  bool bajaAlDoblar,
  float zonaReposoADC
) {
  float salida = 0.0;

  if (bajaAlDoblar) {
    if (adc >= recto - zonaReposoADC) {
      salida = 0.0;
    }
    else if (adc >= poco) {
      salida = mapFloat(adc, recto, poco, 0.0, 25.0);
    }
    else if (adc >= medio) {
      salida = mapFloat(adc, poco, medio, 25.0, 50.0);
    }
    else if (adc >= fuerte) {
      salida = mapFloat(adc, medio, fuerte, 50.0, 75.0);
    }
    else if (adc >= doblado) {
      salida = mapFloat(adc, fuerte, doblado, 75.0, 100.0);
    }
    else {
      salida = 100.0;
    }
  }
  else {
    if (adc <= recto + zonaReposoADC) {
      salida = 0.0;
    }
    else if (adc <= poco) {
      salida = mapFloat(adc, recto, poco, 0.0, 25.0);
    }
    else if (adc <= medio) {
      salida = mapFloat(adc, poco, medio, 25.0, 50.0);
    }
    else if (adc <= fuerte) {
      salida = mapFloat(adc, medio, fuerte, 50.0, 75.0);
    }
    else if (adc <= doblado) {
      salida = mapFloat(adc, fuerte, doblado, 75.0, 100.0);
    }
    else {
      salida = 100.0;
    }
  }

  salida = limitar(salida, 0.0, 100.0);

  if (salida < ZONA_MUERTA_FINAL) {
    salida = 0.0;
  }

  return salida;
}


// ===== FILTRO ESPECIAL DEL ÍNDICE =====
float filtrarPicosADCIndice(float adcActual) {
  float salto = fabs(adcActual - adcIndiceEstable);

  if (
    adcActual >= INDICE_ADC_PICO_ALTO &&
    adcIndiceEstable < INDICE_ADC_REPOSO - 300.0 &&
    salto > INDICE_SALTO_BRUSCO_ADC
  ) {
    if (fabs(adcActual - adcCandidatoIndice) < INDICE_CANDIDATO_CERCANO_ADC) {
      contadorSaltoIndice++;
    } else {
      adcCandidatoIndice = adcActual;
      contadorSaltoIndice = 1;
    }

    if (contadorSaltoIndice < INDICE_CONFIRMAR_SALTO) {
      return adcIndiceEstable;
    }
  }

  if (salto > INDICE_SALTO_BRUSCO_ADC) {
    if (fabs(adcActual - adcCandidatoIndice) < INDICE_CANDIDATO_CERCANO_ADC) {
      contadorSaltoIndice++;
    } else {
      adcCandidatoIndice = adcActual;
      contadorSaltoIndice = 1;
    }

    if (contadorSaltoIndice < INDICE_CONFIRMAR_SALTO) {
      return adcIndiceEstable;
    }
  } else {
    contadorSaltoIndice = 0;
    adcCandidatoIndice = adcActual;
  }

  adcIndiceEstable = adcActual;
  return adcIndiceEstable;
}

float calcularIndicePorZonasADC(float adcIndice) {
  float flex = 0.0;

  if (adcIndice >= INDICE_ADC_REPOSO) {
    flex = 0.0;
  }

  else if (adcIndice >= INDICE_ADC_POCA) {
    flex = mapFloat(
      adcIndice,
      INDICE_ADC_REPOSO,
      INDICE_ADC_POCA,
      0.0,
      25.0
    );
  }

  else if (adcIndice >= INDICE_ADC_MEDIA) {
    flex = mapFloat(
      adcIndice,
      INDICE_ADC_POCA,
      INDICE_ADC_MEDIA,
      25.0,
      55.0
    );
  }

  else if (adcIndice >= INDICE_ADC_FUERTE) {
    flex = mapFloat(
      adcIndice,
      INDICE_ADC_MEDIA,
      INDICE_ADC_FUERTE,
      55.0,
      82.0
    );
  }

  else if (adcIndice >= INDICE_ADC_COMPLETA) {
    flex = mapFloat(
      adcIndice,
      INDICE_ADC_FUERTE,
      INDICE_ADC_COMPLETA,
      82.0,
      100.0
    );
  }

  else {
    flex = 100.0;
  }

  return limitar(flex, 0.0, 100.0);
}

float calcularIndiceFlexibleDesdeADC(float adcIndiceActual) {

  float adcLimpio = filtrarPicosADCIndice(adcIndiceActual);

  if (adcLimpio >= INDICE_ADC_REPOSO) {
    contadorReposoIndice++;
  } else {
    contadorReposoIndice = 0;
    indiceActivo = true;
  }

  if (contadorReposoIndice >= INDICE_CONFIRMAR_REPOSO) {
    indiceActivo = false;
    flexIndiceFiltrada = 0.0;
    contadorBajadaIndice = 0;
    return 0.0;
  }

  float objetivo = calcularIndicePorZonasADC(adcLimpio);

  bool bajonBrusco = objetivo < (flexIndiceFiltrada - INDICE_BAJON_TOLERANCIA);

  if (bajonBrusco) {
    contadorBajadaIndice++;

    if (contadorBajadaIndice < INDICE_CONFIRMAR_BAJADA) {
      objetivo = flexIndiceFiltrada;
    } else {
      float minimoPermitido = flexIndiceFiltrada - INDICE_MAX_BAJA_POR_CICLO;

      if (objetivo < minimoPermitido) {
        objetivo = minimoPermitido;
      }
    }
  } else {
    contadorBajadaIndice = 0;
  }

  float alpha = INDICE_ALPHA_SUBE;

  if (objetivo < flexIndiceFiltrada) {
    alpha = INDICE_ALPHA_BAJA;
  }

  flexIndiceFiltrada =
    (alpha * objetivo) + ((1.0 - alpha) * flexIndiceFiltrada);

  if (flexIndiceFiltrada < 0.7 && contadorReposoIndice >= INDICE_CONFIRMAR_REPOSO) {
    flexIndiceFiltrada = 0.0;
  }

  return limitar(flexIndiceFiltrada, 0.0, 100.0);
}


// ===== CURVAS DE SENSIBILIDAD =====
float aplicarCurvaSensibilidad(float flex, float curva, float minimoVisible) {
  flex = limitar(flex, 0.0, 100.0);

  if (flex < 0.4) {
    return 0.0;
  }

  float corregido = 0.0;

  if (flex < 20.0) {
    corregido = mapFloat(flex, 0.4, 20.0, 1.0, 25.0);
  }
  else if (flex < 60.0) {
    corregido = mapFloat(flex, 20.0, 60.0, 25.0, 65.0);
  }
  else {
    float normalizado = mapFloat(flex, 60.0, 100.0, 0.0, 1.0);
    normalizado = limitar(normalizado, 0.0, 1.0);

    corregido = 65.0 + pow(normalizado, curva) * 35.0;
  }

  corregido = limitar(corregido, 0.0, 100.0);

  if (corregido > 0.0 && corregido < minimoVisible) {
    corregido = minimoVisible;
  }

  return corregido;
}

float aplicarCurvaPulgar(float flex) {
  flex = limitar(flex, 0.0, 100.0);

  if (flex < 0.15) {
    return 0.0;
  }

  float corregido = 0.0;

  if (flex < 8.0) {
    corregido = mapFloat(flex, 0.15, 8.0, 1.0, 22.0);
  }
  else if (flex < 30.0) {
    corregido = mapFloat(flex, 8.0, 30.0, 22.0, 50.0);
  }
  else {
    corregido = mapFloat(flex, 30.0, 100.0, 50.0, 100.0);
  }

  return limitar(corregido, 0.0, 100.0);
}

float corregirFlexionFinal(float valor, float reposoReal, float maximoReal) {
  if (valor <= reposoReal) {
    return 0.0;
  }

  if (fabs(maximoReal - reposoReal) < 0.1) {
    return limitar(valor, 0.0, 100.0);
  }

  float corregido = (valor - reposoReal) * 100.0 / (maximoReal - reposoReal);
  corregido = limitar(corregido, 0.0, 100.0);

  if (corregido < ZONA_MUERTA_FINAL) {
    corregido = 0.0;
  }

  return corregido;
}


// ===== MPU6050 =====
void iniciarMPU6050() {
  Wire.begin(SDA_MPU, SCL_MPU);
  delay(100);

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x80);
  Wire.endTransmission(true);
  delay(100);

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x01);
  Wire.endTransmission(true);
  delay(100);

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1A);
  Wire.write(0x05);
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1C);
  Wire.write(0x00);
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1B);
  Wire.write(0x00);
  Wire.endTransmission(true);
}

void calibrarGiroscopio() {
  float suma_x = 0;
  float suma_y = 0;
  float suma_z = 0;

  const int muestras = 500;

  for (int i = 0; i < muestras; i++) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 6, true);

    if (Wire.available() == 6) {
      int16_t gx = Wire.read() << 8 | Wire.read();
      int16_t gy = Wire.read() << 8 | Wire.read();
      int16_t gz = Wire.read() << 8 | Wire.read();

      suma_x += gx / 131.0;
      suma_y += gy / 131.0;
      suma_z += gz / 131.0;
    }

    delay(3);
  }

  gyro_offset_x = suma_x / muestras;
  gyro_offset_y = suma_y / muestras;
  gyro_offset_z = suma_z / muestras;
}

void leerMPU6050() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);

  if (Wire.endTransmission(false) != 0) {
    return;
  }

  Wire.requestFrom(MPU_ADDR, 14, true);

  if (Wire.available() == 14) {
    int16_t ax = Wire.read() << 8 | Wire.read();
    int16_t ay = Wire.read() << 8 | Wire.read();
    int16_t az = Wire.read() << 8 | Wire.read();

    Wire.read();
    Wire.read();

    int16_t gx = Wire.read() << 8 | Wire.read();
    int16_t gy = Wire.read() << 8 | Wire.read();
    int16_t gz = Wire.read() << 8 | Wire.read();

    ax_g = ax / 16384.0;
    ay_g = ay / 16384.0;
    az_g = az / 16384.0;

    gx_dps = (gx / 131.0) - gyro_offset_x;
    gy_dps = (gy / 131.0) - gyro_offset_y;
    gz_dps = (gz / 131.0) - gyro_offset_z;
  }
}


// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  delay(1000);

  analogReadResolution(12);

  analogSetPinAttenuation(PIN_PULGAR, ADC_11db);
  analogSetPinAttenuation(PIN_INDICE, ADC_11db);
  analogSetPinAttenuation(PIN_CORAZON, ADC_11db);
  analogSetPinAttenuation(PIN_ANULAR, ADC_11db);
  analogSetPinAttenuation(PIN_MENIQUE, ADC_11db);

  inicializarMediaMovil();

  delay(500);
  calibrarReposoIndice();

  iniciarMPU6050();
  calibrarGiroscopio();

  leerMPU6050();

  ax_filtrado = ax_g;
  ay_filtrado = ay_g;
  az_filtrado = az_g;

  gx_filtrado = gx_dps;
  gy_filtrado = gy_dps;
  gz_filtrado = gz_dps;

  if (MODO_DEBUG) {
    Serial.println();
    Serial.println("====================================");
    Serial.println("GUANTE FINAL");
    Serial.println("Indice con 4 zonas ADC + anti-picos");
    Serial.println("Pulgar, corazon, anular y menique igual");
    Serial.println("====================================");
    Serial.println("Enviar 'i' por Monitor Serie recalibra el indice.");
  }
}


// ===== LOOP =====
void loop() {
  actualizarMediaMovil();

  if (Serial.available() > 0) {
    char c = Serial.read();

    if (c == 'i' || c == 'I') {
      calibrarReposoIndice();
    }
  }

  float adcPulgar  = adcFiltrado[0];
  float adcIndice  = adcFiltrado[1];
  float adcCorazon = adcFiltrado[2];
  float adcAnular  = adcFiltrado[3];
  float adcMenique = adcFiltrado[4];

  float flexPulgar = adcAFlexion100Direccion(
    adcPulgar,
    PULGAR_RECTO,
    PULGAR_POCO,
    PULGAR_MEDIO,
    PULGAR_FUERTE,
    PULGAR_DOBLADO,
    PULGAR_BAJA_AL_DOBLAR,
    PULGAR_ZONA_REPOSO_ADC
  );

  float flexIndice = calcularIndiceFlexibleDesdeADC(adcIndice);

  float flexCorazon = adcAFlexion100Direccion(
    adcCorazon,
    CORAZON_RECTO,
    CORAZON_POCO,
    CORAZON_MEDIO,
    CORAZON_FUERTE,
    CORAZON_DOBLADO,
    CORAZON_BAJA_AL_DOBLAR,
    10.0
  );

  float flexAnular = adcAFlexion100Direccion(
    adcAnular,
    ANULAR_RECTO,
    ANULAR_POCO,
    ANULAR_MEDIO,
    ANULAR_FUERTE,
    ANULAR_DOBLADO,
    ANULAR_BAJA_AL_DOBLAR,
    10.0
  );

  float flexMenique = adcAFlexion100Direccion(
    adcMenique,
    MENIQUE_RECTO,
    MENIQUE_POCO,
    MENIQUE_MEDIO,
    MENIQUE_FUERTE,
    MENIQUE_DOBLADO,
    MENIQUE_BAJA_AL_DOBLAR,
    10.0
  );

  flexCorazon = corregirFlexionFinal(
    flexCorazon,
    CORAZON_REPOSO_REAL_FINAL,
    CORAZON_MAX_REAL_FINAL
  );

  flexAnular = corregirFlexionFinal(
    flexAnular,
    ANULAR_REPOSO_REAL_FINAL,
    ANULAR_MAX_REAL_FINAL
  );

  flexMenique = corregirFlexionFinal(
    flexMenique,
    MENIQUE_REPOSO_REAL_FINAL,
    MENIQUE_MAX_REAL_FINAL
  );

  flexPulgar = aplicarCurvaPulgar(flexPulgar);

  flexIndice = limitar(flexIndice, 0.0, 100.0);

  flexCorazon = aplicarCurvaSensibilidad(
    flexCorazon,
    CURVA_CORAZON,
    MINIMO_VISIBLE_CORAZON
  );

  flexAnular = aplicarCurvaSensibilidad(
    flexAnular,
    CURVA_ANULAR,
    MINIMO_VISIBLE_ANULAR
  );

  flexMenique = aplicarCurvaSensibilidad(
    flexMenique,
    CURVA_MENIQUE,
    MINIMO_VISIBLE_MENIQUE
  );

  leerMPU6050();

  ax_filtrado = (alpha_accel * ax_g) + ((1.0 - alpha_accel) * ax_filtrado);
  ay_filtrado = (alpha_accel * ay_g) + ((1.0 - alpha_accel) * ay_filtrado);
  az_filtrado = (alpha_accel * az_g) + ((1.0 - alpha_accel) * az_filtrado);

  gx_filtrado = (alpha_gyro * gx_dps) + ((1.0 - alpha_gyro) * gx_filtrado);
  gy_filtrado = (alpha_gyro * gy_dps) + ((1.0 - alpha_gyro) * gy_filtrado);
  gz_filtrado = (alpha_gyro * gz_dps) + ((1.0 - alpha_gyro) * gz_filtrado);

  if (fabs(gx_filtrado) < 4.0) gx_filtrado = 0.0;
  if (fabs(gy_filtrado) < 4.0) gy_filtrado = 0.0;
  if (fabs(gz_filtrado) < 4.0) gz_filtrado = 0.0;

  if (MODO_DEBUG) {
    Serial.println();
    Serial.println("========== DEBUG GUANTE ==========");

    Serial.print("Pulgar ADC: ");
    Serial.print(adcPulgar, 1);
    Serial.print(" | Flex final: ");
    Serial.println(flexPulgar, 1);

    Serial.print("Indice ADC bruto/media: ");
    Serial.print(adcIndice, 1);
    Serial.print(" | ADC estable: ");
    Serial.print(adcIndiceEstable, 1);
    Serial.print(" | Reposo calibrado: ");
    Serial.print(adcReposoIndice, 1);
    Serial.print(" | Salto count: ");
    Serial.print(contadorSaltoIndice);
    Serial.print(" | Activo: ");
    Serial.print(indiceActivo ? "SI" : "NO");
    Serial.print(" | Flex final: ");
    Serial.println(flexIndice, 1);

    Serial.print("Corazon ADC: ");
    Serial.print(adcCorazon, 1);
    Serial.print(" | Flex final: ");
    Serial.println(flexCorazon, 1);

    Serial.print("Anular ADC: ");
    Serial.print(adcAnular, 1);
    Serial.print(" | Flex final: ");
    Serial.println(flexAnular, 1);

    Serial.print("Menique ADC: ");
    Serial.print(adcMenique, 1);
    Serial.print(" | Flex final: ");
    Serial.println(flexMenique, 1);
  }
  else {
    Serial.print(flexPulgar, 1);   Serial.print(",");
    Serial.print(flexIndice, 1);   Serial.print(",");
    Serial.print(flexCorazon, 1);  Serial.print(",");
    Serial.print(flexAnular, 1);   Serial.print(",");
    Serial.print(flexMenique, 1);  Serial.print(",");

    Serial.print(ax_filtrado, 4);  Serial.print(",");
    Serial.print(ay_filtrado, 4);  Serial.print(",");
    Serial.print(az_filtrado, 4);  Serial.print(",");

    Serial.print(gx_filtrado, 4);  Serial.print(",");
    Serial.print(gy_filtrado, 4);  Serial.print(",");
    Serial.println(gz_filtrado, 4);
  }

  delay(DELAY_LOOP);
}
