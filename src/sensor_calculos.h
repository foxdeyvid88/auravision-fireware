#ifndef SENSOR_CALCULOS_H
#define SENSOR_CALCULOS_H

#include <stdint.h>

// Conversión pura de los 2 bytes de respuesta del US-100 a distancia
// en cm. Sin dependencias de hardware para poder probarla en host.
// Devuelve -1 si la distancia queda fuera del rango válido (2..450 cm).
float distanciaDesdeBytes(uint8_t alta, uint8_t baja);

#endif