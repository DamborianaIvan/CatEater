#include "services/OtaService.h"

namespace
{
constexpr char OTA_USERNAME[] = "admin";
}

OtaService::OtaService(Motor& motor) : _motor(motor) {}

bool OtaService::isAuthorized(ESP8266WebServer& server) const
{
    // La contraseña se define mediante una macro local de compilación y nunca
    // debe almacenarse en el repositorio.
#ifdef CATFEEDER_OTA_PASSWORD
    if (!server.authenticate(OTA_USERNAME, CATFEEDER_OTA_PASSWORD))
    {
        server.requestAuthentication();
        return false;
    }

    return true;
#else
    Serial.println("[OtaService] OTA deshabilitada: falta CATFEEDER_OTA_PASSWORD.");
    server.send(503, "application/json", R"({"success":false,"message":"OTA not configured"})");
    return false;
#endif
}

void OtaService::handlePage(ESP8266WebServer& server)
{
    if (!isAuthorized(server))
    {
        return;
    }

    const char* page = R"html(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <title>CatFeeder OTA</title>
</head>
<body>
    <h1>CatFeeder OTA</h1>
    <form method="POST" action="/update" enctype="multipart/form-data">
        <input type="file" name="firmware" accept=".bin" required>
        <button type="submit">Actualizar firmware</button>
    </form>
</body>
</html>
)html";

    server.send(200, "text/html", page);
}

bool OtaService::beginUpdate(HTTPUpload& upload)
{
    _updateInProgress = false;
    _updateFailed = false;
    _expectedSize = upload.contentLength;
    _receivedSize = 0;

    if (_expectedSize == 0 || _expectedSize > MAX_FIRMWARE_SIZE)
    {
        Serial.printf("[OtaService] Tamaño de firmware inválido: %u bytes.\n",
                      static_cast<unsigned int>(_expectedSize));
        _updateFailed = true;
        return false;
    }

    if (!Update.begin(_expectedSize))
    {
        Serial.printf("[OtaService] Error iniciando OTA: %s\n",
                      Update.getErrorString().c_str());
        _updateFailed = true;
        return false;
    }

    _updateInProgress = true;
    Serial.printf("[OtaService] Iniciando OTA: %s (%u bytes)\n",
                  upload.filename.c_str(), static_cast<unsigned int>(_expectedSize));
    return true;
}

bool OtaService::writeUpdateChunk(HTTPUpload& upload)
{
    if (!_updateInProgress || _updateFailed)
    {
        return false;
    }

    if (_receivedSize + upload.currentSize > _expectedSize)
    {
        _updateFailed = true;
        Serial.println("[OtaService] El firmware recibido supera el tamaño declarado.");
        Update.abort();
        return false;
    }

    const size_t written = Update.write(upload.buf, upload.currentSize);

    if (written != upload.currentSize)
    {
        _updateFailed = true;
        Serial.printf("[OtaService] Error escribiendo OTA: %s\n",
                      Update.getErrorString().c_str());
        Update.abort();
        return false;
    }

    _receivedSize += written;
    return true;
}

bool OtaService::finishUpdate(ESP8266WebServer& server, HTTPUpload& upload)
{
    if (!_updateInProgress || _updateFailed)
    {
        Update.abort();
        _updateInProgress = false;
        server.send(500, "application/json", R"({"success":false,"message":"OTA failed"})");
        return false;
    }

    if (_receivedSize != _expectedSize || _receivedSize != upload.totalSize)
    {
        Serial.printf("[OtaService] Tamaño incompleto: recibido=%u esperado=%u total=%u\n",
                      static_cast<unsigned int>(_receivedSize),
                      static_cast<unsigned int>(_expectedSize),
                      static_cast<unsigned int>(upload.totalSize));
        Update.abort();
        _updateInProgress = false;
        server.send(400, "application/json", R"({"success":false,"message":"Incomplete firmware"})");
        return false;
    }

    if (!Update.end(true))
    {
        Serial.printf("[OtaService] Error finalizando OTA: %s\n",
                      Update.getErrorString().c_str());
        _updateInProgress = false;
        server.send(500, "application/json", R"({"success":false,"message":"OTA failed"})");
        return false;
    }

    _updateInProgress = false;
    Serial.printf("[OtaService] OTA completada. Bytes: %u\n",
                  static_cast<unsigned int>(_receivedSize));

    server.send(200, "application/json",
                R"({"success":true,"message":"Firmware updated. Restarting..."})");

    delay(500);
    ESP.restart();
    return true;
}

void OtaService::abortUpdate()
{
    if (_updateInProgress)
    {
        Update.abort();
    }

    _updateInProgress = false;
    _updateFailed = true;
    _expectedSize = 0;
    _receivedSize = 0;
}

void OtaService::handleUpdate(ESP8266WebServer& server)
{
    if (!isAuthorized(server))
    {
        return;
    }

    if (_motor.isFeeding())
    {
        server.send(409, "application/json", R"({"success":false,"message":"Motor is feeding"})");
        return;
    }

    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START)
    {
        if (!beginUpdate(upload))
        {
            server.send(400, "application/json", R"({"success":false,"message":"Invalid firmware size or OTA unavailable"})");
        }
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (!writeUpdateChunk(upload))
        {
            abortUpdate();
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        finishUpdate(server, upload);
    }
    else if (upload.status == UPLOAD_FILE_ABORTED)
    {
        abortUpdate();
        Serial.println("[OtaService] OTA cancelada.");
        server.send(400, "application/json", R"({"success":false,"message":"OTA aborted"})");
    }
}
