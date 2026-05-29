#include <Wire.h>

// ================= PINES DE SENSORES DE FLEXIÓN =================
const int PIN_AZUL_VERDE = 34;
const int PIN_MARRON_BLANCO = 35;
const int PIN_ROJO_NARANJA = 32;
const int PIN_MORADO_AZUL = 33;
const int PIN_VERDE_AMARILLO = 25;

// ================= PINES Y DIRECCIÓN DEL MPU6050 =================
const int SDA_MPU = 21;
const int SCL_MPU = 22;
const int MPU_ADDR = 0x68;

// ================= VARIABLES MPU6050 (ACELERÓMETRO Y GIROSCOPIO) =================
float ax_g = 0, ay_g = 0, az_g = 0;
float gx_dps = 0, gy_dps = 0, gz_dps = 0;

// Offsets para calibración automática del giroscopio
float gyro_offset_x = 0;
float gyro_offset_y = 0;
float gyro_offset_z = 0;

// Variables para el Filtro Exponencial (EMA) por Software
float ax_filtrado = 0, ay_filtrado = 0, az_filtrado = 1.0;
float gx_filtrado = 0, gy_filtrado = 0, gz_filtrado = 0;

// Factores de suavizado del MPU
const float alpha_accel = 0.15; 
const float alpha_gyro = 0.10;  

// ================= CONFIGURACIÓN SENSOR MORADO/AZUL =================
float filtradoMoradoAzul = 0;
const float alphaMoradoAzul = 0.35;
const int muestrasMoradoAzul = 15;
int estiradoMoradoAzul =2600;
int dobladoMoradoAzul = 2750;

// ================= CONFIGURACIÓN SENSOR MARRON/BLANCO =================
float filtradoMarronBlanco = 0;
const float alphaMarronBlanco = 0.2;
const int muestrasMarronBlanco = 20;
int estiradoMarronBlanco = 1875;
int dobladoMarronBlanco = 2015;

// ================= CONFIGURACIÓN SENSOR ROJO/NARANJA =================
float filtradoRojoNaranja = 0;
const float alphaRojoNaranja = 0.2;
const int muestrasRojoNaranja = 20;
int estiradoRojoNaranja = 1986;
int dobladoRojoNaranja = 2210;

// ================= CONFIGURACIÓN SENSOR VERDE/AMARILLO =================
float filtradoVerdeAmarillo = 0;
const float alphaVerdeAmarillo = 0.2;
const int muestrasVerdeAmarillo = 20;
int estiradoVerdeAmarillo = 1949;
int dobladoVerdeAmarillo = 2200;

// ================= CONFIGURACIÓN SENSOR AZUL/VERDE =================
float filtradoAzulVerde = 0;
const float alphaAzulVerde = 0.12;
const int muestrasAzulVerde = 20;
int estiradoAzulVerde = 2500;
int dobladoAzulVerde = 2620;

// ================= FUNCIONES DE LOS SENSORES DE FLEXIÓN =================
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

  if (porcentajeFlexion <= 20) {
    porcentajeFlexion = 0;
  }
  if (porcentajeFlexion > 85) {
    porcentajeFlexion = 100;
  }
  return porcentajeFlexion;
}

// ================= FUNCIONES SEGURAS DEL MPU6050 =================
void iniciarMPU6050() {
  Wire.begin(SDA_MPU, SCL_MPU);
  delay(100);

  // 1. Reset por software para limpiar registros corruptos
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x80); 
  Wire.endTransmission(true);
  delay(200); // Tiempo extendido de seguridad post-reset

  // 2. Despertar chip y asignar reloj estable (Giroscopio eje X)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x01); 
  Wire.endTransmission(true);
  delay(100);

  // 3. Activar Filtro Pasabajo Interno (DLPF) a ~10Hz por Hardware
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1A);
  Wire.write(0x05); 
  Wire.endTransmission(true);

  // 4. Configurar Acelerómetro (+/- 2g)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1C);
  Wire.write(0x00);
  Wire.endTransmission(true);

  // 5. Configurar Giroscopio (+/- 250 dps)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1B);
  Wire.write(0x00);
  Wire.endTransmission(true);
}

