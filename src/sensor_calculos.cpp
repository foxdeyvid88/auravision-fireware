#include "sensor_calculos.h"

namespace {
constexpr float DIST_MIN_CM = 2.0f;
constexpr float DIST_MAX_CM = 450.0f;
}

float distanciaDesdeBytes(uint8_t alta, uint8_t baja) {
  int distanciaMm = (alta << 8) | baja;
  float cm = distanciaMm / 10.0f;
  return (cm >= DIST_MIN_CM && cm <= DIST_MAX_CM) ? cm : -1.0f;
}