import serial
import time
from datetime import datetime

# ================= CONFIGURACIÓN =================
PUERTO = "/dev/ttyUSB0"
BAUDIOS = 115200
ARCHIVO_SALIDA = "lecturas_guante.txt"

# Si quieres guardar también hora en cada línea
GUARDAR_FECHA_HORA = True

# ==================================================

try:
    print(f"Abriendo puerto {PUERTO} a {BAUDIOS} baudios...")
    ser = serial.Serial(PUERTO, BAUDIOS, timeout=1)
    time.sleep(2)

    print(f"Guardando datos en: {ARCHIVO_SALIDA}")
    print("Pulsa CTRL + C para parar.\n")

    with open(ARCHIVO_SALIDA, "a", encoding="utf-8") as archivo:
        archivo.write("\n")
        archivo.write("=====================================\n")
        archivo.write(f"Inicio captura: {datetime.now()}\n")
        archivo.write("=====================================\n")

        while True:
            linea = ser.readline().decode("utf-8", errors="ignore").strip()

            if linea:
                print(linea)

                if GUARDAR_FECHA_HORA:
                    fecha = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                    archivo.write(f"{fecha} | {linea}\n")
                else:
                    archivo.write(linea + "\n")

                archivo.flush()

except serial.SerialException as e:
    print("Error abriendo el puerto serie.")
    print(e)
    print()
    print("Comprueba que:")
    print("1. El ESP32 está conectado.")
    print("2. El puerto es /dev/ttyUSB0.")
    print("3. No tienes abierto Arduino Serial Monitor, Serial Plotter o picocom.")

except KeyboardInterrupt:
    print("\nCaptura detenida por el usuario.")

finally:
    try:
        ser.close()
        print("Puerto serie cerrado.")
    except:
        pass