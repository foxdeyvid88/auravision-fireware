#ifndef SUAVIZADO_H
#define SUAVIZADO_H

// Filtro exponencial puro del motor. Sin dependencias de hardware
// para poder probarlo en host.
// Si el objetivo es 0 apaga al instante; si no, acerca el valor
// actual al objetivo con el factor indicado (0 < factor < 1).
float suavizarPwm(float actual, int objetivo, float factor);

#endif