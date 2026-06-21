<p align="center">
  <img src="assets/logo_gestus.png" alt="Logo GestUS" width="280"/>
</p>

<h1 align="center">GestUS</h1>

<p align="center">
  Guante sensorial para capturar la flexión de los dedos y reconocer gestos mediante sensores, ESP32, Python e IA.
</p>

---

## Descripción

**GestUS** es un prototipo desarrollado para Hack4Change por el Team Kbros.  
El proyecto consiste en un guante sensorial capaz de leer el movimiento de la mano mediante sensores de flexión y un módulo MPU6050.

El sistema captura los datos del guante, los calibra en una escala común de **0 a 100** por dedo y los utiliza en dos modos principales:

1. **Traducción de gestos a letras** mediante un modelo de IA entrenado con datos del guante.
2. **Visualización web en tiempo real** mediante una mano 3D interactiva.

El objetivo inicial es reconocer letras o gestos concretos, con una posible aplicación futura en accesibilidad, rehabilitación, educación o interfaces de control.

---

## Objetivos del proyecto

- Capturar la flexión de los cinco dedos de una mano.
- Usar un ESP32 como unidad de lectura y envío de datos.
- Filtrar y calibrar las señales de sensores caseros.
- Crear un dataset con muestras etiquetadas por letra.
- Entrenar un modelo de IA para reconocer gestos.
- Visualizar el movimiento del guante en una interfaz web 3D.
- Plantear una versión futura inalámbrica y autónoma.

---

## Hardware utilizado

- ESP32.
- 5 sensores de flexión caseros.
- MPU6050 para acelerómetro y giroscopio.
- Resistencias para divisores de tensión.
- Cables y protoboard/conexiones.
- Guante como soporte físico.
- Ordenador para recibir y procesar los datos.

Los sensores caseros se basan en una pista resistiva de grafito. Al doblarse, cambia la resistencia eléctrica y el ESP32 detecta esa variación mediante una entrada analógica.

---

## Funcionamiento general

```text
Sensores de flexión + MPU6050
          ↓
        ESP32
          ↓
Lectura ADC + filtrado + calibración
          ↓
Datos de flexión 0-100
          ↓
Python
          ↓
IA / Visualización web
```

El ESP32 lee los sensores y envía una línea de datos con este formato:

```text
pulgar,indice,corazon,anular,menique,ax,ay,az,gx,gy,gz
```

Cada dedo se representa con un valor entre:

```text
0   = dedo recto
100 = dedo completamente flexionado
```

---

## Calibración y filtrado

Como los sensores son caseros, sus lecturas pueden variar y tener ruido.  
Por eso se aplican varios pasos de estabilización:

- **Filtro de mediana:** elimina picos puntuales.
- **Media móvil:** suaviza la señal.
- **Zona muerta:** ignora pequeñas variaciones en reposo.
- **Rangos calibrados:** convierten el valor ADC en flexión 0-100.
- **Corrección por dedo:** cada sensor puede tener valores distintos.

Esto permite que todos los sensores se interpreten en una escala común.

---

## Modos del sistema

### Modo 1: Traducción de gestos

Este modo utiliza los valores del guante para reconocer letras entrenadas previamente.

```text
Guante → ESP32 → Python → Modelo IA → Letra reconocida
```

El modelo recibe los valores de flexión de los dedos y los datos del MPU6050.  
Con esos datos predice la letra o gesto correspondiente.

Tecnologías usadas:

- Python
- pandas
- numpy
- scikit-learn
- pyserial
- modelo entrenado con dataset propio

---

### Modo 2: Visualización web 3D

Este modo muestra una mano 3D que se mueve en tiempo real según los valores del guante.

```text
ESP32
  ↓ Puerto serie
Python
  ↓ WebSocket / JSON
Navegador web
  ↓ Three.js
Mano 3D
```

Funcionamiento:

- El ESP32 envía los datos al PC mediante puerto serie.
- Python lee esos datos y actúa como servidor local.
- La web se conecta a Python mediante WebSocket.
- Los datos se envían en formato JSON.
- Three.js mueve las articulaciones de la mano 3D.

