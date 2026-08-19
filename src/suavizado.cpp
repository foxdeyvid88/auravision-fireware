#include "suavizado.h"

float suavizarPwm(float actual, int objetivo, float factor) {
  if (objetivo == 0) return 0.0f;
  return actual + factor * (objetivo - actual);
}