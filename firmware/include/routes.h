#ifndef ROUTES_H
#define ROUTES_H

#include <WebServer.h>

void handleRoot(WebServer &server);
void handleLogin(WebServer &server);
void handleGetSchedules(WebServer &server);
void handleCreateSchedule(WebServer &server);
void handleDeleteSchedule(WebServer &server);
void handleCORS(WebServer &server);
void configurarRotas(WebServer &server);

#endif
