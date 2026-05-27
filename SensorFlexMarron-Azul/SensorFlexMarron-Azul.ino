const int PIN_FLEX = 34;

// --- Configuración Ágil y Ligera ---
float valorFiltrado = 0;
const float alpha = 0.35;       
const int NUM_MUESTRAS = 15;    

// --- Valores de Calibración ---
int valorDedoEstirado = 2485; 
int valorDedoDoblado = 2600;  

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_FLEX, ADC_11db);
  valorFiltrado = analogRead(PIN_FLEX);
}

void loop() {
  long suma = 0;

  // 1. Sobremuestreo rápido
  for (int i = 0; i < NUM_MUESTRAS; i++) {
    suma += analogRead(PIN_FLEX);
    delayMicroseconds(100); 
  }
  float valorBrutoPromedio = (float)suma / NUM_MUESTRAS;

  // 2. Filtro EMA dinámico
  valorFiltrado = alpha * valorBrutoPromedio + (1.0 - alpha) * valorFiltrado;

  // 3. Mapeo directo
  int porcentajeFlexion = map((int)valorFiltrado, valorDedoEstirado, valorDedoDoblado, 0, 100);
  porcentajeFlexion = constrain(porcentajeFlexion, 0, 100); 

  // 4. Zona muerta suavizada 
  if (porcentajeFlexion < 25) {
    porcentajeFlexion = 0;
  }

  // 5. FORMATO CORREGIDO PARA EL SERIAL PLOTTER
  // Ahora sí podrás ver las dos variables por separado con sus nombres correctos
  Serial.print("Valor_Crudo_Filtrado:");
  Serial.print(valorFiltrado);
  Serial.print(","); // Separador fundamental para el Plotter
  Serial.print("Porcentaje_Flexion:");
  Serial.println(porcentajeFlexion); 

  delay(50); 
}