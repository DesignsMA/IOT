# Interruptor Bluetooth con NimBLE

## Propósito de la práctica

Este proyecto es una práctica con ESP-IDF y el stack Bluetooth LE NimBLE de
Espressif. El objetivo es conectar un teléfono con el ESP32, controlar un
interruptor desde una aplicación BLE y observar cómo cambia el estado del
dispositivo mediante un LED RGB direccionable.

El proyecto está basado en los ejemplos de Espressif **NimBLE_GATT_Server** y
**NimBLE_Beacon**. Toma de ellos la inicialización de NimBLE, la configuración
de publicidad y el manejo de eventos GAP, y los adapta para agregar un servicio
GATT con una característica de escritura que controla la salida del interruptor.

## Funcionamiento

1. El ESP32 inicia NimBLE y anuncia el dispositivo por Bluetooth LE.
2. Desde una aplicación central, como **nRF Connect for Mobile**, se establece
   la conexión y el emparejamiento.
3. La aplicación escribe un byte en la característica del servicio:
   - Un valor distinto de `0` activa el interruptor (GPIO 4).
   - `0` desactiva el interruptor (GPIO 4).
4. El LED RGB muestra el estado de la conexión y del interruptor.

La característica GATT requiere escritura cifrada. El código configura
emparejamiento, vinculación (bonding) y una clave estática; estos parámetros se
encuentran en `main/main.c` y `main/include/common.h`.

## Estados del LED RGB

| Color | Estado |
| --- | --- |
| Cian | El dispositivo está anunciándose y espera una conexión. |
| Verde | Hay una conexión BLE activa. |
| Rojo | Indicación breve al desconectarse; después se reanuda la publicidad y vuelve el cian. |
| Ámbar | El interruptor está activado. |
| Naranja | El interruptor está desactivado. |
| Apagado | Al iniciar o durante transiciones breves entre estados. |

## Organización del código

- `main/main.c`: inicializa GPIO, LED, NVS y NimBLE; configura el servidor GATT
  e inicia la tarea del host NimBLE.
- `main/src/gap.c`: configura la publicidad y procesa eventos GAP, como
  conexión, desconexión y emparejamiento.
- `main/src/gatt_svc.c`: define el servicio y procesa las escrituras BLE que
  activan o desactivan el interruptor.
- `main/src/led.c`: controla el LED RGB direccionable y sus transiciones.

## Compilar y ejecutar

Con ESP-IDF configurado, selecciona el objetivo correspondiente a tu placa y
compila y carga el proyecto:

```shell
idf.py set-target <objetivo>
idf.py build
idf.py -p <puerto> flash monitor
```

Por ejemplo, usa `esp32s3` como objetivo si tu placa está basada en ESP32-S3.
Luego, conéctate al dispositivo desde nRF Connect for Mobile, descubre el
servicio y escribe `1` o `0` en su característica para controlar el interruptor.

## Alternativa simple para Arduino IDE y Bluetooth Classic

El sketch independiente [`arduino_classic/arduino_classic.ino`](./arduino_classic/arduino_classic.ino)
controla el relevador por Bluetooth Classic SPP: empareja el teléfono con
`MarcoFoco728` y envía `1` para encender o `0` para apagar. Usa GPIO 4 y nivel alto
para activar el relevador, igual que la implementación ESP-IDF.

Esta alternativa requiere una placa ESP32 con Bluetooth Classic, como ESP32
original (WROOM/WROVER). No es compatible con ESP32-S3, ESP32-C3 ni otros
modelos que solo admiten Bluetooth LE; el objetivo actual de este proyecto es
ESP32-S3.
