#include "routes.h"
#include "auth.h"
#include "config.h"
#include "html.h"
#include "models.h"
#include "secrets.h"
#include <ArduinoJson.h>

Horario horarios[MAX_HORARIOS];
int totalHorarios = 0;
int proximoId = 1;

void handleRoot(WebServer &server) {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleCORS(WebServer &server) {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods",
                    "GET, POST, DELETE, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers",
                    "Content-Type, Authorization");
  server.send(204);
}

void handleLogin(WebServer &server) {
  server.sendHeader("Access-Control-Allow-Origin", "*");

  if (server.method() != HTTP_POST) {
    server.send(405, "application/json",
                "{\"error\":\"Método não permitido\"}");
    return;
  }

  String body = server.arg("plain");
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, body);

  if (error) {
    server.send(400, "application/json", "{\"error\":\"JSON inválido\"}");
    Serial.println("Erro ao parsear JSON: " + String(error.c_str()));
    return;
  }

  String nome = doc["name"] | "";
  String senha = doc["password"] | "";

  if (nome == "" || senha == "") {
    server.send(400, "application/json",
                "{\"error\":\"Nome e senha são obrigatórios\"}");
    return;
  }

  if (senha != SYSTEM_PASSWORD) {
    server.send(401, "application/json", "{\"error\":\"Senha incorreta\"}");
    Serial.println("Tentativa de login com senha incorreta: " + nome);
    return;
  }

  String token = gerarToken(nome);

  StaticJsonDocument<256> response;
  response["token"] = token;
  response["name"] = nome;

  String responseStr;
  serializeJson(response, responseStr);

  server.send(200, "application/json", responseStr);
  Serial.println("Login realizado com sucesso: " + nome);
}

void handleGetSchedules(WebServer &server) {
  server.sendHeader("Access-Control-Allow-Origin", "*");

  String nomeUsuario;
  if (!verificarAuth(server, nomeUsuario)) {
    return;
  }

  StaticJsonDocument<2048> doc;
  JsonArray array = doc.to<JsonArray>();

  for (int i = 0; i < totalHorarios; i++) {
    JsonObject obj = array.createNestedObject();
    obj["id"] = horarios[i].id;
    obj["hora"] = horarios[i].hora;
    obj["creator"] = horarios[i].creator;
  }

  String json;
  serializeJson(doc, json);

  server.send(200, "application/json", json);
}

void handleCreateSchedule(WebServer &server) {
  server.sendHeader("Access-Control-Allow-Origin", "*");

  String nomeUsuario;
  if (!verificarAuth(server, nomeUsuario)) {
    return;
  }

  if (totalHorarios >= MAX_HORARIOS) {
    server.send(400, "application/json",
                "{\"error\":\"Limite de horários atingido\"}");
    return;
  }

  String body = server.arg("plain");
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, body);

  if (error) {
    server.send(400, "application/json", "{\"error\":\"JSON inválido\"}");
    return;
  }

  String hora = doc["hora"] | "";

  if (hora == "") {
    server.send(400, "application/json", "{\"error\":\"Hora é obrigatória\"}");
    return;
  }

  // Criar novo horário
  horarios[totalHorarios].id = proximoId++;
  horarios[totalHorarios].hora = hora;
  horarios[totalHorarios].creator = nomeUsuario;
  totalHorarios++;

  // Criar resposta
  StaticJsonDocument<256> response;
  response["id"] = horarios[totalHorarios - 1].id;
  response["hora"] = hora;
  response["creator"] = nomeUsuario;

  String responseStr;
  serializeJson(response, responseStr);

  server.send(201, "application/json", responseStr);
  Serial.println("Horário criado: " + hora + " por " + nomeUsuario);
}

void handleDeleteSchedule(WebServer &server) {
  server.sendHeader("Access-Control-Allow-Origin", "*");

  String nomeUsuario;
  if (!verificarAuth(server, nomeUsuario)) {
    return;
  }

  String uri = server.uri();
  int lastSlash = uri.lastIndexOf('/');
  String idStr = uri.substring(lastSlash + 1);
  int id = idStr.toInt();

  bool encontrado = false;
  for (int i = 0; i < totalHorarios; i++) {
    if (horarios[i].id == id) {
      // Remover horário deslocando os elementos
      for (int j = i; j < totalHorarios - 1; j++) {
        horarios[j] = horarios[j + 1];
      }
      totalHorarios--;
      encontrado = true;
      break;
    }
  }

  if (encontrado) {
    server.send(200, "application/json",
                "{\"message\":\"Horário excluído com sucesso\"}");
    Serial.println("Horário " + String(id) + " excluído por " + nomeUsuario);
  } else {
    server.send(404, "application/json",
                "{\"error\":\"Horário não encontrado\"}");
  }
}

void configurarRotas(WebServer &server) {
  server.on("/", HTTP_GET, [&server]() { handleRoot(server); });

  server.on("/login", HTTP_POST, [&server]() { handleLogin(server); });
  server.on("/login", HTTP_OPTIONS, [&server]() { handleCORS(server); });

  server.on("/schedules", HTTP_GET,
            [&server]() { handleGetSchedules(server); });
  server.on("/schedules", HTTP_POST,
            [&server]() { handleCreateSchedule(server); });
  server.on("/schedules", HTTP_OPTIONS, [&server]() { handleCORS(server); });

  server.onNotFound([&server]() {
    if (server.uri().startsWith("/schedules/") &&
        server.method() == HTTP_DELETE) {
      handleDeleteSchedule(server);
    } else if (server.method() == HTTP_OPTIONS) {
      handleCORS(server);
    } else {
      server.send(404, "application/json",
                  "{\"error\":\"Rota não encontrada\"}");
    }
  });
}
