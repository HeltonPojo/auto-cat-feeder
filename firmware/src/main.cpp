#include <Arduino.h>
#include <ESP32Servo.h>
#include <WebServer.h>
#include <cronos.hpp>
#include <esp_sntp.h>

#include <config.h>
#include <htmlTemplates.h>
#include <secrets.h>

/*❚█══█❚▬▬ι═══════>-------------------------------------------------------------------------------------------------<═══════ι▬▬❚█══█❚*/
#define SPIN_R 0
#define SPIN_L 180
#define STOP 90

Servo servo;
CronoS cron;

const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";

const char *time_zone = "UTC+3";

const char *cron_schedule = "0 15 7,9,12,15,18,20,22 * * *";

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

void feed(cronos_tid id, void *arg) {
  if (spinning) {
    Serial.println("Rotina já iniciada");
    return;
  }

  Serial.println("Iniciando a rotina de alimentação");
  spinning = true;

  digitalWrite(BUZZER_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);
  servo.write(SPIN_R);

  delay(2000);
  spinning = false;

  Serial.println("Finalizando a rotina de alimentação");
  servo.write(STOP);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  return;
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void timeavailable(struct timeval *t) {
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
  // reevaluate rules on time change
  cron.reload();
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

  // REST-API
  // server.on("/feed", HTTP_POST, handleFeed);

  // Default cron jobs
  configTzTime(time_zone, ntpServer1, ntpServer2);

  sntp_set_time_sync_notification_cb(timeavailable);
  cron.addCallback(cron_schedule, feed);

  server.begin();

  Serial.println("Sistema iniciado!");
}

void loop() {}
