#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled. Select an ESP32 board with Bluetooth Classic support.
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Bluetooth SPP is not available on this chip.
#endif

const char *deviceName = "MarcoFoco728";
const int relayPin = 4;

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  if (!SerialBT.begin(deviceName)) {
    Serial.println("No se pudo iniciar Bluetooth Classic.");
    while (true) {
      delay(1000);
    }
  }

  Serial.printf("Bluetooth Classic iniciado: %s\n", deviceName);
  Serial.println("Envia 1 para encender o 0 para apagar el relevador.");
}

void loop() {
  if (SerialBT.available()) {
    char command = SerialBT.read();

    if (command == '1') {
      digitalWrite(relayPin, HIGH);
      SerialBT.println("Relevador encendido");
      Serial.println("Relevador encendido");
    } else if (command == '0') {
      digitalWrite(relayPin, LOW);
      SerialBT.println("Relevador apagado");
      Serial.println("Relevador apagado");
    }
  }

  delay(20);
}
