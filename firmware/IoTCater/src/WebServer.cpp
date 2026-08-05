#include "WebServer.h"
#include "Pages.h"
#include "WifiServices.h"
#include "ConfigurationStorage.h"
#include <ArduinoJson.h>
#include "Scheduler.h"

WebServer::WebServer(
    Motor& motor,
    WiFiService& wifi,
    Scheduler& scheduler,
    Configuration& configuration,
    ConfigurationStorage& storage)
    :
    _motor(motor),
    _wifi(wifi),
    _scheduler(scheduler),
    _configuration(configuration),
    _storage(storage)
{
}
void WebServer::begin()
{
    registerRoutes();
    _server.begin();
    Serial.println("Servidor Web iniciado.");
}

void WebServer::update()
{
    _server.handleClient();
}

void WebServer::registerRoutes()
{
    _server.on("/", [this]()
    {
        handleHome();
    });

    _server.onNotFound([this]()
    {
        handleNotFound();
    });

    _server.on("/feed", HTTP_POST, [this]()
    {
        handleFeed();
    });

    _server.on("/status", [this]()
    {
        handleStatus();
    });

    _server.on("/config", HTTP_PUT, [this]() {
        handleUpdateConfig();
    });

    _server.on("/config", HTTP_GET, [this]() {
        handleGetConfiguration();
    });
}   



void WebServer::handleHome()
{
    _server.send(
        200,
        "text/html",
        HOME_PAGE
    );
}

void WebServer::handleNotFound()
{
    _server.send(
        404,
        "text/plain",
        "404 - Recurso no encontrado."
    );
}

void WebServer::handleFeed()
{
    bool accepted = _motor.feed();

    if (accepted)
    {
        _server.send(
            200,
            "application/json",
            R"json(
{
    "success": true,
    "message": "Feeding started"
}
)json"
        );
    }
    else
    {
        _server.send(
            409,
            "application/json",
            R"json(
{
    "success": false,
    "message": "Motor is busy"
}
)json"
        );
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

    _server.send(
        200,
        "application/json",
        response
    );
}

void WebServer::handleUpdateConfig()
{
    //Leer el body
    String body = _server.arg("plain");

    //Parsear JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);

    if (error)
    {
        _server.send(
            400,
            "application/json",
            R"({
                "success": false,
                "message": "Invalid JSON"
            })"
        );
        return;
    }

    if (!doc.containsKey("stepsPerFeed"))
    {
        _server.send(
            400,
            "application/json",
            R"({
                "success": false,
                "message": "Falta el campo"
            })"
        );
        return;
      
    }

    if (!doc["stepsPerFeed"].is<int>())
    {
        _server.send(
            400,
            "application/json",
            R"({
                "success": false,
                "message": "Campo invalido"
            })"
        );
        return;
    }

    //Obtener el dato
    int steps = doc["stepsPerFeed"];

    //Intentar actualizar el Motor
    if (!_motor.setStepsPerFeed(steps))
    {
        _server.send(
            400,
            "application/json",
            R"({
                "success": false,
                "message": "Invalid stepsPerPortion"
            })"
        );
        return;
    }
    

    //Responder éxito
    _configuration.stepsPerFeed = steps;
    _storage.saveConfiguration(_configuration);
    _server.send(
        200,
        "application/json",
        R"({
            "success": true,
            "message": "Configuration updated"
        })"
    );
}

void WebServer::handleGetConfiguration(){
    int configuracionActual = _configuration.stepsPerFeed;

    String response = "{";
    response += "\"stepsPerFeed\": ";
    response += configuracionActual;
    response += "}";
    _server.send(
        200,
        "application/json",
        response);
      
}