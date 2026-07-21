#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "infrastructure/FeederWebServer.h"

namespace {
constexpr char kWifiName[] = "CatEater";
constexpr char kWifiPassword[] = "catfeeder";
constexpr uint16_t kMinimumPortions = 1;
constexpr uint16_t kMaximumPortions = 10;
}  // namespace

FeederWebServer::FeederWebServer(IoTCoreApplication& application, ULN2003FeederActuator& motor,
                                 IConfigStorage& configStorage)
    : server_(80), application_(application), motor_(motor), configStorage_(configStorage) {}

void FeederWebServer::begin() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(kWifiName, kWifiPassword);
  server_.on("/", HTTP_GET, [this]() { showHome(); });
  server_.on("/servir", HTTP_POST, [this]() { serveFood(); });
  server_.on("/calibrar", HTTP_GET, [this]() { showCalibration(); });
  server_.on("/calibrar", HTTP_POST, [this]() { saveCalibration(); });
  server_.begin();
}

void FeederWebServer::handleClient() {
  server_.handleClient();
}

void FeederWebServer::showHome() {
  static const char kPage[] PROGMEM = R"html(<!doctype html><html lang="es"><meta name="viewport" content="width=device-width,initial-scale=1"><title>CatEater</title><body><h1>CatEater</h1><p>Servir alimento</p><form action="/servir" method="post"><label>Porciones <input type="number" name="porciones" min="1" max="10" value="1"></label><button type="submit">Servir</button></form><p><a href="/calibrar">Calibrar porcion</a></p></body></html>)html";
  server_.send_P(200, "text/html; charset=utf-8", kPage);
}

void FeederWebServer::serveFood() {
  const long portions = server_.arg("porciones").toInt();
  if (portions < kMinimumPortions || portions > kMaximumPortions) {
    server_.send(400, "text/plain; charset=utf-8", "Cantidad de porciones invalida.");
    return;
  }

  if (application_.feed(static_cast<uint16_t>(portions))) {
    server_.send(200, "text/plain; charset=utf-8", "Orden enviada al motor.");
  } else {
    server_.send(409, "text/plain; charset=utf-8", "El motor esta ocupado.");
  }
}

void FeederWebServer::showCalibration() {
  String page = "<!doctype html><html lang='es'><meta name='viewport' content='width=device-width,initial-scale=1'><title>Calibrar CatEater</title><body><h1>Calibrar porcion</h1><p>Pasos actuales: " + String(motor_.stepsPerPortion()) + "</p><form action='/calibrar' method='post'><label>Pasos por porcion <input type='number' name='pasos' min='16' max='4096' value='" + String(motor_.stepsPerPortion()) + "'></label><button type='submit'>Guardar</button></form><p><a href='/'>Volver</a></p></body></html>";
  server_.send(200, "text/html; charset=utf-8", page);
}

void FeederWebServer::saveCalibration() {
  const long steps = server_.arg("pasos").toInt();
  if (steps < 16 || steps > 4096) {
    server_.send(400, "text/plain; charset=utf-8", "Los pasos deben estar entre 16 y 4096.");
    return;
  }

  DeviceConfig config;
  config.stepsPerPortion = static_cast<uint16_t>(steps);
  if (!configStorage_.save(config)) {
    server_.send(500, "text/plain; charset=utf-8", "No se pudo guardar la configuracion.");
    return;
  }
  motor_.setStepsPerPortion(config.stepsPerPortion);
  server_.send(200, "text/plain; charset=utf-8", "Calibracion guardada. Reiniciar no perdera este valor.");
}
