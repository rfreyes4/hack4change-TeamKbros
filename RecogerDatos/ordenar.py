import pandas as pd
from pathlib import Path
import sys

# ==========================
# CONFIGURACIÓN
# ==========================

COLUMNAS_ORIGINALES = [
    "pulgar",
    "indice",
    "corazon",
    "anular",
    "menique",
    "ax",
    "ay",
    "az",
    "gx",
    "gy",
    "gz",
    "letra"
]

COLUMNAS_NUEVAS = [
    "Pulgar (Flexión 0-100)",
    "Índice (Flexión 0-100)",
    "Corazón (Flexión 0-100)",
    "Anular (Flexión 0-100)",
    "Meñique (Flexión 0-100)",
    "AX (Acelerómetro X en Gs)",
    "AY (Acelerómetro Y en Gs)",
    "AZ (Acelerómetro Z en Gs)",
    "GX (Giroscopio X en grados/seg)",
    "GY (Giroscopio Y en grados/seg)",
    "GZ (Giroscopio Z en grados/seg)",
    "Letra"
]


def procesar_csv(ruta_entrada):
    ruta_entrada = Path(ruta_entrada)

    if not ruta_entrada.exists():
        print(f"ERROR: No existe el archivo: {ruta_entrada}")
        return

    if ruta_entrada.suffix.lower() != ".csv":
        print("ERROR: El archivo debe ser .csv")
        return

    df = pd.read_csv(ruta_entrada)

    # Quitar espacios raros en nombres de columnas
    df.columns = df.columns.str.strip()

    # Borrar columna tiempo_pc si existe
    if "tiempo_pc" in df.columns:
        df = df.drop(columns=["tiempo_pc"])

    # Comprobar que existen todas las columnas necesarias
    faltan = [col for col in COLUMNAS_ORIGINALES if col not in df.columns]

    if faltan:
        print("ERROR: Faltan estas columnas en el CSV:")
        for col in faltan:
            print(f" - {col}")
        print("\nColumnas encontradas:")
        print(list(df.columns))
        return

    # Reordenar columnas y poner letra al final
    df = df[COLUMNAS_ORIGINALES]

    # Renombrar columnas con los nombres bonitos
    df.columns = COLUMNAS_NUEVAS

    # Crear archivo de salida
    ruta_salida = ruta_entrada.with_name(ruta_entrada.stem + "_ordenado.csv")

    df.to_csv(ruta_salida, index=False, encoding="utf-8-sig")

    print("CSV procesado correctamente.")
    print(f"Archivo generado: {ruta_salida}")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Uso:")
        print("  python3 ordenar_csv.py archivo.csv")
        print("\nEjemplo:")
        print("  python3 ordenar_csv.py datos_guante.csv")
    else:
        procesar_csv(sys.argv[1])