#include "motor.h"
#include "suavizado.h"
#include <Arduino.h>

MotorVibracion::MotorVibracion(float factorSuavizado) : factor(factorSuavizado) {}

void MotorVibracion::iniciar(int pin, int canal, int frecuencia, int resolucion) {
  canalPwm = canal;
  pinMode(pin, OUTPUT);
  ledcSetup(canalPwm, frecuencia, resolucion);
  ledcAttachPin(pin, canalPwm);
  ledcWrite(canalPwm, 0);
}

void MotorVibracion::setIntensidad(int intensidad) {
  objetivo = intensidad;
}

void MotorVibracion::actualizar() {
  pwmSuave = suavizarPwm(pwmSuave, objetivo, factor);
  ledcWrite(canalPwm, (int)pwmSuave);
}

int MotorVibracion::pwmActual() const {
  return (int)pwmSuave;
}