void calibrarGiroscopio() {
  float suma_x = 0, suma_y = 0, suma_z = 0;
  int lecturas_validas = 0;
  const int muestras = 100;

  Serial.println("Calibrando giroscopio... DEJA EL GUANTE INMÓVIL");
  
  for (int i = 0; i < muestras; i++) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x43); // Registro inicial del giroscopio
    
    // Verificación de bus libre para blindar contra colapsos de memoria (Panic Core)
    if (Wire.endTransmission(false) == 0) {
      Wire.requestFrom(MPU_ADDR, 6, true);

      if (Wire.available() == 6) {
        int16_t gx = Wire.read() << 8 | Wire.read();
        int16_t gy = Wire.read() << 8 | Wire.read();
        int16_t gz = Wire.read() << 8 | Wire.read();

        suma_x += (gx / 131.0);
        suma_y += (gy / 131.0);
        suma_z += (gz / 131.0);
        lecturas_validas++;
      }
    }
    delay(20);
  }

  // Carga de offsets calculados de manera segura
  if (lecturas_validas > 0) {
    gyro_offset_x = suma_x / lecturas_validas;
    gyro_offset_y = suma_y / lecturas_validas;
    gyro_offset_z = suma_z / lecturas_validas;
    Serial.println("Calibración completada con éxito.");
  } else {
    Serial.println("Fallo crítico: El MPU6050 no respondió en la calibración.");
  }
}

void leerMPU6050() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // Registro inicial del acelerómetro
  if (Wire.endTransmission(false) != 0) return; // Salida segura si falla el bus

  Wire.requestFrom(MPU_ADDR, 14, true);

  if (Wire.available() == 14) {
    int16_t ax = Wire.read() << 8 | Wire.read();
    int16_t ay = Wire.read() << 8 | Wire.read();
    int16_t az = Wire.read() << 8 | Wire.read();

    int16_t temp = Wire.read() << 8 | Wire.read(); // Ignoramos byte de temperatura

    int16_t gx = Wire.read() << 8 | Wire.read();
    int16_t gy = Wire.read() << 8 | Wire.read();
    int16_t gz = Wire.read() << 8 | Wire.read();

    // Conversión a escala G física
    ax_g = ax / 16384.0;
    ay_g = ay / 16384.0;
    az_g = az / 16384.0;

    // Conversión a grados por segundo aplicando offset
    gx_dps = (gx / 131.0) - gyro_offset_x;
    gy_dps = (gy / 131.0) - gyro_offset_y;
    gz_dps = (gz / 131.0) - gyro_offset_z;
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  // Configuración de los ADC internos del ESP32
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_AZUL_VERDE, ADC_11db);
  analogSetPinAttenuation(PIN_MARRON_BLANCO, ADC_11db);
  analogSetPinAttenuation(PIN_ROJO_NARANJA, ADC_11db);
  analogSetPinAttenuation(PIN_MORADO_AZUL, ADC_11db);
  analogSetPinAttenuation(PIN_VERDE_AMARILLO, ADC_11db);

  // Inicializar hardware y calibrar MPU6050
  iniciarMPU6050();
  calibrarGiroscopio(); 

  // Lectura inicial de estabilización para filtros EMA
  leerMPU6050();
  ax_filtrado = ax_g; ay_filtrado = ay_g; az_filtrado = az_g;
  gx_filtrado = gx_dps; gy_filtrado = gy_dps; gz_filtrado = gz_dps;

  // Carga inicial de datos de los flexores
  filtradoAzulVerde = analogRead(PIN_AZUL_VERDE);
  filtradoMarronBlanco = analogRead(PIN_MARRON_BLANCO);
  filtradoRojoNaranja = analogRead(PIN_ROJO_NARANJA);
  filtradoMoradoAzul = analogRead(PIN_MORADO_AZUL);
  filtradoVerdeAmarillo = analogRead(PIN_VERDE_AMARILLO);
}

