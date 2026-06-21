import os
import sys
import time
import subprocess
import webbrowser
from pathlib import Path


# ==========================================================
# RUTAS DEL PROYECTO
# ==========================================================

BASE_DIR = Path(__file__).resolve().parent

MODO_1_DIR = BASE_DIR / "modo1_traduccion"
MODO_1_SCRIPT = MODO_1_DIR / "traductor_vivo.py"

MODO_2_DIR = BASE_DIR / "modo2_web_visualizacion"
MODO_2_SCRIPT = MODO_2_DIR / "guante_datos.py"
MODO_2_HTML = MODO_2_DIR / "index.html"


# ==========================================================
# COLORES DE TERMINAL
# ==========================================================

ROJO = "\033[91m"
VERDE = "\033[92m"
AZUL = "\033[94m"
AMARILLO = "\033[93m"
NEGRITA = "\033[1m"
RESET = "\033[0m"


# ==========================================================
# FUNCIONES GENERALES
# ==========================================================

def limpiar_pantalla():
    os.system("cls" if os.name == "nt" else "clear")


def mostrar_logo():
    print(f"""
{NEGRITA}   ____           _    {ROJO} _   _ ____ {RESET}
{NEGRITA}  / ___| ___  ___| |_ {ROJO}| | | / ___|{RESET}
{NEGRITA} | |  _ / _ \\/ __| __|{ROJO}| | | \\___ \\{RESET}
{NEGRITA} | |_| |  __/\\__ \\ |_ {ROJO}| |_| |___) |{RESET}
{NEGRITA}  \\____|\\___||___/\\__|{ROJO} \\___/|____/{RESET}

{AZUL} Guante sensorial inteligente{RESET}
{AZUL} Traducción de gestos · Web 3D · Rehabilitación{RESET}
----------------------------------------------------------
""")


def pausar():
    input("\nPulsa ENTER para continuar...")


def comprobar_archivo(ruta, nombre):
    if not ruta.exists():
        print(f"{ROJO}ERROR: No se encontró {nombre}{RESET}")
        print(f"Ruta esperada: {ruta}")
        pausar()
        return False
    return True


def detener_proceso(proceso):
    if proceso and proceso.poll() is None:
        proceso.terminate()
        try:
            proceso.wait(timeout=3)
        except subprocess.TimeoutExpired:
            proceso.kill()


# ==========================================================
# MODO 1 - TRADUCCIÓN DE LETRAS
# ==========================================================

def modo_1_traduccion():
    limpiar_pantalla()
    mostrar_logo()

    print(f"{NEGRITA}MODO 1 - Traducción de letras sencillas{RESET}")
    print("Este modo ejecuta el traductor en tiempo real.\n")

    if not comprobar_archivo(MODO_1_SCRIPT, "traductor_vivo.py"):
        return

    if not comprobar_archivo(MODO_1_DIR / "modelo_guante.pkl", "modelo_guante.pkl"):
        return

    if not comprobar_archivo(MODO_1_DIR / "escalador_guante.pkl", "escalador_guante.pkl"):
        return

    print(f"{AMARILLO}IMPORTANTE:{RESET}")
    print("- Cierra el Monitor Serie de Arduino antes de ejecutar este modo.")
    print("- Revisa el puerto dentro de traductor_vivo.py si no conecta.")
    print()
    input("Pulsa ENTER para iniciar el traductor...")

    try:
        resultado = subprocess.run(
            [sys.executable, str(MODO_1_SCRIPT)],
            cwd=str(MODO_1_DIR)
        )

        # Si desde traductor_vivo.py se pulsa 0, salimos del launcher completo
        if resultado.returncode == 10:
            limpiar_pantalla()
            print("Saliendo de GestUS...")
            sys.exit(0)

    except KeyboardInterrupt:
        print("\nVolviendo al menú principal...")
        time.sleep(1)


# ==========================================================
# MODO 2 - WEB DE VISUALIZACIÓN 3D
# ==========================================================

def modo_2_web():
    limpiar_pantalla()
    mostrar_logo()

    print(f"{NEGRITA}MODO 2 - Web de visualización 3D / rehabilitación{RESET}")
    print("Este modo ejecuta la web de visualización del guante.\n")

    if not comprobar_archivo(MODO_2_SCRIPT, "guante_datos.py"):
        return

    if not comprobar_archivo(MODO_2_HTML, "index.html"):
        return

    print(f"{AMARILLO}IMPORTANTE:{RESET}")
    print("- Cierra el Monitor Serie de Arduino antes de ejecutar este modo.")
    print("- Revisa el puerto dentro de guante_datos.py si no conecta.")
    print()
    print("Cuando se abra el modo web, usa los comandos del propio programa.")
    print("Dentro de guante_datos.py puedes escribir 0 para cerrar ese modo.")
    print()

    proceso = None

    try:
        proceso = subprocess.Popen(
            [sys.executable, str(MODO_2_SCRIPT)],
            cwd=str(MODO_2_DIR)
        )

        time.sleep(2)

        print("Abriendo index.html en el navegador...")
        webbrowser.open(MODO_2_HTML.as_uri())

        print("\nModo web activo.")
        print("Cuando cierres el modo web con 0, volverás al menú principal.\n")

        proceso.wait()

    except KeyboardInterrupt:
        detener_proceso(proceso)
        print("\nVolviendo al menú principal...")
        time.sleep(1)


# ==========================================================
# MENÚ PRINCIPAL
# ==========================================================

def menu_principal():
    while True:
        limpiar_pantalla()
        mostrar_logo()

        print(f"{NEGRITA}Selecciona un modo de uso:{RESET}\n")
        print("1. Modo 1 - Traducción de letras sencillas")
        print("2. Modo 2 - Web de visualización 3D / rehabilitación")
        print("0. Salir")
        print()

        opcion = input("Opción >> ").strip()

        if opcion == "1":
            modo_1_traduccion()

        elif opcion == "2":
            modo_2_web()

        elif opcion == "0":
            limpiar_pantalla()
            print("Saliendo de GestUS...")
            break

        else:
            print(f"\n{ROJO}Opción no válida.{RESET}")
            pausar()


# ==========================================================
# MAIN
# ==========================================================

if __name__ == "__main__":
    menu_principal()