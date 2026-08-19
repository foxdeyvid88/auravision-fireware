#include <unity.h>
#include "sensor_calculos.h"
#include "../../src/sensor_calculos.cpp"

void setUp(void) {}

void tearDown(void) {}

void test_orden_big_endian() {
  // 1000 mm = 0x03E8 -> 100.0 cm
  TEST_ASSERT_EQUAL_FLOAT(100.0f, distanciaDesdeBytes(0x03, 0xE8));
}

void test_conversion_mm_a_cm() {
  TEST_ASSERT_EQUAL_FLOAT(25.5f, distanciaDesdeBytes(0x00, 0xFF));
  TEST_ASSERT_EQUAL_FLOAT(2.5f, distanciaDesdeBytes(0x00, 25));
  TEST_ASSERT_EQUAL_FLOAT(3.0f, distanciaDesdeBytes(0x00, 30));
}

void test_rango_minimo_inclusivo() {
  TEST_ASSERT_EQUAL_FLOAT(2.0f, distanciaDesdeBytes(0x00, 20));
  TEST_ASSERT_EQUAL_FLOAT(-1.0f, distanciaDesdeBytes(0x00, 19));
  TEST_ASSERT_EQUAL_FLOAT(-1.0f, distanciaDesdeBytes(0x00, 0x00));
}

void test_rango_maximo_inclusivo() {
  TEST_ASSERT_EQUAL_FLOAT(450.0f, distanciaDesdeBytes(0x11, 0x94));
  TEST_ASSERT_EQUAL_FLOAT(-1.0f, distanciaDesdeBytes(0x11, 0x95));
  TEST_ASSERT_EQUAL_FLOAT(-1.0f, distanciaDesdeBytes(0xFF, 0xFF));
}

void test_bytes_swapped_invalidan() {
  // Si se invierte el orden, 0xE803 = 59395 mm -> fuera de rango
  TEST_ASSERT_EQUAL_FLOAT(-1.0f, distanciaDesdeBytes(0xE8, 0x03));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_orden_big_endian);
  RUN_TEST(test_conversion_mm_a_cm);
  RUN_TEST(test_rango_minimo_inclusivo);
  RUN_TEST(test_rango_maximo_inclusivo);
  RUN_TEST(test_bytes_swapped_invalidan);
  return UNITY_END();
}