#include <Arduino.h>

#if __has_include("config/FactoryDeviceCredential.local.h")
#include "config/FactoryDeviceCredential.local.h"
#endif

#include "hardware/Motor.h"
#include "services/WifiService.h"
#include "web/WebServer.h"
#include "storage/ConfigurationStorage.h"
#include "services/TimeService.h"
#include "services/ProvisioningService.h"
#include "storage/WifiCredentialsStorage.h"
#include "services/Scheduler.h"
#include "domain/Configuration.h"
#include "device/DeviceInfo.h"
#include "network/HttpClient.h"
#include "network/ApiClient.h"
#include "services/BackendConnectionService.h"
#include "services/RemoteStateService.h"
#include "services/HeartbetService.h"
#include "services/ButtonService.h"
#include "services/FeedingService.h"
#include "services/FeedingHistoryService.h"
#include "services/SyncService.h"
#include "services/OtaService.h"
#include "storage/DeviceCredentialStorage.h"

Motor motor;
FeedingHistoryService feedingHistoryService;
WiFiService wifi;
TimeService timeService(wifi);
FeedingService feedingService(motor, feedingHistoryService, timeService);
HttpClient httpClient;
Configuration configuration;
Scheduler scheduler(timeService, feedingService, configuration);
ConfigurationStorage storage;
WifiCredentialsStorage wifiCredentialsStorage;
DeviceInfo deviceInfo(wifi);
BackendConnectionService backendConnectionService;
DeviceCredentialStorage deviceCredentialStorage;
ApiClient apiClient(httpClient, deviceInfo, backendConnectionService, deviceCredentialStorage);
RemoteCommandStorage remoteCommandStorage;
OtaService otaService(motor);
WebServer webServer(motor, wifi, feedingService, otaService, backendConnectionService);
HeartbeatService heartbeatService(apiClient, wifi);
ButtonService buttonService(feedingService, D0);
SyncService syncService(apiClient, feedingHistoryService, timeService, wifi);
ProvisioningService provisioningService(wifi, wifiCredentialsStorage);
ConfigurationRevisionStorage configurationRevisionStorage;
ConfigurationSyncService configurationSyncService(apiClient, storage, configurationRevisionStorage,
                                                  configuration, motor);
RemoteStateService remoteStateService(apiClient, feedingService, wifi, remoteCommandStorage,
                                      configurationSyncService);

void provisionFactoryDeviceCredential()
{
#ifdef CATFEEDER_FACTORY_DEVICE_CREDENTIAL
    if (apiClient.hasDeviceCredential())
    {
        return;
    }

    const String deviceCredential = CATFEEDER_FACTORY_DEVICE_CREDENTIAL;

    if (!DeviceCredentialStorage::isValid(deviceCredential))
    {
        Serial.println("[Factory] Device credential invalida.");
        return;
    }

    if (deviceCredentialStorage.save(deviceCredential))
    {
        Serial.println("[Factory] Device credential almacenada correctamente.");
    }
    else
    {
        Serial.println("[Factory] No se pudo almacenar la device credential.");
    }
#endif
}

void handleWifiConnected()
{
    deviceInfo.printBootInfo();

    if (!apiClient.hasDeviceCredential())
    {
        Serial.println("[ApiClient] No hay device credential disponible.");
        Serial.println("[ApiClient] El dispositivo requiere provisioning de fabrica.");
        return;
    }

    Serial.println("[ApiClient] Device credential disponible. Iniciando operacion normal.");
}

bool hasWifiJustConnected()
{
    static ConnectionState previousState = ConnectionState::Disconnected;
    ConnectionState currentState = wifi.getConnectionState();
    bool connected = currentState == ConnectionState::Connected &&
                     previousState != ConnectionState::Connected;
    previousState = currentState;
    return connected;
}

void setup()
{
    Serial.begin(115200);
    motor.begin();
    storage.begin();
    deviceCredentialStorage.begin();
    timeService.begin();
    backendConnectionService.begin();

    configuration = storage.loadConfiguration(Configuration{});
    motor.setStepsPerFeed(configuration.stepsPerFeed);

    provisionFactoryDeviceCredential();

    buttonService.begin();
    scheduler.begin();
    heartbeatService.begin();
    feedingHistoryService.begin();
    syncService.begin();
    provisioningService.begin();
    if (!provisioningService.isActive())
    {
        webServer.begin();
    }
    remoteCommandStorage.begin();
    remoteStateService.loadCommand();
    configurationSyncService.begin();
}

void loop()
{
    motor.update();
    buttonService.update();
    provisioningService.update();

    if (provisioningService.consumeProvisioned())
    {
        webServer.begin();
    }

    wifi.update();

    if (hasWifiJustConnected())
    {
        handleWifiConnected();
    }

    timeService.update();
    scheduler.update();
    webServer.update();

    if (!motor.isFeeding())
    {
        remoteStateService.update();
        heartbeatService.update();
        syncService.update();
    }
}
