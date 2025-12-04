#ifndef MODELS_H
#define MODELS_H

#include <Arduino.h>

struct Horario {
  int id;
  String hora;
  String creator;
};

extern Horario horarios[50];
extern int totalHorarios;
extern int proximoId;

#endif
