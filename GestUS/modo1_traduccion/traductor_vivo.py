import serial
import pickle
import numpy as np
import warnings
import sys
import time
import os
from pathlib import Path


# ======================================================
# LIBRERÍAS VISUALES OPCIONALES
# ======================================================

try:
    from rich.console import Console
    from rich.panel import Panel
    from rich.align import Align
    from rich.text import Text
    from pyfiglet import Figlet

    RICH_ACTIVO = True
    console = Console()
    figlet = Figlet(font="big")

except Exception:
    RICH_ACTIVO = False
    console = None
    figlet = None


# ======================================================
# CONFIGURACIÓN
# ======================================================

warnings.filterwarnings("ignore")

BASE_DIR = Path(__file__).resolve().parent

ARCHIVO_MODELO = BASE_DIR / "modelo_guante.pkl"
ARCHIVO_ESCALADOR = BASE_DIR / "escalador_guante.pkl"

PUERTO = "/dev/ttyUSB0"
BAUDIOS = 115200

CODIGO_SALIR_COMPLETO = 10


# ======================================================
# COLORES ANSI
# ======================================================

ROJO = "\033[91m"
VERDE = "\033[92m"
AZUL = "\033[94m"
AMARILLO = "\033[93m"
NEGRITA = "\033[1m"
RESET = "\033[0m"


# ======================================================
# FUNCIONES VISUALES
# ======================================================

def limpiar_pantalla():
    if RICH_ACTIVO:
        console.clear()
    else:
        os.system("cls" if os.name == "nt" else "clear")


def mostrar_logo():
    if RICH_ACTIVO:
        logo = Text()
        logo.append("Gest", style="bold white")
        logo.append("US", style="bold red")

        console.print()
        console.print(Align.center(logo))
        console.print(Align.center("[blue]Guante sensorial inteligente[/blue]"))
        console.print(Align.center("[blue]Traducción de letras sencillas en tiempo real[/blue]"))
        console.print()

    else:
        print(f"""
{NEGRITA}   ____           _    {ROJO} _   _ ____ {RESET}
{NEGRITA}  / ___| ___  ___| |_ {ROJO}| | | / ___|{RESET}
{NEGRITA} | |  _ / _ \\/ __| __|{ROJO}| | | \\___ \\{RESET}
{NEGRITA} | |_| |  __/\\__ \\ |_ {ROJO}| |_| |___) |{RESET}
{NEGRITA}  \\____|\\___||___/\\__|{ROJO} \\___/|____/{RESET}

{AZUL} Guante sensorial inteligente{RESET}
{AZUL} Traducción de letras sencillas en tiempo real{RESET}
----------------------------------------------------------
""")


def letra_grande(letra):
    if RICH_ACTIVO:
        return figlet.renderText(str(letra))
    else:
        return f"[ {letra} ]"


def mostrar_panel(letra, total_predicciones, puerto):
    limpiar_pantalla()
    mostrar_logo()

    if RICH_ACTIVO:
        letra_ascii = letra_grande(letra)

        contenido = f"""
[bold green]Estado:[/bold green] Traductor activo
[bold green]Puerto:[/bold green] {puerto}

[bold white]Letra detectada:[/bold white]

[bold red]{letra_ascii}[/bold red]

[bold cyan]Predicciones realizadas:[/bold cyan] {total_predicciones}

[yellow]Controles:[/yellow]
1 + ENTER  -> volver al menú principal
0 + ENTER  -> salir completamente
CTRL + C   -> cerrar traductor
"""

        panel = Panel(
            contenido,
            title="[bold red]MODO 1 - Traducción de letras sencillas[/bold red]",
            border_style="red",
            padding=(1, 4)
        )

        console.print(panel)

    else:
        print(f"{NEGRITA}MODO 1 - Traducción de letras sencillas{RESET}")
        print()
        print(f"{VERDE}Estado:{RESET} Traductor activo")
        print(f"{VERDE}Puerto:{RESET} {puerto}")
        print()
        print("==========================================================")
        print()
        print("                 Letra detectada")
        print()
        print(f"                       {NEGRITA}{ROJO}[ {letra} ]{RESET}")
        print()
        print("==========================================================")
        print()
        print(f"Predicciones realizadas: {total_predicciones}")
        print()
        print(f"{AMARILLO}Controles:{RESET}")
        print("1 + ENTER  -> volver al menú principal")
        print("0 + ENTER  -> salir completamente")
        print("CTRL + C   -> cerrar traductor")
        print()


def leer_comando_no_bloqueante():
    try:
        import select

        if sys.stdin in select.select([sys.stdin], [], [], 0)[0]:
            return sys.stdin.readline().strip()

    except Exception:
        pass

    return None


