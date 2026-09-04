#pragma once

#include <DNSServer.h>
#include <ESP8266WebServer.h>

#include "services/WifiService.h"
#include "storage/WifiCredentialsStorage.h"

class ProvisioningService
{
   public:
    ProvisioningService(WiFiService& wifiService, WifiCredentialsStorage& credentialsStorage);

    void begin();
    void update();
    void resetWifi();

    bool isActive() const;
    bool consumeProvisioned();

   private:
    static constexpr uint16_t PORT = 8080;
    static constexpr unsigned long CONNECTION_TIMEOUT_MS = 15000;
    static constexpr unsigned int MAX_WIFI_CONNECTION_ATTEMPTS = 10;

    WiFiService& _wifiService;
    WifiCredentialsStorage& _credentialsStorage;
    ESP8266WebServer _server{PORT};
    DNSServer _dnsServer;

    bool _active = false;
    bool _connecting = false;
    bool _provisioned = false;
    bool _routesRegistered = false;
    unsigned long _connectionStartedAt = 0;
    String _candidateSsid;
    String _candidatePassword;
    String _previousSsid;
    String _previousPassword;
    String _lastError;
    String _accessPointSsid;
    String _accessPointPassword;

    void startPortal();
    void stopPortal();
    void registerRoutes();
    void startNetworkScan();
    void handleRoot();
    void handleConfigure();
    void handleResetWifi();
    void handleNotFound();
    void restorePreviousWifi(const String& error);
    String buildPortalPage() const;
};
