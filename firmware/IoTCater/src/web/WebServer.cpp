#include "web/WebServer.h"
#include "web/Pages.h"
#include "services/WifiService.h"
#include "storage/ConfigurationStorage.h"
#include <ArduinoJson.h>
#include "services/Scheduler.h"

WebServer::WebServer(Motor& motor, WiFiService& wifi, Scheduler& scheduler,
                     Configuration& configuration, ConfigurationStorage& storage)
    : _motor(motor),
      _wifi(wifi),
      _storage(storage),
      _configuration(configuration),
      _scheduler(scheduler)
{
}

void WebServer::begin()
{
    registerRoutes();
    _server.begin();
    Serial.println("[WebServer] Iniciado.");
}

void WebServer::update()
{
    _server.handleClient();
}

void WebServer::registerRoutes()
{
    _server.on("/", [this]() { handleHome(); });

    _server.onNotFound([this]() { handleNotFound(); });

    _server.on("/feed", HTTP_POST, [this]() { handleFeed(); });

    _server.on("/status", [this]() { handleStatus(); });

    _server.on("/config", HTTP_PUT, [this]() { handleUpdateConfig(); });

    _server.on("/config", HTTP_GET, [this]() { handleGetConfiguration(); });
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
    bool accepted = _motor.feed();

    if (accepted)
    {
        _server.send(200, "application/json",
                     R"json(
{
    "success": true,
    "message": "Feeding started"
}
)json");
    }
    else
    {
        _server.send(409, "application/json",
                     R"json(
{
    "success": false,
    "message": "Motor is busy"
}
)json");
    }
}

void WebServer::handleStatus()
{
    const bool feeding = _motor.isFeeding();
    const bool wifiConnected = _wifi.isConnected();
    const String ipAddress = _wifi.getIpAddress();

    String response = "{";
    response += "\"feeding\": ";
    response += feeding ? "true" : "false";
    response += ",";

    response += "\"wifiConnected\": ";
    response += wifiConnected ? "true" : "false";
    response += ",";

    response += "\"ipAddress\": ";
    response += "\"" + ipAddress + "\"";

    response += "}";

    _server.send(200, "application/json", response);
}

void WebServer::handleUpdateConfig()
{
    String body = _server.arg("plain");

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);

    if (error)
    {
        _server.send(400, "application/json",
                     R"({
                "success": false,
                "message": "Invalid JSON"
            })");
        return;
    }

    if (!doc["stepsPerFeed"].is<int>())
    {
        _server.send(400, "application/json",
                     R"({
                "success": false,
                "message": "Invalid stepsPerFeed"
            })");
        return;
    }

    if (!doc["schedules"].is<JsonArray>())
    {
        _server.send(400, "application/json",
                     R"({
                "success": false,
                "message": "Invalid schedules"
            })");
        return;
    }

    JsonArray schedules = doc["schedules"].as<JsonArray>();

    if (schedules.size() != MAX_SCHEDULES)
    {
        _server.send(400, "application/json",
                     R"({
                "success": false,
                "message": "Invalid schedules count"
            })");
        return;
    }

    Configuration newConfiguration = _configuration;

    newConfiguration.stepsPerFeed = doc["stepsPerFeed"];

    for (uint8_t i = 0; i < MAX_SCHEDULES; i++)
    {
        JsonObject schedule = schedules[i];

        if (!schedule["hour"].is<int>() || !schedule["minute"].is<int>() ||
            !schedule["portions"].is<int>() || !schedule["enabled"].is<bool>())
        {
            _server.send(400, "application/json",
                         R"({
                    "success": false,
                    "message": "Missing schedule fields"
                })");
            return;
        }

        int hour = schedule["hour"];
        int minute = schedule["minute"];
        int portions = schedule["portions"];
        bool enabled = schedule["enabled"];

        if (hour < 0 || hour > 23)
        {
            _server.send(400, "application/json",
                         R"({
                    "success": false,
                    "message": "Invalid hour"
                })");
            return;
        }

        if (minute < 0 || minute > 59)
        {
            _server.send(400, "application/json",
                         R"({
                    "success": false,
                    "message": "Invalid minute"
                })");
            return;
        }

        if (portions <= 0)
        {
            _server.send(400, "application/json",
                         R"({
                    "success": false,
                    "message": "Invalid portions"
                })");
            return;
        }

        newConfiguration.schedules[i].hour = hour;
        newConfiguration.schedules[i].minute = minute;
        newConfiguration.schedules[i].portions = portions;
        newConfiguration.schedules[i].enabled = enabled;
    }

    // Validacion de horarios duplicados
    for (uint8_t i = 0; i < MAX_SCHEDULES; ++i)
    {
        if (!newConfiguration.schedules[i].enabled)
        {
            continue;
        }

        for (uint8_t j = i + 1; j < MAX_SCHEDULES; ++j)
        {
            if (!newConfiguration.schedules[j].enabled)
            {
                continue;
            }

            if (newConfiguration.schedules[i].hour == newConfiguration.schedules[j].hour &&
                newConfiguration.schedules[i].minute == newConfiguration.schedules[j].minute)
            {
                _server.send(400, "application/json",
                             R"({
                        "success": false,
                        "message": "Duplicate schedules are not allowed"
                    })");

                return;
            }
        }
    }

    if (!_motor.setStepsPerFeed(newConfiguration.stepsPerFeed))
    {
        _server.send(400, "application/json",
                     R"({
                "success": false,
                "message": "Invalid stepsPerFeed"
            })");
        return;
    }

    _configuration = newConfiguration;

    if (!_storage.saveConfiguration(_configuration))
    {
        _server.send(500, "application/json",
                     R"({
                "success": false,
                "message": "Failed to save configuration"
            })");
        return;
    }

    _server.send(200, "application/json",
                 R"({
                    "success": true,
                    "message": "Configuration updated"
                 })");
}

bool WebServer::isValidSchedule(int hour, int minute, int portions) const
{
    return hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59 && portions > 0;
}

void WebServer::handleGetConfiguration()
{
    JsonDocument doc;

    doc["stepsPerFeed"] = _configuration.stepsPerFeed;

    JsonArray schedules = doc["schedules"].to<JsonArray>();

    for (uint8_t i = 0; i < MAX_SCHEDULES; i++)
    {
        JsonObject schedule = schedules.add<JsonObject>();

        schedule["hour"] = _configuration.schedules[i].hour;
        schedule["minute"] = _configuration.schedules[i].minute;
        schedule["portions"] = _configuration.schedules[i].portions;
        schedule["enabled"] = _configuration.schedules[i].enabled;
    }

    String response;
    serializeJson(doc, response);

    _server.send(200, "application/json", response);
}