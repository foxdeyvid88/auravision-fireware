#ifndef MOTOR_H
#define MOTOR_H

// Motor de vibración controlado por PWM (LEDC). Aplica suavizado
// exponencial sobre la intensidad objetivo y se apaga al instante
// cuando el objetivo es 0.
class MotorVibracion {
public:
  explicit MotorVibracion(float factorSuavizado);

  void iniciar(int pin, int canal, int frecuencia, int resolucion);
  void setIntensidad(int intensidad);
  void actualizar();
  int pwmActual() const;

private:
  float factor;
  int objetivo = 0;
  float pwmSuave = 0.0f;
  int canalPwm = -1;
};

#endif