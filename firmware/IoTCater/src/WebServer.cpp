#include "WebServer.h"
#include "Pages.h"

WebServer::WebServer(Motor& motor,
                     WiFiService& wifi)
    : _motor(motor),
      _wifi(wifi)
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