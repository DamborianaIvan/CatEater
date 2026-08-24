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
#include "storage/DeviceCredentialStorage.h"
#include "storage/BootstrapCredentialStorage.h"

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
BootstrapCredentialStorage bootstrapCredentialStorage;
ApiClient apiClient(httpClient, deviceInfo, backendConnectionService, deviceCredentialStorage,
                    bootstrapCredentialStorage);
RemoteCommandStorage remoteCommandStorage;
OtaService otaService(motor);
WebServer webServer(motor, wifi, feedingService, scheduler, configuration, storage, otaService,
                    backendConnectionService);
HeartbeatService heartbeatService(apiClient, wifi);
ButtonService buttonService(feedingService, D0);
SyncService syncService(apiClient, feedingHistoryService, timeService, wifi);
ProvisioningService provisioningService(wifi, wifiCredentialsStorage);
ConfigurationRevisionStorage configurationRevisionStorage;
ConfigurationSyncService configurationSyncService(apiClient, storage, configurationRevisionStorage,
                                                  configuration, motor);
RemoteStateService remoteStateService(apiClient, feedingService, wifi, remoteCommandStorage,
                                      configurationSyncService);

void handleWifiConnected()
{
    deviceInfo.printBootInfo();

    RegistrationResult registrationResult = apiClient.registerDevice();

    switch (registrationResult)
    {
        case RegistrationResult::Registered:
        {
            Serial.println("[ApiClient] Dispositivo registrado. Iniciando enrollment.");
            EnrollmentResult enrollmentResult = apiClient.enrollDevice();
            switch (enrollmentResult)
            {
                case EnrollmentResult::Enrolled:
                    Serial.println("[ApiClient] Dispositivo enrolado correctamente.");
                    break;
                case EnrollmentResult::AlreadyEnrolled:
                    Serial.println("[ApiClient] Dispositivo ya está enrolado.");
                    break;
                case EnrollmentResult::Unauthorized:
                    Serial.println("[ApiClient] Error de autenticacion durante enrollment.");
                    break;
                case EnrollmentResult::NotFound:
                    Serial.println("[ApiClient] Feeder no encontrado durante enrollment.");
                    break;
                case EnrollmentResult::ServerError:
                    Serial.println("[ApiClient] Error del servidor durante enrollment.");
                    break;
                case EnrollmentResult::ConnectionError:
                    Serial.println("[ApiClient] Error de conexion durante enrollment.");
                    break;
            }
            break;
        }
        case RegistrationResult::AlreadyRegistered:
        {
            if (apiClient.hasDeviceCredential())
            {
                Serial.println("[ApiClient] Dispositivo ya registrado y credencial disponible.");
                break;
            }

            Serial.println("[ApiClient] Dispositivo registrado sin credencial local. Iniciando enrollment.");
            EnrollmentResult enrollmentResult = apiClient.enrollDevice();
            switch (enrollmentResult)
            {
                case EnrollmentResult::Enrolled:
                    Serial.println("[ApiClient] Dispositivo enrolado correctamente.");
                    break;
                case EnrollmentResult::AlreadyEnrolled:
                    Serial.println("[ApiClient] Dispositivo ya está enrolado, pero la credencial local no está disponible.");
                    break;
                case EnrollmentResult::Unauthorized:
                    Serial.println("[ApiClient] Error de autenticacion durante enrollment.");
                    break;
                case EnrollmentResult::NotFound:
                    Serial.println("[ApiClient] Feeder no encontrado durante enrollment.");
                    break;
                case EnrollmentResult::ServerError:
                    Serial.println("[ApiClient] Error del servidor durante enrollment.");
                    break;
                case EnrollmentResult::ConnectionError:
                    Serial.println("[ApiClient] Error de conexion durante enrollment.");
                    break;
            }
            break;
        }
        case RegistrationResult::Unauthorized:
            Serial.println("[ApiClient] Error de autenticacion durante registro.");
            break;
        case RegistrationResult::InvalidData:
            Serial.println("[ApiClient] Datos de registro invalidos.");
            break;
        case RegistrationResult::ServerError:
            Serial.println("[ApiClient] Error del servidor durante registro.");
            break;
        case RegistrationResult::ConnectionError:
            Serial.println("[ApiClient] Error de conexion durante registro.");
            break;
    }
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
    bootstrapCredentialStorage.begin();
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

    if (!motor.isFeeding())
    {
        remoteStateService.update();
        heartbeatService.update();
        syncService.update();
    }
}
