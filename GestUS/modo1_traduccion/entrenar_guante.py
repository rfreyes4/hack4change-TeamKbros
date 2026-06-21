import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import MinMaxScaler
from sklearn.neural_network import MLPClassifier
from sklearn.metrics import accuracy_score, classification_report
import warnings
import pickle

# Ignoramos advertencias de sklearn para tener la consola limpia
warnings.filterwarnings('ignore')

# ======================================================
# 1. CARGAR LOS DATOS
# ======================================================
print("Cargando los datos del guante...")
try:
    df = pd.read_csv("FinalCsv.csv") # <-- Pon el nombre exacto de tu archivo aquí
except FileNotFoundError:
    print("❌ ERROR: No se encuentra el archivo 'FinalCsv.csv' en esta carpeta.")
    exit()

# Si la columna donde están tus letras (A, B, C...) se llama de otra forma en tu CSV, 
# cambia la palabra "letra" aquí abajo por el nombre que tenga en la primera fila de tu Excel.
COLUMNA_OBJETIVO = "Letra" 

try:
    X = df.drop(COLUMNA_OBJETIVO, axis=1)
    y = df[COLUMNA_OBJETIVO]
except KeyError:
    print(f"❌ ERROR: No encuentro la columna '{COLUMNA_OBJETIVO}' en tu CSV.")
    print("Columnas disponibles:", list(df.columns))
    exit()

# ======================================================
# 2. DIVIDIR EN ENTRENAMIENTO Y PRUEBA (80% - 20%)
# ======================================================
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# ======================================================
# 3. NORMALIZAR LOS VALORES
# ======================================================
scaler = MinMaxScaler()
X_train_scaled = scaler.fit_transform(X_train)
X_test_scaled = scaler.transform(X_test)

# ======================================================
# 4. CREAR Y ENTRENAR LA RED NEURONAL
# ======================================================
print("Entrenando la red neuronal... (Esto puede tardar unos segundos dependiendo de los datos)")

# Usamos 64 neuronas en la primera capa y 32 en la segunda. 
# max_iter=2000 asegura que la red tenga tiempo suficiente para aprender todo el abecedario.
red_guante = MLPClassifier(hidden_layer_sizes=(64, 32), activation='relu', max_iter=2000, random_state=42)

red_guante.fit(X_train_scaled, y_train)
print("¡Entrenamiento completado exitosamente!")

# ======================================================
# 5. EVALUACIÓN Y RESULTADOS
# ======================================================
predicciones = red_guante.predict(X_test_scaled)
precision = accuracy_score(y_test, predicciones)

print(f"\n==========================================")
print(f"🎯 PRECISIÓN FINAL DEL GUANTE: {precision * 100:.2f}%")
print(f"==========================================\n")

print("Reporte detallado por letra (Fíjate en la columna 'f1-score'):")
print(classification_report(y_test, predicciones))

# ======================================================
# 6. GUARDAR EL MODELO Y EL ESCALADOR
# ======================================================
print("Guardando el cerebro del guante para el tiempo real...")

# Guardamos la red neuronal
with open("modelo_guante.pkl", "wb") as f:
    pickle.dump(red_guante, f)

# Guardamos el escalador (¡obligatorio, ya que recuerda los límites de los sensores!)
with open("escalador_guante.pkl", "wb") as f:
    pickle.dump(scaler, f)

print("¡Archivos 'modelo_guante.pkl' y 'escalador_guante.pkl' guardados con éxito! 🧠")