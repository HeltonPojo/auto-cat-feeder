#include <Arduino.h>
#include <ESP32Servo.h>
#include <WebServer.h>

#include <config.h>
#include <secrets.h>

/*❚█══█❚▬▬ι═══════>-------------------------------------------------------------------------------------------------<═══════ι▬▬❚█══█❚*/
#define SPIN_R 0
#define SPIN_L 180
#define STOP 90

Servo servo;

// Wifi configuraton
WebServer server(80);

int lastButtonStage = HIGH;
int currentButtonStage;
bool spinning = false;

/*❚█══█❚▬▬ι═══════>-------------------------------------------------------------------------------------------------<═══════ι▬▬❚█══█❚*/

bool isAuthenticated() {
  if (server.hasHeader("Authorization")) {
    String authHeader = server.header("Authorization");
    String expectedAuth = "Bearer " + String(TOKEN);
    return authHeader == expectedAuth;
  }

  return false;
}

void handleFeed() {
  if (server.method() != HTTP_POST) {
    server.send(405, "application/json", "{\"error\": \"Wrong method\"}");
    return;
  }

  if (!isAuthenticated()) {
    server.send(401);
    return;
  }

  if (spinning) {
    server.send(201, "application/json", "{\"msg\": \"Already feeding\"}");
    return;
  }

  server.send(200, "application/json",
              "{\"msg\": \"Initializating feeding routing\"}");
  Serial.println("Iniciando a rotina de alimentação");
  digitalWrite(BUZZER_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);
  spinning = true;
  servo.write(SPIN_R);

  delay(2000);

  Serial.println("Finalizando a rotina de alimentação");
  servo.write(STOP);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  spinning = false;
  return;
}

/*❚█══█❚▬▬ι═══════>-------------------------------------------------------------------------------------------------<═══════ι▬▬❚█══█❚*/

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  servo.attach(SERVO_PIN);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.print("Conectando ao WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  server.on("/feed", handleFeed);

  server.begin();
  Serial.println("Sistema iniciado!");
}

void loop() {

  server.handleClient();
  // delay(50);
  // currentButtonStage = digitalRead(BUTTON_PIN);
  // if (currentButtonStage != lastButtonStage) {
  //   handleFeed();
  //   lastButtonStage = currentButtonStage;
  // }
}
