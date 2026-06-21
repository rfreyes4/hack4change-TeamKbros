import asyncio
import json
import websockets
import serial
import csv
import sys
import time

print("🤖 MODO TELEOPERACIÓN EN TIEMPO REAL (PUERTO SERIE DIRECTO) 🤖")

PUERTO_SERIE = '/dev/ttyUSB0'
BAUD_RATE = 115200 
ARCHIVO_SALIDA = "datos_ord.csv"
SEGUNDOS_POR_LETRA = 10

# Mantendremos las variables de los sensores actualizándose en vivo aquí
valores_tiempo_real = {"pulgar": 0, "indice": 0, "corazon": 0, "anular": 0, "menique": 0}

# Variable global para controlar qué ve el 3D
estado_visor = {"modo": "streaming", "dedos_fijos": {}, "letra_fija": ""}

datos_actuales = {
    "tipo": "streaming", 
    "dedos": {"pulgar": 0, "indice": 0, "corazon": 0, "anular": 0, "menique": 0},
    "imu": {"ax": 0, "ay": 0, "az": 0, "gx": 0, "gy": 0, "gz": 0}
}
letra_activa = ""
guante_serial = None
guante_conectado = False

try:
    guante_serial = serial.Serial(PUERTO_SERIE, BAUD_RATE, timeout=0.01)
    time.sleep(2)
    guante_serial.reset_input_buffer()
    guante_conectado = True
    print(f"✅ Guante físico DETECTADO y conectado en {PUERTO_SERIE}")
except Exception as e:
    guante_conectado = False
    print(f"⚠️ Guante NO detectado en {PUERTO_SERIE}. (Modo simulación sin hardware ACTIVO)")
    print("👉 Puedes usar comandos 'mostrar <letra>' o 'pose' para probar el visor 3D.\n")

def calcular_media_letra(letra_objetivo):
    """Busca en el CSV todas las filas de la letra y calcula el promedio de flexión"""
    total_dedos = {"pulgar": 0.0, "indice": 0.0, "corazon": 0.0, "anular": 0.0, "menique": 0.0}
    coincidencias = 0
    
    try:
        with open(ARCHIVO_SALIDA, "r", encoding="utf-8-sig") as f:
            muestra = f.read(2048)
            delimitador = ';' if ';' in muestra else ','
            f.seek(0)
            
            reader = csv.DictReader(f, delimiter=delimitador)
            
            mapeo_columnas = {}
            if reader.fieldnames:
                for col in reader.fieldnames:
                    col_limpia = col.strip().lower()
                    col_limpia = col_limpia.replace("í", "i").replace("ó", "o").replace("ñ", "n")
                    
                    if "pulgar" in col_limpia: mapeo_columnas["pulgar"] = col
                    elif "indice" in col_limpia: mapeo_columnas["indice"] = col
                    elif "corazon" in col_limpia: mapeo_columnas["corazon"] = col
                    elif "anular" in col_limpia: mapeo_columnas["anular"] = col
                    elif "menique" in col_limpia: mapeo_columnas["menique"] = col
                    elif "letra" in col_limpia: mapeo_columnas["letra"] = col
            else:
                return None

            columnas_requeridas = ["pulgar", "indice", "corazon", "anular", "menique", "letra"]
            faltantes = [c for c in columnas_requeridas if c not in mapeo_columnas]
            if faltantes:
                print(f"❌ Error de formato en el CSV. No se identificaron las columnas para: {faltantes}")
                print(f"👉 Tu cabecera real procesada es: {reader.fieldnames}")
                return None

            for fila in reader:
                if fila[mapeo_columnas["letra"]].strip().upper() == letra_objetivo:
                    total_dedos["pulgar"] += float(fila[mapeo_columnas["pulgar"]])
                    total_dedos["indice"] += float(fila[mapeo_columnas["indice"]])
                    total_dedos["corazon"] += float(fila[mapeo_columnas["corazon"]])
                    total_dedos["anular"] += float(fila[mapeo_columnas["anular"]])
                    total_dedos["menique"] += float(fila[mapeo_columnas["menique"]])
                    coincidencias += 1
    except FileNotFoundError:
        print(f"❌ No se encontró el archivo '{ARCHIVO_SALIDA}'")
        return None
    except Exception as e:
        print(f"❌ Error al procesar el CSV: {e}")
        return None

    if coincidencias == 0:
        return None
        
    return {k: v / coincidencias for k, v in total_dedos.items()}

