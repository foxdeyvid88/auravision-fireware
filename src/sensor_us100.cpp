#include "sensor_us100.h"
#include "sensor_calculos.h"

namespace {
constexpr unsigned long PAUSA_TRAS_FALLO_MS = 100;
}

SensorUS100::SensorUS100(HardwareSerial &puerto, unsigned long timeout)
    : serial(puerto), timeoutMs(timeout) {}

void SensorUS100::iniciar(unsigned long baud, int pinRx, int pinTx) {
  serial.begin(baud, SERIAL_8N1, pinRx, pinTx);
}

bool SensorUS100::medir() {
  unsigned long ahora = millis();

  switch (estado) {
    case Estado::IDLE:
      while (serial.available()) serial.read();
      serial.write(0x55);
      estado = Estado::ESPERANDO;
      cambioEstado = ahora;
      break;

    case Estado::ESPERANDO:
      if (serial.available() >= 2) {
        uint8_t alta = serial.read();
        uint8_t baja = serial.read();
        ultimaDistancia = distanciaDesdeBytes(alta, baja);
        estado = Estado::IDLE;
        return true;
      }
      if (ahora - cambioEstado >= timeoutMs) {
        ultimaDistancia = -1.0f;
        estado = Estado::PAUSA;
        cambioEstado = ahora;
        return true;
      }
      break;

    case Estado::PAUSA:
      if (ahora - cambioEstado >= PAUSA_TRAS_FALLO_MS) {
        estado = Estado::IDLE;
      }
      break;
  }

  return false;
}

float SensorUS100::distancia() const {
  return ultimaDistancia;
}