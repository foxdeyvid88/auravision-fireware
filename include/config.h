#ifndef CONFIG_H
#define CONFIG_H

// ==========================================
// PINES (XIAO ESP32-S3)
// ==========================================
#define S1_RX_PIN D0
#define S1_TX_PIN D1
#define S2_RX_PIN D2
#define S2_TX_PIN D3

#define MOTOR_IZQ_PIN D4
#define MOTOR_DER_PIN D5

#define LED_PIN LED_BUILTIN

// ==========================================
// SENSOR US-100 (UART)
// ==========================================
constexpr unsigned long SENSOR_BAUD = 9600;
constexpr unsigned long SENSOR_TIMEOUT_MS = 80;

// ==========================================
// MOTORES (PWM)
// ==========================================
constexpr int PWM_FREQ = 5000;
constexpr int PWM_RES = 8;
constexpr int PWM_CH_IZQ = 0;
constexpr int PWM_CH_DER = 1;
constexpr float FACTOR_SUAVIZADO = 0.75f;

// ==========================================
// LOOP
// ==========================================
constexpr unsigned long LOOP_PERIODO_MS = 83;  // ~12 lecturas/s

#endif