async def transmitir_datos(websocket, path=None):
    """
    Evalúa qué modo está activo para enviar al visor 3D
    """
    global estado_visor
    try:
        while True:
            if estado_visor["modo"] == "streaming":
                paquete = {
                    "tipo": "streaming",
                    "dedos": valores_tiempo_real
                }
            else:
                paquete = {
                    "tipo": "pose_fija",
                    "dedos": estado_visor["dedos_fijos"],
                    "letra_mostrada": estado_visor["letra_fija"]
                }
            
            await websocket.send(json.dumps(paquete))
            await asyncio.sleep(0.015)
    except websockets.exceptions.ConnectionClosed:
        pass

async def leer_y_grab_serial():
    global letra_activa, valores_tiempo_real
    while True:
        if guante_conectado and guante_serial and guante_serial.in_waiting > 0:
            try:
                linea = guante_serial.readline().decode('utf-8', errors='ignore').strip()
                if not linea: continue
                
                partes = [p for p in linea.replace("\t", ",").replace(" ", "").split(',') if p]
                
                if len(partes) >= 11:
                    v = [float(x) for x in partes[:11]]
                    
                    valores_tiempo_real = {"pulgar": v[0], "indice": v[1], "corazon": v[2], "anular": v[3], "menique": v[4]}
                    
                    datos_actuales["dedos"] = valores_tiempo_real.copy()
                    datos_actuales["imu"] = {"ax": v[5], "ay": v[6], "az": v[7], "gx": v[8], "gy": v[9], "gz": v[10]}
                    
                    if letra_activa != "":
                        with open(ARCHIVO_SALIDA, "a", newline="", encoding="utf-8") as archivo:
                            csv.writer(archivo).writerow(v + [letra_activa])
            except ValueError:
                pass
        await asyncio.sleep(0.001)

