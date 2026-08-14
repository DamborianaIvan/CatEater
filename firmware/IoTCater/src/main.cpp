#include <Arduino.h>
#include "Motor.h"
#include "Config.h"
#include "WifiServices.h"
#include "WebServer.h"
#include "ConfigurationStorage.h"
#include "TimeService.h"
#include "Scheduler.h"
#include "Configuration.h"
#include "device/DeviceInfo.h"
#include "network/HttpClient.h"
#include "network/ApiClient.h"
#include "services/RemoteStateService.h"
#include "HeartbetServices.h"
#include "ButtonService.h"
#include "FeedingService.h"
#include "FeedingHistoryService.h"
#include "SyncService.h"
#include "TimeService.h"

Motor motor;
FeedingHistoryService feedingHistoryService;
WiFiService wifi;
TimeService timeService(wifi);
FeedingService feedingService(motor, feedingHistoryService, timeService);
HttpClient httpClient;
Configuration configuration;
Scheduler scheduler(timeService, feedingService, configuration);
ConfigurationStorage storage;
DeviceInfo deviceInfo(wifi);
ApiClient apiClient(httpClient, deviceInfo);
RemoteStateService remoteStateService(apiClient, feedingService, wifi);
WebServer webServer(motor, wifi, scheduler, configuration, storage);
HeartbeatService heartbeatService(apiClient);
ButtonService buttonService(feedingService, D0);
SyncService syncService(apiClient, feedingHistoryService, timeService);

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

    configuration = storage.loadConfiguration(Configuration{});
    motor.setStepsPerFeed(configuration.stepsPerFeed);

    wifi.begin(WIFI_SSID, WIFI_PASSWORD);
    buttonService.begin();
    scheduler.begin();
    webServer.begin();
    heartbeatService.begin();
    feedingHistoryService.begin();

    const auto history = feedingHistoryService.getHistory();

    Serial.println("========== Feeding History ==========");

    for (const auto& event : history)
    {
        Serial.print("Timestamp: ");
        Serial.println(event.timestamp);

        Serial.print("Portions: ");
        Serial.println(event.portions);

        Serial.print("Source: ");

        switch (event.source)
        {
            case FeedingSource::Physical:
                Serial.println("physical");
                break;

            case FeedingSource::Scheduled:
                Serial.println("scheduled");
                break;

            case FeedingSource::Remote:
                Serial.println("remote");
                break;
        }

        Serial.print("Synced: ");
        Serial.println(event.synced ? "true" : "false");

        Serial.println("--------------------------------------");
    }
}

void loop()
{
    motor.update();
    buttonService.update();

    wifi.update();
    if (hasWifiJustConnected())
    {
        handleWifiConnected();
    }

    timeService.update();

    scheduler.update();
    remoteStateService.update();
    webServer.update();
    heartbeatService.update();
    syncService.update();
}