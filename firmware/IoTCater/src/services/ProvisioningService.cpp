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
        _previousSsid = ssid;
        _previousPassword = password;
        _wifiService.begin(ssid, password);
        return;
    }

    _wifiService.disconnect();
    startPortal();
}

void ProvisioningService::update()
{
    if (!_active &&
        _wifiService.getConsecutiveConnectionAttempts() >= MAX_WIFI_CONNECTION_ATTEMPTS)
    {
        Serial.println("[ProvisioningService] Limite de intentos WiFi alcanzado. Activando portal de recuperacion.");
        startPortal();
    }

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
            _previousSsid = _candidateSsid;
            _previousPassword = _candidatePassword;
            _connecting = false;
            _provisioned = true;
            _lastError = "";
            Serial.println("[ProvisioningService] WiFi configurado correctamente.");
            return;
        }

        restorePreviousWifi("No se pudieron guardar las credenciales. Se mantiene la configuracion anterior.");
        return;
    }

    if (millis() - _connectionStartedAt >= CONNECTION_TIMEOUT_MS)
    {
        restorePreviousWifi("No fue posible conectarse a esa red. Se mantiene la configuracion anterior.");
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

void ProvisioningService::resetWifi()
{
    _connecting = false;
    _provisioned = false;
    _candidateSsid = "";
    _candidatePassword = "";
    _previousSsid = "";
    _previousPassword = "";
    _lastError = "";

    if (!_credentialsStorage.clear())
    {
        _lastError = "No se pudieron borrar las credenciales WiFi.";
        return;
    }

    _wifiService.disconnect();
    WiFi.mode(WIFI_AP_STA);
    startPortal();
    Serial.println("[ProvisioningService] Credenciales WiFi eliminadas.");
}

void ProvisioningService::startPortal()
{
    if (_active)
    {
        return;
    }

    _accessPointSsid = "CatFeeder-setup";
    _accessPointPassword = "catfeeder";

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

    // El escaneo se inicia bajo demanda desde el portal. No se ejecuta al
    // arrancar el equipo para evitar interferir con la conexion HTTPS.

    Serial.print("[ProvisioningService] Portal activo: http://");
    Serial.print(WiFi.softAPIP());
    Serial.println(":8080");
    Serial.print("[ProvisioningService] AP: ");
    Serial.println(_accessPointSsid);
    Serial.print("[ProvisioningService] Password AP: ");
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
    _server.on("/reset", HTTP_POST, [this]() { handleResetWifi(); });
    _server.onNotFound([this]() { handleNotFound(); });
    _routesRegistered = true;
}

void ProvisioningService::startNetworkScan()
{
    const int previousScan = WiFi.scanComplete();
    if (previousScan == WIFI_SCAN_RUNNING)
    {
        return;
    }

    if (previousScan >= 0)
    {
        WiFi.scanDelete();
    }

    WiFi.scanNetworks();
}

void ProvisioningService::handleRoot()
{
    if (WiFi.scanComplete() == WIFI_SCAN_FAILED)
    {
        startNetworkScan();
    }

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

    _wifiService.disconnect();
    WiFi.mode(WIFI_AP_STA);
    _wifiService.begin(_candidateSsid, _candidatePassword, true);

    _server.send(202, "text/html",
                 "<html><body><p>Conectando a la nueva red. Espere hasta 15 segundos y vuelva al portal.</p></body></html>");
}

void ProvisioningService::handleResetWifi()
{
    resetWifi();
    _server.sendHeader("Location", "/");
    _server.send(303);
}

void ProvisioningService::handleNotFound()
{
    _server.sendHeader("Location", "/");
    _server.send(302, "text/plain", "");
}

String ProvisioningService::buildPortalPage() const
{
    String page = "<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>";
    page += "<title>CatFeeder - WiFi</title></head><body><h1>Configurar CatFeeder</h1>";
    page += "<p>AP: <strong>CatFeeder-setup</strong></p><p>Password AP: <strong>catfeeder</strong></p>";

    if (_wifiService.isConnected())
    {
        page += "<p>Estado: conectado a <strong>" + WiFi.SSID() + "</strong></p>";
        page += "<p>IP: " + WiFi.localIP().toString() + "</p>";
    }
    else
    {
        page += "<p>Estado: no conectado a una red WiFi.</p>";
    }

    if (_connecting)
    {
        page += "<p>Probando nueva conexion WiFi...</p>";
    }
    if (!_lastError.isEmpty())
    {
        page += "<p>" + _lastError + "</p>";
    }

    page += "<form method='post' action='/configure'><label>Red WiFi</label><select name='network'>";
    const int networks = WiFi.scanComplete();
    if (networks >= 0)
    {
        for (int i = 0; i < networks; ++i)
        {
            page += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + "</option>";
        }
    }
    page += "</select><p>Si no aparece, escriba el SSID:</p><input name='manualSsid' maxlength='32'>";
    page += "<p>Password WiFi (deje vacio para red abierta):</p><input name='password' type='password' maxlength='63'>";
    page += "<p><button type='submit'>Conectar y guardar</button></p></form>";
    page += "<form method='post' action='/reset'><button type='submit'>Borrar configuracion WiFi</button></form>";
    page += "<p>Recargue la pagina para actualizar las redes detectadas.</p></body></html>";
    return page;
}

void ProvisioningService::restorePreviousWifi(const String& error)
{
    _connecting = false;
    _lastError = error;
    _wifiService.disconnect();

    if (!_previousSsid.isEmpty())
    {
        WiFi.mode(WIFI_AP_STA);
        _wifiService.begin(_previousSsid, _previousPassword, true);
    }
    else
    {
        WiFi.mode(WIFI_AP_STA);
    }
}