async def consola_control():
    global letra_activa, estado_visor
    try:
        with open(ARCHIVO_SALIDA, "x", newline="", encoding="utf-8") as f:
            csv.writer(f).writerow(["Pulgar","Índice","Corazón","Anular","Meñique","AX","AY","AZ","GX","GY","GZ","Letra"])
    except FileExistsError:
        pass

    print("========================================================")
    print("-> El visualizador 3D está en TIEMPO REAL 🟢")
    print("-> Escribe una LETRA para grabar 10s (Requiere Guante)")
    print("-> Escribe 'mostrar <letra>' para ver promedios en 3D")
    print("-> Escribe 'pose <p> <i> <c> <a> <m>' para forzar una pose manual (Ej: pose 100 50 0 0 100)")
    print("-> Escribe 'valores' para ver la lectura instantánea del hardware")
    print("-> Escribe 'volver' para regresar al tiempo real")
    print("-> Escribe 0 para salir")
    print("========================================================\n")

    while True:
        entrada = await asyncio.to_thread(input, "Comando >> ")
        entrada = entrada.strip()

        if entrada == "0":
            if guante_conectado and guante_serial:
                guante_serial.close()
            sys.exit(0)

        # Comando para regresar al tiempo real
        if entrada.lower() == "volver":
            estado_visor["modo"] = "streaming"
            print("🟢 Visor 3D de vuelta al TIEMPO REAL.")
            continue

        # Comando: Control manual de porcentajes
        if entrada.lower().startswith("pose "):
            partes_entrada = entrada.split(" ")
            
            if len(partes_entrada) != 6:
                print("❌ Formato incorrecto. Debes introducir 5 valores entre 0 y 100.")
                print("👉 Uso: pose <pulgar> <índice> <corazón> <anular> <meñique>")
                print("👉 Ejemplo: pose 100 50 0 0 100")
                continue
            
            try:
                valores = [float(v) for v in partes_entrada[1:6]]
                valores = [max(0.0, min(100.0, v)) for v in valores]
                
                estado_visor["modo"] = "pose_fija"
                estado_visor["dedos_fijos"] = {
                    "pulgar": valores[0],
                    "indice": valores[1],
                    "corazon": valores[2],
                    "anular": valores[3],
                    "menique": valores[4]
                }
                estado_visor["letra_fija"] = "MANUAL"
                
                print(f"🎮 [Consola] Visor 3D actualizado a pose manual:")
                print(f"   Pulgar: {valores[0]}% | Índice: {valores[1]}% | Corazón: {valores[2]}% | Anular: {valores[3]}% | Meñique: {valores[4]}%")
                print("👉 Escribe 'volver' para regresar al tiempo real.\n")
                
            except ValueError:
                print("❌ Error: Los valores deben ser números (puedes usar decimales con punto).")
            continue

        # Comando para mostrar letra del CSV
        if entrada.lower().startswith("mostrar "):
            partes_entrada = entrada.split(" ")
            if len(partes_entrada) < 2:
                print("❌ Especifica una letra. Ejemplo: mostrar A")
                continue
            letra_a_buscar = partes_entrada[1].strip().upper()
            medias = calcular_media_letra(letra_a_buscar)
            
            if medias:
                print(f"\n📊 [Consola] Promedios para la Letra '{letra_a_buscar}':")
                for dedo, valor in medias.items():
                    print(f"   {dedo.capitalize()}: {valor:.2f}%")
                
                estado_visor["modo"] = "pose_fija"
                estado_visor["dedos_fijos"] = medias
                estado_visor["letra_fija"] = letra_a_buscar
                print(f"🎬 Visor 3D actualizado a la pose de la letra '{letra_a_buscar}'.")
                print("👉 Escribe 'volver' para regresar al tiempo real.\n")
                
            else:
                print(f"❌ No hay muestras válidas en '{ARCHIVO_SALIDA}' para la letra '{letra_a_buscar}'")
            continue

        # Comando: Mostrar valores actuales en tiempo real
        if entrada.lower() == "valores":
            if not guante_conectado:
                print("❌ No se pueden mostrar valores. El guante físico no está conectado.")
                continue
            
            imu = datos_actuales["imu"]
            print("\n🧤 [Consola] Lectura instantánea del guante:")
            print("--------------------------------------------------------")
            print(f" 🖐️  DEDOS:")
            print(f"    Pulgar:  {valores_tiempo_real['pulgar']:.2f}%")
            print(f"    Índice:  {valores_tiempo_real['indice']:.2f}%")
            print(f"    Corazón: {valores_tiempo_real['corazon']:.2f}%")
            print(f"    Anular:  {valores_tiempo_real['anular']:.2f}%")
            print(f"    Meñique: {valores_tiempo_real['menique']:.2f}%")
            print(f" 🧭 IMU (Acelerómetro y Giroscopio):")
            print(f"    AX: {imu['ax']:.2f} | AY: {imu['ay']:.2f} | AZ: {imu['az']:.2f}")
            print(f"    GX: {imu['gx']:.2f} | GY: {imu['gy']:.2f} | GZ: {imu['gz']:.2f}")
            print("--------------------------------------------------------\n")
            continue

        # Lógica por defecto: Grabar letra
        letra = entrada.upper()
        if len(letra) == 1 and letra.isalpha():
            if not guante_conectado:
                print("❌ Grabación cancelada: Necesitas conectar el guante físico para recolectar datos.")
                continue
                
            print(f"\n🔴 Grabando letra '{letra}' en 3 segundos...")
            for i in range(3, 0, -1):
                print(f"{i}...")
                await asyncio.sleep(1)

            letra_activa = letra
            print("🟢 CAPTURANDO DATOS AL CSV...")
            await asyncio.sleep(SEGUNDOS_POR_LETRA)
            letra_activa = ""
            print(f"✨ Muestras guardadas para la letra '{letra}'\n")

async def main():
    server = await websockets.serve(transmitir_datos, "localhost", 8765)
    await asyncio.gather(server.wait_closed(), leer_y_grab_serial(), consola_control())

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        if guante_conectado and guante_serial:
            guante_serial.close()