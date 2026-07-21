#include "WebServer.h"
#include "Pages.h"

WebServer::WebServer(Motor& motor)
    : _motor(motor)
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