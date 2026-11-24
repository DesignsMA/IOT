# Implementación con Bluetooth Classic

Este ejemplo contiene una implementación sencilla para probar una conexión mediante Bluetooth Classic desde el entorno de Arduino. Está pensado principalmente para placas ESP32 compatibles con el protocolo Bluetooth Serial, como las que utilizan la biblioteca `BluetoothSerial`.

El programa configura el dispositivo con el nombre **MarcoFoco728** y permite controlar un relevador conectado al pin **4**. Al recibir una conexión Bluetooth, el usuario puede enviar los siguientes comandos:

- `1`: activa la salida del relevador.
- `0`: desactiva la salida del relevador.

Para conectarse, el dispositivo Bluetooth debe emparejarse desde otro teléfono o computadora. Después se puede utilizar una aplicación de terminal serie, como **Serial Bluetooth Terminal**, para enviar los comandos descritos.

## Requisitos

- Una placa ESP32 compatible con Bluetooth Classic.
- Arduino IDE con el soporte para ESP32 instalado.
- Un relevador o dispositivo equivalente conectado al pin 4.
- Protección eléctrica adecuada si se controla una carga de red, como 120 V o 220 V.

## Puesta en marcha

1. Abra el archivo `arduino_classic.ino` en Arduino IDE.
2. Seleccione la placa ESP32 correspondiente y el puerto serie.
3. Compile y cargue el programa en la placa.
4. Abra el monitor serie a una velocidad de 115200 baudios.
5. Busque el dispositivo **MarcoFoco728** desde el dispositivo Bluetooth externo.
6. Empareje ambos dispositivos y establezca la conexión serie.
7. Envíe `1` para encender el relevador o `0` para apagarlo.

## Consideraciones

La implementación muestra los estados y las comunicaciones también por el puerto serie a 115200 baudios. El pin del relevador inicia en nivel bajo durante el arranque para evitar una activación inesperada.