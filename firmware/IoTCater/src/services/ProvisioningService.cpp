#include "services/ProvisioningService.h"

ProvisioningService::ProvisioningService(WiFiService& wifiService,
                                         WifiCredentialsStorage& credentialsStorage)
    : _wifiService(wifiService), _credentialsStorage(credentialsStorage)
{
}

void ProvisioningService::begin()
{
    _credentialsStorage.begin();

    String ssid;
    String password;
    if (_credentialsStorage.load(ssid, password))
    {
        _wifiService.begin(ssid, password);
        return;
    }

    _wifiService.disconnect(true);
    startPortal();
}

void ProvisioningService::update()
{
    if (!_active)
    {
        return;
    }

    _dnsServer.processNextRequest();
    _server.handleClient();

    if (!_connecting)
    {
        return;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        if (_credentialsStorage.save(_candidateSsid, _candidatePassword))
        {
            _connecting = false;
            _provisioned = true;
            _lastError = "";
            stopPortal();
            Serial.println("[ProvisioningService] WiFi configurado correctamente.");
            return;
        }

        _wifiService.disconnect();
        _connecting = false;
        _lastError = "No se pudieron guardar las credenciales. Reintente.";
        return;
    }

    if (millis() - _connectionStartedAt >= CONNECTION_TIMEOUT_MS)
    {
        _wifiService.disconnect();
        WiFi.mode(WIFI_AP_STA);
        _connecting = false;
        _lastError = "No fue posible conectarse a esa red. Revise los datos y reintente.";
        Serial.println("[ProvisioningService] Tiempo de conexion WiFi agotado.");
    }
}

bool ProvisioningService::isActive() const
{
    return _active;
}

bool ProvisioningService::consumeProvisioned()
{
    const bool provisioned = _provisioned;
    _provisioned = false;
    return provisioned;
}

void ProvisioningService::startPortal()
{
    if (_active)
    {
        return;
    }

    _accessPointSsid = "CatFeeder-setup";
    _accessPointPassword = String("CF-") + String(ESP.getChipId(), HEX) + "-setup";

    WiFi.mode(WIFI_AP_STA);
    if (!WiFi.softAP(_accessPointSsid.c_str(), _accessPointPassword.c_str()))
    {
        Serial.println("[ProvisioningService] Error iniciando punto de acceso.");
        return;
    }

    registerRoutes();
    _dnsServer.start(53, "*", WiFi.softAPIP());
    _server.begin();
    _active = true;
    startNetworkScan();

    Serial.print("[ProvisioningService] Portal activo: ");
    Serial.println(_accessPointSsid);
    Serial.println(_accessPointPassword);
}

void ProvisioningService::stopPortal()
{
    _dnsServer.stop();
    _server.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    _active = false;
}

void ProvisioningService::registerRoutes()
{
    if (_routesRegistered)
    {
        return;
    }

    _server.on("/", HTTP_GET, [this]() { handleRoot(); });
    _server.on("/configure", HTTP_POST, [this]() { handleConfigure(); });
    _server.onNotFound([this]() { handleNotFound(); });
    _routesRegistered = true;
}

void ProvisioningService::startNetworkScan()
{
    if (WiFi.scanComplete() == WIFI_SCAN_RUNNING)
    {
        return;
    }
    WiFi.scanNetworks(true);
}

void ProvisioningService::handleRoot()
{
    _server.send(200, "text/html", buildPortalPage());
}

void ProvisioningService::handleConfigure()
{
    String ssid = _server.arg("manualSsid");
    if (ssid.isEmpty())
    {
        ssid = _server.arg("network");
    }
    const String password = _server.arg("password");
    if (!WifiCredentialsStorage::areValid(ssid, password))
    {
        _lastError = "SSID invalido o password WiFi con longitud invalida.";
        _server.sendHeader("Location", "/");
        _server.send(303);
        return;
    }

    _candidateSsid = ssid;
    _candidatePassword = password;
    _lastError = "";
    _connecting = true;
    _connectionStartedAt = millis();
    _wifiService.begin(_candidateSsid, _candidatePassword, true);
    _server.send(202, "text/html",
                 "<html><body><p>Conectando. Espere hasta 15 segundos y recargue la "
                 "pagina.</p></body></html>");
}

void ProvisioningService::handleNotFound()
{
    _server.sendHeader("Location", "/");
    _server.send(302, "text/plain", "");
}

String ProvisioningService::buildPortalPage() const
{
    String page = "<!doctype html><html><body><h1>Configurar CatFeeder</h1>";
    if (_connecting)
    {
        page += "<p>Probando conexion WiFi...</p>";
    }
    if (!_lastError.isEmpty())
    {
        page += "<p>" + _lastError + "</p>";
    }

    page +=
        "<form method='post' action='/configure'><label>Red WiFi</label><select name='network'>";
    const int networks = WiFi.scanComplete();
    if (networks >= 0)
    {
        for (int i = 0; i < networks; ++i)
        {
            page += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + "</option>";
        }
    }
    page +=
        "</select><p>Si no aparece, escriba el SSID:</p><input name='manualSsid' maxlength='32'>";
    page +=
        "<p>Password (deje vacio para red abierta):</p><input name='password' type='password' "
        "maxlength='63'>";
    page += "<p><button type='submit'>Conectar</button></p></form>";
    page += "<p>Recargue para actualizar las redes detectadas.</p></body></html>";
    return page;
}
