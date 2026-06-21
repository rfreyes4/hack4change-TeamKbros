import serial
import time
import csv
import sys
from datetime import datetime


# ======================================================
# CONFIGURACIÓN
# ======================================================

# En tu captura Arduino está usando /dev/ttyUSB1
PUERTO = "/dev/ttyUSB0"
BAUDRATE = 115200

ARCHIVO_SALIDA = "datos_guante_columnas_v3.csv"

SEGUNDOS_POR_LETRA = 10

# Poner True si quieres ver líneas descartadas
DEBUG_DESCARTADAS = False


# ======================================================
# SERIAL
# ======================================================

def abrir_serial(puerto, baudrate):
    print(f"Abriendo {puerto} a {baudrate} baudios...")

    ser = serial.Serial(
        port=puerto,
        baudrate=baudrate,
        timeout=1
    )

    # Esperar a que el ESP32 reinicie al abrir el puerto
    time.sleep(2)

    ser.reset_input_buffer()

    print("Puerto abierto correctamente.")
    print()
    return ser


# ======================================================
# PARSEO DE DATOS
# ======================================================

def parsear_linea(linea):
    linea_original = linea.strip()

    if linea_original == "":
        return None

    # Normalizar separadores
    linea_limpia = linea_original.replace("\t", ",")
    linea_limpia = linea_limpia.replace(" ", "")

    partes = linea_limpia.split(",")

    # Esperamos exactamente 11 valores:
    # pulgar,indice,corazon,anular,menique,ax,ay,az,gx,gy,gz
    if len(partes) != 11:
        if DEBUG_DESCARTADAS:
            print("Descartada por columnas:", linea_original)
        return None

    try:
        valores = [float(x) for x in partes]
        return valores
    except ValueError:
        if DEBUG_DESCARTADAS:
            print("Descartada por no numérica:", linea_original)
        return None


# ======================================================
# GRABAR UNA LETRA
# ======================================================

def grabar_letra(ser, writer, archivo, letra, segundos):
    print()
    print("========================================")
    print(f"Letra a grabar: {letra}")
    print("Coloca la mano en la postura de esa letra.")
    print("========================================")
    print()

    for i in range(3, 0, -1):
        print(f"Empieza en {i}...")
        time.sleep(1)

    print()
    print(f"Grabando letra {letra} durante {segundos} segundos...")
    print("Mantén el gesto estable.")
    print()

    # Limpiar datos viejos antes de grabar
    ser.reset_input_buffer()

    inicio = time.time()
    contador = 0

    while time.time() - inicio < segundos:
        try:
            raw = ser.readline().decode("utf-8", errors="ignore")
        except Exception:
            continue

        valores = parsear_linea(raw)

        if valores is None:
            continue

        timestamp = datetime.now().isoformat(timespec="milliseconds")

        fila = [timestamp, letra] + valores

        writer.writerow(fila)
        archivo.flush()

        contador += 1

        # Mostrar la muestra por consola
        print(",".join(map(str, fila)))

    print()
    print(f"Letra {letra} terminada.")
    print(f"Muestras guardadas: {contador}")
    print()


# ======================================================
# PROGRAMA PRINCIPAL
# ======================================================

def main():
    puerto = PUERTO
    baudrate = BAUDRATE
    archivo_salida = ARCHIVO_SALIDA
    segundos = SEGUNDOS_POR_LETRA

    # Permite usar:
    # python3 datos.py /dev/ttyUSB1 115200 datos.csv 10
    if len(sys.argv) >= 2:
        puerto = sys.argv[1]

    if len(sys.argv) >= 3:
        baudrate = int(sys.argv[2])

    if len(sys.argv) >= 4:
        archivo_salida = sys.argv[3]

    if len(sys.argv) >= 5:
        segundos = int(sys.argv[4])

    ser = None

    try:
        ser = abrir_serial(puerto, baudrate)

        with open(archivo_salida, "a", newline="", encoding="utf-8") as archivo:
            writer = csv.writer(archivo)

            # Escribir cabecera solo si el archivo está vacío
            if archivo.tell() == 0:
                writer.writerow([
                    "tiempo_pc",
                    "letra",
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
                    "gz"
                ])
                archivo.flush()

            print("Listo para grabar datos.")
            print()
            print("Escribe una letra y pulsa ENTER.")
            print("Ejemplo: A")
            print("Para salir escribe: 0")
            print()

            while True:
                letra = input("Letra a grabar, o 0 para salir: ").strip().upper()

                if letra == "0":
                    break

                if letra == "":
                    continue

                grabar_letra(ser, writer, archivo, letra, segundos)

    except serial.SerialException as e:
        print()
        print("ERROR abriendo el puerto serie.")
        print(e)
        print()
        print("Soluciones:")
        print("1. Cierra el Monitor Serial de Arduino.")
        print("2. Comprueba si el puerto es /dev/ttyUSB0 o /dev/ttyUSB1.")
        print("3. Ejecuta: ls /dev/ttyUSB*")
        print("4. Prueba: python3 datos.py /dev/ttyUSB1 115200")
        print()

    except KeyboardInterrupt:
        print()
        print("Grabación interrumpida por el usuario.")

    finally:
        if ser is not None and ser.is_open:
            ser.close()

        print()
        print(f"Datos guardados en: {archivo_salida}")


if __name__ == "__main__":
    main()