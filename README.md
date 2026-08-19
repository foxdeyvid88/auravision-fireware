# AuraVision Firmware

Firmware para **Seeed XIAO ESP32-S3** (framework Arduino, PlatformIO) que detecta
obstáculos con dos sensores ultrasónicos **US-100** y vibra dos motores con
intensidad proporcional a la distancia, como ayuda de movilidad.

## Características

- Dos sensores ultrasónicos US-100 en modo UART (izquierda/derecha) con lectura **no bloqueante**.
- Dos motores de vibración controlados por PWM (LEDC) con suavizado exponencial.
- LED de estado que parpadea cada 500 ms.
- Lógica de intensidad, filtro y parseo de bytes como **funciones puras** probadas con tests unitarios en el host.
- Loop con pacing de ~12 mediciones/segundo por sensor.

## Mapa de pines (XIAO ESP32-S3)

| Señal          | Pin |
| -------------- | --- |
| Sensor 1 RX    | D0  |
| Sensor 1 TX    | D1  |
| Sensor 2 RX    | D2  |
| Sensor 2 TX    | D3  |
| Motor izquierdo| D4  |
| Motor derecho  | D5  |
| LED de estado  | LED_BUILTIN |

Los pines y constantes se configuran centralmente en `include/config.h`.

## Estructura del proyecto

```
├── include/
│   └── config.h              # Pines y parámetros de configuración
├── src/
│   ├── main.cpp              # Orquestación: setup() y loop()
│   ├── intensidad.{h,cpp}    # Distancia -> intensidad de vibración (lógica pura)
│   ├── sensor_us100.{h,cpp}  # Protocolo UART del US-100 (máquina de estados)
│   ├── motor.{h,cpp}         # PWM + suavizado del motor
│   ├── suavizado.{h,cpp}     # Filtro exponencial (lógica pura)
│   └── sensor_calculos.{h,cpp}  # Parseo de bytes del sensor (lógica pura)
├── test/
│   ├── test_intensidad/      # Tests de la regla distancia -> intensidad
│   ├── test_suavizado/       # Tests del filtro exponencial
│   └── test_sensor_calculos/ # Tests del parseo de bytes (rango, orden)
└── platformio.ini            # Entornos: firmware y tests nativos
```

## Cómo funciona

En cada iteración del `loop()`:

1. Cada sensor se dispara con el byte `0x55`. La respuesta (2 bytes, distancia en mm,
   big-endian) se lee de forma asíncrona por una máquina de estados
   (`IDLE -> ESPERANDO -> PAUSA`) que nunca bloquea el loop.
2. Al llegar una lectura nueva, se mapea a intensidad: 225 si el obstáculo está
   a <80 cm, 175 si está entre 80 y 140 cm, 130 entre 140 y 180 cm, y 0 fuera de
   ese rango o sin lectura.
3. La intensidad se suaviza con un filtro exponencial (factor 0.75) y se escribe
   por PWM. Si la intensidad objetivo es 0, el motor se apaga al instante.

## Compilar y subir

```bash
# Compilar el firmware
pio run -e seeed_xiao_esp32s3

# Subir a la placa (ajusta el puerto en tu sistema si hace falta)
pio run -e seeed_xiao_esp32s3 -t upload

# Monitor serie
pio device monitor -e seeed_xiao_esp32s3 -b 115200
```

## Tests unitarios

Las funciones puras (`intensidadParaDistancia`, `suavizarPwm` y
`distanciaDesdeBytes`) se prueban en el host con el framework Unity.

```bash
# Requisito: compilador de C++ (Debian/Ubuntu: sudo apt install g++; Fedora: sudo dnf install gcc-c++)
pio test -e native
```

Nota: `pio test -e native` solo compila y ejecuta los tests en tu PC; no toca la
placa. El hardware (sensor y motor) se valida con `pio run -e seeed_xiao_esp32s3
-t upload`.