def detectar_puerto():
    posibles = [
        "/dev/ttyUSB0",
        "/dev/ttyUSB1",
        "/dev/ttyACM0",
        "/dev/ttyACM1",
    ]

    for puerto in posibles:
        if Path(puerto).exists():
            return puerto

    return PUERTO


# ======================================================
# 1. CARGAR MODELO Y ESCALADOR
# ======================================================

limpiar_pantalla()
mostrar_logo()

if RICH_ACTIVO:
    console.print("[yellow]Despertando el cerebro del guante...[/yellow]")
else:
    print("Despertando el cerebro del guante...")

try:
    with open(ARCHIVO_MODELO, "rb") as f:
        red_guante = pickle.load(f)

    with open(ARCHIVO_ESCALADOR, "rb") as f:
        scaler = pickle.load(f)

    if RICH_ACTIVO:
        console.print("[green]Modelo cargado correctamente.[/green]")
        console.print("[green]Escalador cargado correctamente.[/green]")
    else:
        print(f"{VERDE}Modelo cargado correctamente.{RESET}")
        print(f"{VERDE}Escalador cargado correctamente.{RESET}")

except FileNotFoundError as e:
    print(f"\n{ROJO}ERROR: No se encontró el modelo o el escalador.{RESET}")
    print()
    print("Comprueba que estos archivos están dentro de modo1_traduccion:")
    print("- modelo_guante.pkl")
    print("- escalador_guante.pkl")
    print()
    print("Detalle:")
    print(e)
    sys.exit(1)

except Exception as e:
    print(f"\n{ROJO}ERROR: No se pudo cargar el modelo.{RESET}")
    print("Comprueba que estás usando el entorno virtual correcto.")
    print()
    print("Para activarlo:")
    print("cd ~/Escritorio/GestUS")
    print("source .venv_modelo_antiguo/bin/activate")
    print("python gestus_launcher.py")
    print()
    print("Detalle del error:")
    print(e)
    sys.exit(1)


# ======================================================
# 2. CONECTAR AL ESP32
# ======================================================

PUERTO_DETECTADO = detectar_puerto()

print()
print(f"Intentando conectar al ESP32 en: {PUERTO_DETECTADO}")

try:
    arduino = serial.Serial(PUERTO_DETECTADO, BAUDIOS, timeout=1)
    time.sleep(2)

    if RICH_ACTIVO:
        console.print("[green]Conectado correctamente al guante.[/green]")
    else:
        print(f"{VERDE}Conectado correctamente al guante.{RESET}")

    time.sleep(1)

except Exception as e:
    print(f"\n{ROJO}ERROR: No se pudo conectar al puerto {PUERTO_DETECTADO}.{RESET}")
    print()
    print("Revisa:")
    print("- Que el ESP32 esté conectado.")
    print("- Que el Monitor Serie de Arduino esté cerrado.")
    print("- Que el puerto sea correcto.")
    print()
    print("Para ver el puerto:")
    print("ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null")
    print()
    print("Detalle:")
    print(e)
    sys.exit(1)


# ======================================================
# 3. TRADUCCIÓN EN TIEMPO REAL
# ======================================================

ultima_letra = None
total_predicciones = 0
ultima_actualizacion = 0

mostrar_panel("-", total_predicciones, PUERTO_DETECTADO)

try:
    while True:
        comando = leer_comando_no_bloqueante()

        if comando == "1":
            print("\nVolviendo al menú principal...")
            break

        if comando == "0":
            print("\nSaliendo completamente de GestUS...")
            sys.exit(CODIGO_SALIR_COMPLETO)

        try:
            linea = arduino.readline().decode("utf-8", errors="ignore").strip()

            if not linea:
                continue

            datos_str = linea.split(",")

            # El modelo espera exactamente 11 valores:
            # 5 flexiones + 6 valores del MPU6050
            if len(datos_str) != 11:
                continue

            datos_float = [float(x) for x in datos_str]

            mano_array = np.array([datos_float])

            mano_escalada = scaler.transform(mano_array)

            letra_predicha = red_guante.predict(mano_escalada)[0]

            total_predicciones += 1
            ahora = time.time()

            # Actualiza pantalla solo si cambia la letra
            # o cada 0.7 segundos para evitar parpadeo excesivo
            if letra_predicha != ultima_letra or ahora - ultima_actualizacion > 0.7:
                mostrar_panel(letra_predicha, total_predicciones, PUERTO_DETECTADO)
                ultima_letra = letra_predicha
                ultima_actualizacion = ahora

        except ValueError:
            pass

        except Exception as e:
            print(f"{ROJO}Error durante la predicción:{RESET}")
            print(e)
            time.sleep(1)

except KeyboardInterrupt:
    print("\nApagando traductor...")

finally:
    try:
        arduino.close()
    except Exception:
        pass

    print("Traductor cerrado correctamente.")