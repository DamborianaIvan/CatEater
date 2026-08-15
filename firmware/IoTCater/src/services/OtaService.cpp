#include "services/OtaService.h"

OtaService::OtaService(Motor& motor) : _motor(motor) {}

void OtaService::handlePage(ESP8266WebServer& server)
{
    const char* page = R"html(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>CatFeeder OTA</title>
</head>
<body>
    <h1>CatFeeder OTA</h1>

    <form method="POST"
          action="/update"
          enctype="multipart/form-data">

        <input type="file"
               name="firmware"
               accept=".bin"
               required>

        <button type="submit">
            Actualizar firmware
        </button>
    </form>
</body>
</html>
)html";

    server.send(200, "text/html", page);
}

void OtaService::handleUpdate(ESP8266WebServer& server)
{
    if (_motor.isFeeding())
    {
        server.send(409, "application/json", R"({"success":false,"message":"Motor is feeding"})");

        return;
    }

    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START)
    {
        _updateFailed = false;
        Serial.printf("[OtaService] Iniciando OTA: %s\n", upload.filename.c_str());
        const size_t updateSize = upload.contentLength;

        if (!Update.begin(updateSize))
        {
            Serial.printf("[OtaService] Error iniciando OTA: %s\n",
                          Update.getErrorString().c_str());

            return;
        }
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
        {
            _updateFailed = true;
            Serial.printf("[OtaService] Error escribiendo OTA: %s\n",
                          Update.getErrorString().c_str());
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (_updateFailed)
        {
            Serial.println("[OtaService] OTA cancelada por error de escritura.");

            server.send(500, "application/json",
                        R"({"success":false,"message":"OTA failed during write"})");

            return;
        }

        if (Update.end(true))
        {
            Serial.printf("[OtaService] OTA completada. Bytes: %u\n", upload.totalSize);

            server.send(200, "application/json",
                        R"({"success":true,"message":"Firmware updated. Restarting..."})");

            delay(500);
            ESP.restart();
        }
        else
        {
            Serial.printf("[OtaService] Error finalizando OTA: %s\n",
                          Update.getErrorString().c_str());

            server.send(500, "application/json", R"({"success":false,"message":"OTA failed"})");
        }
    }
    else if (upload.status == UPLOAD_FILE_ABORTED)
    {
        Serial.println("[OtaService] OTA cancelada.");

        server.send(400, "application/json", R"({"success":false,"message":"OTA aborted"})");
    }
}