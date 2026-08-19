#include <unity.h>
#include "suavizado.h"
#include "../../src/suavizado.cpp"

void setUp(void) {}

void tearDown(void) {}

void test_objetivo_cero_apaga_al_instante() {
  TEST_ASSERT_EQUAL_FLOAT(0.0f, suavizarPwm(0.0f, 0, 0.75f));
  TEST_ASSERT_EQUAL_FLOAT(0.0f, suavizarPwm(200.0f, 0, 0.75f));
  TEST_ASSERT_EQUAL_FLOAT(0.0f, suavizarPwm(-1.0f, 0, 0.75f));
}

void test_avanza_hacia_el_objetivo() {
  TEST_ASSERT_EQUAL_FLOAT(168.75f, suavizarPwm(0.0f, 225, 0.75f));
  TEST_ASSERT_EQUAL_FLOAT(212.5f, suavizarPwm(175.0f, 225, 0.75f));
  TEST_ASSERT_EQUAL_FLOAT(225.0f, suavizarPwm(225.0f, 225, 0.75f));
}

void test_converge_hacia_el_objetivo() {
  float pwm = 0.0f;
  for (int i = 0; i < 1000; i++) {
    pwm = suavizarPwm(pwm, 225, 0.75f);
  }
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 225.0f, pwm);
}

void test_no_sobrepasa_el_objetivo() {
  float pwm = 0.0f;
  for (int i = 0; i < 100; i++) {
    pwm = suavizarPwm(pwm, 130, 0.75f);
    TEST_ASSERT_TRUE(pwm <= 130.0f);
  }
}

void test_baja_hacia_un_objetivo_menor() {
  TEST_ASSERT_EQUAL_FLOAT(141.25f, suavizarPwm(175.0f, 130, 0.75f));
}

void test_factor_cero_mantiene_valor() {
  TEST_ASSERT_EQUAL_FLOAT(100.0f, suavizarPwm(100.0f, 225, 0.0f));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_objetivo_cero_apaga_al_instante);
  RUN_TEST(test_avanza_hacia_el_objetivo);
  RUN_TEST(test_converge_hacia_el_objetivo);
  RUN_TEST(test_no_sobrepasa_el_objetivo);
  RUN_TEST(test_baja_hacia_un_objetivo_menor);
  RUN_TEST(test_factor_cero_mantiene_valor);
  return UNITY_END();
}