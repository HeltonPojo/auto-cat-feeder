#include <ArduinoJWT.h>
#include <ArduinoJson.h>
#include <WebServer.h>

#include <secrets.h>

String gerarToken(String name) {
  ArduinoJWT jwt(SECRET_KEY);

  StaticJsonDocument<256> payloadDoc;
  payloadDoc["name"] = name;
  payloadDoc["exp"] = time(nullptr) + (3600 * 24 * 7);

  String payload;
  serializeJson(payloadDoc, payload);

  String token = jwt.encodeJWT(payload);
  return token;
}

bool validarToken(String token, String &nomeUsuario) {
  ArduinoJWT jwt(SECRET_KEY);

  String payload;

  if (!jwt.decodeJWT(token, payload)) {
    return false;
  }

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    return false;
  }

  if (!doc["exp"].isNull()) {
    unsigned long exp = doc["exp"].as<unsigned long>();
    if (time(nullptr) > exp) {
      return false;
    }
  }

  if (doc["name"].isNull()) {
    return false;
  }

  nomeUsuario = (const char *)doc["name"];
  return true;
}

String extrairToken(WebServer &server) {
  if (!server.hasHeader("Authorization")) {
    return "";
  }

  String auth = server.header("Authorization");

  if (auth.startsWith("Bearer ")) {
    return auth.substring(7);
  }

  return "";
}

bool verificarAuth(WebServer &server, String &nomeUsuario) {
  String token = extrairToken(server);

  if (token == "") {
    server.send(401, "application/json", "{\"error\":\"Token não enviado\"}");
    return false;
  }

  if (!validarToken(token, nomeUsuario)) {
    server.send(401, "application/json",
                "{\"error\":\"Token inválido ou expirado\"}");
    return false;
  }

  return true;
}
