#include "web/WebServer.h"
#include "web/Pages.h"
#include "services/WifiService.h"
#include <ArduinoJson.h>

WebServer::WebServer(Motor& motor, WiFiService& wifi, FeedingService& feedingService,
                     OtaService& otaService,
                     const BackendConnectionService& backendConnectionService)
    : _motor(motor),
      _wifi(wifi),
      _feedingService(feedingService),
      _otaService(otaService),
      _backendConnectionService(backendConnectionService)
{
}

void WebServer::begin()
{
    if (_started)
    {
        return;
    }

    registerRoutes();
    _server.begin();
    _started = true;
    Serial.println("[WebServer] Iniciado.");
}

void WebServer::stop()
{
    if (!_started)
    {
        return;
    }

    _server.stop();
    _started = false;
}

void WebServer::update()
{
    if (_started)
    {
        _server.handleClient();
    }
}

void WebServer::registerRoutes()
{
    _server.on("/", [this]() { handleHome(); });
    _server.onNotFound([this]() { handleNotFound(); });
    _server.on("/feed", HTTP_POST, [this]() { handleFeed(); });
    _server.on("/status", [this]() { handleStatus(); });
    _server.on("/update", HTTP_GET, [this]() { _otaService.handlePage(_server); });
    _server.on("/update", HTTP_POST, [this]() {}, [this]() { _otaService.handleUpdate(_server); });
}

void WebServer::handleHome()
{
    _server.send(200, "text/html", HOME_PAGE);
}

void WebServer::handleNotFound()
{
    _server.send(404, "text/plain", "404 - Recurso no encontrado.");
}

void WebServer::handleFeed()
{
    int portions = 1;
    const String body = _server.arg("plain");

    if (!body.isEmpty())
    {
        JsonDocument doc;
        const DeserializationError error = deserializeJson(doc, body);

        if (error || !doc["portions"].is<int>())
        {
            _server.send(400, "application/json",
                         R"json({"success":false,"message":"Invalid portions"})json");
            return;
        }

        portions = doc["portions"].as<int>();
    }

    if (!Configuration::isValidPortions(portions))
    {
        _server.send(400, "application/json",
                     R"json({"success":false,"message":"Invalid portions"})json");
        return;
    }

    const bool accepted = _feedingService.feed(portions, FeedingSource::Physical);

    if (accepted)
    {
        _server.send(200, "application/json",
                     R"json({"success":true,"message":"Feeding started","portions":)json" +
                         String(portions) + "}");
    }
    else
    {
        _server.send(409, "application/json",
                     R"json({"success":false,"message":"Motor is busy"})json");
    }
}

void WebServer::handleStatus()
{
    const bool feeding = _motor.isFeeding();
    const bool wifiConnected = _wifi.isConnected();
    const bool backendAvailable = _backendConnectionService.isAvailable();
    const String ipAddress = _wifi.getIpAddress();

    String response = "{";
    response += "\"feeding\": ";
    response += feeding ? "true" : "false";
    response += ",";
    response += "\"wifiConnected\": ";
    response += wifiConnected ? "true" : "false";
    response += ",";
    response += "\"backendAvailable\": ";
    response += backendAvailable ? "true" : "false";
    response += ",";
    response += "\"ipAddress\": ";
    response += "\"" + ipAddress + "\"";
    response += "}";

    _server.send(200, "application/json", response);
}
