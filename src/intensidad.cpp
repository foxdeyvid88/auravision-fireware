#include "intensidad.h"

namespace {
constexpr float DIST_MIN_CM = 20.0f;
constexpr float DIST_MAX_CM = 180.0f;
constexpr float UMBRAL_ALTA_CM = 80.0f;
constexpr float UMBRAL_MEDIA_CM = 140.0f;
constexpr int INTENSIDAD_ALTA = 225;
constexpr int INTENSIDAD_MEDIA = 175;
constexpr int INTENSIDAD_BAJA = 130;
}

int intensidadParaDistancia(float distancia) {
  if (distancia < 0.0f) return 0;                       // sin lectura
  if (distancia < DIST_MIN_CM || distancia > DIST_MAX_CM) return 0;  // fuera de rango
  if (distancia < UMBRAL_ALTA_CM) return INTENSIDAD_ALTA;
  if (distancia < UMBRAL_MEDIA_CM) return INTENSIDAD_MEDIA;
  return INTENSIDAD_BAJA;
}