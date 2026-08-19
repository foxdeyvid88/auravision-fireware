#include <Arduino.h>
#include "config.h"
#include "intensidad.h"
#include "sensor_us100.h"
#include "motor.h"

HardwareSerial SerialSensor2(2);

SensorUS100 sensorIzq(Serial1, SENSOR_TIMEOUT_MS);
SensorUS100 sensorDer(SerialSensor2, SENSOR_TIMEOUT_MS);

MotorVibracion motorIzq(FACTOR_SUAVIZADO);
MotorVibracion motorDer(FACTOR_SUAVIZADO);

unsigned long ultimoParpadeo = 0;
bool estadoLed = false;

void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  sensorIzq.iniciar(SENSOR_BAUD, S1_RX_PIN, S1_TX_PIN);
  sensorDer.iniciar(SENSOR_BAUD, S2_RX_PIN, S2_TX_PIN);

  motorIzq.iniciar(MOTOR_IZQ_PIN, PWM_CH_IZQ, PWM_FREQ, PWM_RES);
  motorDer.iniciar(MOTOR_DER_PIN, PWM_CH_DER, PWM_FREQ, PWM_RES);

  Serial.println("=== SISTEMA INICIADO ===");
}

void loop() {
  unsigned long inicioLoop = millis();

  if (millis() - ultimoParpadeo >= 500) {
    ultimoParpadeo = millis();
    estadoLed = !estadoLed;
    digitalWrite(LED_PIN, estadoLed);
  }

  if (sensorIzq.medir()) {
    motorIzq.setIntensidad(intensidadParaDistancia(sensorIzq.distancia()));
  }

  if (sensorDer.medir()) {
    motorDer.setIntensidad(intensidadParaDistancia(sensorDer.distancia()));
  }

  motorIzq.actualizar();
  motorDer.actualizar();

  Serial.print("IZQ: ");
  Serial.print(sensorIzq.distancia());
  Serial.print(" cm | PWM: ");
  Serial.print(motorIzq.pwmActual());

  Serial.print(" || DER: ");
  Serial.print(sensorDer.distancia());
  Serial.print(" cm | PWM: ");
  Serial.println(motorDer.pwmActual());

  unsigned long tiempoLoop = millis() - inicioLoop;
  if (tiempoLoop < LOOP_PERIODO_MS) {
    delay(LOOP_PERIODO_MS - tiempoLoop);
  }
}