#ifndef SENSOR_US100_H
#define SENSOR_US100_H

#include <HardwareSerial.h>

// Sensor ultrasónico US-100 en modo UART. Lectura NO bloqueante:
// medir() dispara una medición, espera la respuesta y avanza por estados
// sin detener el loop. Devuelve true cuando hay una lectura nueva.
class SensorUS100 {
public:
  SensorUS100(HardwareSerial &puerto, unsigned long timeoutMs);

  void iniciar(unsigned long baud, int pinRx, int pinTx);
  bool medir();
  float distancia() const;

private:
  enum class Estado { IDLE, ESPERANDO, PAUSA };

  HardwareSerial &serial;
  unsigned long timeoutMs;
  Estado estado = Estado::IDLE;
  unsigned long cambioEstado = 0;
  float ultimaDistancia = -1.0f;
};

#endif