// ================= LOOP =================
void loop() {
  // --- 1. Procesamiento de Flexores (0 a 100%) ---
  int flexAzulVerde = procesarSensorAzulVerde();
  int flexMarronBlanco = procesarSensorNormal(PIN_MARRON_BLANCO, filtradoMarronBlanco, alphaMarronBlanco, muestrasMarronBlanco, 300, estiradoMarronBlanco, dobladoMarronBlanco, 20, 85, true);
  int flexRojoNaranja = procesarSensorNormal(PIN_ROJO_NARANJA, filtradoRojoNaranja, alphaRojoNaranja, muestrasRojoNaranja, 300, estiradoRojoNaranja, dobladoRojoNaranja, 23, 85, true);
  int flexMoradoAzul = procesarSensorNormal(PIN_MORADO_AZUL, filtradoMoradoAzul, alphaMoradoAzul, muestrasMoradoAzul, 100, estiradoMoradoAzul, dobladoMoradoAzul, 25, 85, false);
  int flexVerdeAmarillo = procesarSensorNormal(PIN_VERDE_AMARILLO, filtradoVerdeAmarillo, alphaVerdeAmarillo, muestrasVerdeAmarillo, 300, estiradoVerdeAmarillo, dobladoVerdeAmarillo, 20, 85, true);

  // --- 2. Lectura y Filtrado Avanzado MPU6050 ---
  leerMPU6050();

  // Suavizado del Acelerómetro (EMA)
  ax_filtrado = (alpha_accel * ax_g) + (1.0 - alpha_accel) * ax_filtrado;
  ay_filtrado = (alpha_accel * ay_g) + (1.0 - alpha_accel) * ay_filtrado;
  az_filtrado = (alpha_accel * az_g) + (1.0 - alpha_accel) * az_filtrado;

  // Suavizado del Giroscopio (EMA)
  gx_filtrado = (alpha_gyro * gx_dps) + (1.0 - alpha_gyro) * gx_filtrado;
  gy_filtrado = (alpha_gyro * gy_dps) + (1.0 - alpha_gyro) * gy_filtrado;
  gz_filtrado = (alpha_gyro * gz_dps) + (1.0 - alpha_gyro) * gz_filtrado;

  // Umbral de zona muerta: corta el micro-ruido sobrante del Giroscopio en reposo
  if (abs(gx_filtrado) < 0.15) gx_filtrado = 0.0;
  if (abs(gy_filtrado) < 0.15) gy_filtrado = 0.0;
  if (abs(gz_filtrado) < 0.15) gz_filtrado = 0.0;

  // --- 3. Salida Unificada para el Serial Plotter ---
  Serial.print("Min:0"); Serial.print(",");

  Serial.print("Azul_Verde:"); Serial.print(flexAzulVerde); Serial.print(",");
  Serial.print("Marron_Blanco:"); Serial.print(flexMarronBlanco); Serial.print(",");
  Serial.print("Rojo_Naranja:"); Serial.print(flexRojoNaranja); Serial.print(",");
  Serial.print("Morado_Azul:"); Serial.print(flexMoradoAzul); Serial.print(",");
  Serial.print("Verde_Amarillo:"); Serial.print(flexVerdeAmarillo); Serial.print(",");

  // Valores ultra-limpios procesados del acelerómetro y giroscopio
  /*Serial.print("AX:"); Serial.print(ax_filtrado); Serial.print(",");
  Serial.print("AY:"); Serial.print(ay_filtrado); Serial.print(",");
  Serial.print("AZ:"); Serial.print(az_filtrado); Serial.print(",");

  Serial.print("GX:"); Serial.print(gx_filtrado); Serial.print(",");
  Serial.print("GY:"); Serial.print(gy_filtrado); Serial.print(",");
  Serial.print("GZ:"); Serial.print(gz_filtrado); Serial.print(",");*/

  Serial.println("Max:100");

  // Muestreo controlado a 20Hz (~50ms) compatible con la adquisición de datos de flexión
  delay(50);
}
