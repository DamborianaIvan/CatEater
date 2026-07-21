#include "WebServer.h"
#include "Pages.h"

WebServer::WebServer()
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