
#include <Arduino.h>
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
#include "services/RemoteStateService.h"

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
ApiClient apiClient(httpClient, deviceInfo, backendConnectionService);
RemoteCommandStorage remoteCommandStorage;
OtaService otaService(motor);
WebServer webServer(motor, wifi, feedingService, scheduler, configuration, storage, otaService);
HeartbeatService heartbeatService(apiClient, wifi);
ButtonService buttonService(feedingService, D0);
SyncService syncService(apiClient, feedingHistoryService, timeService, wifi);
ProvisioningService provisioningService(wifi, wifiCredentialsStorage);
ConfigurationRevisionStorage configurationRevisionStorage;
ConfigurationSyncService configurationSyncService(apiClient, storage, configurationRevisionStorage,
                                                  configuration);
RemoteStateService remoteStateService(apiClient, feedingService, wifi, remoteCommandStorage,
                                      configurationSyncService);
void handleWifiConnected()
{
    deviceInfo.printBootInfo();

    RegistrationResult result = apiClient.registerDevice();

    switch (result)
    {
        case RegistrationResult::Registered:
            Serial.println("[ApiClient] Dispositivo registrado correctamente.");
            break;

        case RegistrationResult::AlreadyRegistered:
            Serial.println("[ApiClient] Dispositivo ya registrado.");
            break;

        case RegistrationResult::Unauthorized:
            Serial.println("[ApiClient] Error de autenticacion.");
            break;

        case RegistrationResult::InvalidData:
            Serial.println("[ApiClient] Datos de registro invalidos.");
            break;

        case RegistrationResult::ServerError:
            Serial.println("[ApiClient] Error del servidor.");
            break;

        case RegistrationResult::ConnectionError:
            Serial.println("[ApiClient] Error de conexion.");
            break;
    }
}

bool hasWifiJustConnected()
{
    static ConnectionState previousState = ConnectionState::Disconnected;

    ConnectionState currentState = wifi.getConnectionState();

    bool connected =
        currentState == ConnectionState::Connected && previousState != ConnectionState::Connected;

    previousState = currentState;

    return connected;
}

void setup()
{
    Serial.begin(115200);
    motor.begin();
    storage.begin();
    timeService.begin();
    backendConnectionService.begin();

    configuration = storage.loadConfiguration(Configuration{});
    motor.setStepsPerFeed(configuration.stepsPerFeed);

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

    // No ejecutar tareas de red bloqueantes mientras el motor alimenta.
    if (!motor.isFeeding())
    {
        remoteStateService.update();
        heartbeatService.update();
        syncService.update();
    }
}
