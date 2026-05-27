const int PIN_FLEX = 34;

// --- Configuración del Filtro ---
float valorFiltrado = 0;
const float alpha = 0.2;       
const int NUM_MUESTRAS = 20;    

// --- Tus Valores Calibrados ---
int valorDedoEstirado = 1949; 
int valorDedoDoblado = 2200;  

void setup() {
  Serial.begin(115200);

  // Configuración del ADC del ESP32
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_FLEX, ADC_11db);
  
  // Lectura inicial para evitar que el filtro empiece en 0
  valorFiltrado = analogRead(PIN_FLEX);
}

void loop() {
  long suma = 0;

  // 1. SOBREMUESTREO: Ráfaga rápida para limpiar el hardware
  for (int i = 0; i < NUM_MUESTRAS; i++) {
    suma += analogRead(PIN_FLEX);
    delayMicroseconds(300); 
  }
  float valorBrutoPromedio = (float)suma / NUM_MUESTRAS;

  // 2. FILTRO EMA: Suavizado digital
  valorFiltrado = alpha * valorBrutoPromedio + (1.0 - alpha) * valorFiltrado;

  // 3. MAPEO A PORCENTAJE (0% a 100%)
  int porcentajeFlexion = map((int)valorFiltrado, valorDedoEstirado, valorDedoDoblado, 0, 100);
  
  // 4. FRENO: Evitamos que se salga del rango matemático
  porcentajeFlexion = constrain(porcentajeFlexion, 0, 100); 

  // 5. SOLUCIÓN A LA HISTÉRESIS (Zonas Muertas)
  // Si el sensor se queda "pillado" intentando volver a la normalidad, lo forzamos.
  // Puedes ajustar ese 12 y ese 88 según veas cómo se comporta tu guante físico.
  if (porcentajeFlexion < 20) {
    porcentajeFlexion = 0;
  }
  if (porcentajeFlexion > 85) {
    porcentajeFlexion = 100;
  }

  // 6. SALIDA AL SERIAL PLOTTER
  Serial.print("Porcentaje_Flexion:");
  Serial.println(porcentajeFlexion); 

  // Delay bajado a 20ms para una lectura súper rápida al traducir
  delay(200); 
}