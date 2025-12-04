#ifndef AUTH_H
#define AUTH_H

#include <Arduino.h>
#include <WebServer.h>

String gerarToken(String nome);
bool validarToken(String token, String &nomeUsuario);
String extrairToken(WebServer &server);
bool verificarAuth(WebServer &server, String &nomeUsuario);

#endif
