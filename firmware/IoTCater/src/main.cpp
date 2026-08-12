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

Motor motor;
WiFiService wifi;
TimeService timeService(wifi);
HttpClient httpClient;
Configuration configuration;
Scheduler scheduler(timeService, motor, configuration);
ConfigurationStorage storage;
DeviceInfo deviceInfo(wifi);
ApiClient apiClient(httpClient, deviceInfo);
RemoteStateService remoteStateService(apiClient, motor);
WebServer webServer(motor, wifi, scheduler, configuration, storage);
HeartbeatService heartbeatService(apiClient);
FeedingService feedingService(motor);
ButtonService buttonService(feedingService, D0);
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
}

void loop()
{
    wifi.update();
    if (hasWifiJustConnected())
    {
        handleWifiConnected();
    }
    timeService.update();
    scheduler.update();
    motor.update();
    remoteStateService.update();
    webServer.update();
    heartbeatService.update();
    buttonService.update();
}