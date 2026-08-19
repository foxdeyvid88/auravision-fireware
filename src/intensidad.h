#ifndef INTENSIDAD_H
#define INTENSIDAD_H

// Regla de negocio pura: distancia (cm) -> intensidad de vibración (0..255).
// Sin dependencias de hardware para poder probarla en host.
int intensidadParaDistancia(float distancia);

#endif