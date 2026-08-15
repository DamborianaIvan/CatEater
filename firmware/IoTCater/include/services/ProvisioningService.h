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
    WiFiService& _wifiService;
    WifiCredentialsStorage& _credentialsStorage;
    ESP8266WebServer _server{80};
    DNSServer _dnsServer;

    bool _active = false;
    bool _connecting = false;
    bool _provisioned = false;
    bool _routesRegistered = false;
    unsigned long _connectionStartedAt = 0;
    String _candidateSsid;
    String _candidatePassword;
    String _lastError;
    String _accessPointSsid;
    String _accessPointPassword;

    static constexpr unsigned long CONNECTION_TIMEOUT_MS = 15000;

    void startPortal();
    void stopPortal();
    void registerRoutes();
    void startNetworkScan();
    void handleRoot();
    void handleConfigure();
    void handleNotFound();
    String buildPortalPage() const;
};
