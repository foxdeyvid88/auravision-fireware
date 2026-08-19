#include <unity.h>
#include "intensidad.h"
#include "../../src/intensidad.cpp"

void setUp(void) {}

void tearDown(void) {}

void test_sin_lectura_o_fuera_de_rango() {
  TEST_ASSERT_EQUAL_INT(0, intensidadParaDistancia(-1.0f));
  TEST_ASSERT_EQUAL_INT(0, intensidadParaDistancia(0.0f));
  TEST_ASSERT_EQUAL_INT(0, intensidadParaDistancia(10.0f));
  TEST_ASSERT_EQUAL_INT(0, intensidadParaDistancia(19.9f));
  TEST_ASSERT_EQUAL_INT(0, intensidadParaDistancia(180.1f));
  TEST_ASSERT_EQUAL_INT(0, intensidadParaDistancia(500.0f));
}

void test_zona_cercana_intensidad_alta() {
  TEST_ASSERT_EQUAL_INT(225, intensidadParaDistancia(20.0f));
  TEST_ASSERT_EQUAL_INT(225, intensidadParaDistancia(21.0f));
  TEST_ASSERT_EQUAL_INT(225, intensidadParaDistancia(79.9f));
}

void test_zona_media_intensidad_media() {
  TEST_ASSERT_EQUAL_INT(175, intensidadParaDistancia(80.0f));
  TEST_ASSERT_EQUAL_INT(175, intensidadParaDistancia(100.0f));
  TEST_ASSERT_EQUAL_INT(175, intensidadParaDistancia(139.9f));
}

void test_zona_lejana_intensidad_baja() {
  TEST_ASSERT_EQUAL_INT(130, intensidadParaDistancia(140.0f));
  TEST_ASSERT_EQUAL_INT(130, intensidadParaDistancia(150.0f));
  TEST_ASSERT_EQUAL_INT(130, intensidadParaDistancia(180.0f));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_sin_lectura_o_fuera_de_rango);
  RUN_TEST(test_zona_cercana_intensidad_alta);
  RUN_TEST(test_zona_media_intensidad_media);
  RUN_TEST(test_zona_lejana_intensidad_baja);
  return UNITY_END();
}