Tecnologías usadas:

- Python
- websockets
- pyserial
- HTML
- CSS
- JavaScript
- Three.js

---

## Versión futura sin cables

Actualmente el cable USB cumple dos funciones:

- Alimentar el ESP32.
- Enviar los datos al PC mediante puerto serie.

En una versión inalámbrica, el ESP32 podría alimentarse mediante una batería o power bank y enviar los datos por WiFi.

```text
Modo actual:

ESP32 + guante
      ↓ USB
Python en PC
      ↓ WebSocket
Web 3D


Modo sin cables:

ESP32 + batería
      ↓ WiFi
Python en PC
      ↓ WebSocket
Web 3D
```

En esta versión, solo cambiaría la primera parte del sistema: se sustituye el puerto serie por comunicación WiFi. El procesamiento en Python, la IA y la visualización web podrían mantenerse igual.

---

## Estructura del repositorio

```text
hack4change-TeamKbros/
│
├── GUANTEV2/
│   └── GUANTEV2.ino
│
├── GestUS/
│   ├── gestus_launcher.py
│   ├── modo1_traduccion/
│   └── modo2_web_visualizacion/
│
├── RecogerDatos/
│   ├── datos.py
│   ├── leer.py
│   ├── ordenar.py
│   ├── generar_qr.py
│   └── datasets CSV
│
├── assets/
│   └── logo_gestus.png
│
└── README.md
```

---

## Instalación

Clonar el repositorio:

```bash
git clone https://github.com/rfreyes4/hack4change-TeamKbros.git
cd hack4change-TeamKbros
```

Crear un entorno virtual:

```bash
python3 -m venv .venv
source .venv/bin/activate
```

Instalar dependencias:

```bash
pip install pandas numpy scikit-learn pyserial websockets
```

---

## Uso

### 1. Cargar el código en el ESP32

Abrir el archivo:

```text
GUANTEV2/GUANTEV2.ino
```

Configurar la placa ESP32 en Arduino IDE y subir el programa.

---

### 2. Ejecutar el lanzador principal

Desde la carpeta del proyecto:

```bash
cd GestUS
python3 gestus_launcher.py
```

El lanzador permite seleccionar entre los modos del proyecto:

```text
Modo 1: Traducción de gestos
Modo 2: Visualización web
```

---

### 3. Visualización web

Para el modo web:

1. Conectar el ESP32 al PC.
2. Ejecutar el script Python del modo web.
3. Abrir el archivo `index.html` en el navegador.
4. Ver la mano 3D moverse con los datos del guante.

---

## Dataset

El dataset se genera a partir de las lecturas del guante.  
Cada fila contiene:

```text
Pulgar, Índice, Corazón, Anular, Meñique, AX, AY, AZ, GX, GY, GZ, Letra
```

Los datos se utilizan para entrenar y probar el modelo de reconocimiento de gestos.

---

## Estado actual

- Lectura de cinco sensores de flexión.
- Lectura del MPU6050.
- Calibración de señales a escala 0-100.
- Envío de datos por puerto serie.
- Captura de datos en CSV.
- Reconocimiento de letras entrenadas mediante IA.
- Visualización web de una mano 3D en tiempo real.

---

## Mejoras futuras

- Aumentar el número de letras y gestos reconocidos.
- Mejorar la comodidad del guante.
- Añadir comunicación inalámbrica mediante WiFi.
- Usar batería para hacerlo autónomo.
- Mejorar el dataset con más muestras.
- Integrar una interfaz más completa para usuarios finales.
- Aplicarlo a rehabilitación, educación o accesibilidad.

---

## Equipo

Proyecto desarrollado por **Team Kbros** para **Hack4Change**.

---

## Nota

Este proyecto es un prototipo académico y experimental.  
No pretende sustituir sistemas profesionales de traducción de lengua de signos, sino demostrar una posible solución técnica basada en sensores, procesamiento de datos e inteligencia artificial.
