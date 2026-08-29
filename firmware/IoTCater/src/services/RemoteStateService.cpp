#include "services/RemoteStateService.h"

RemoteStateService::RemoteStateService(ApiClient& apiClient, FeedingService& feedingService,
                                       WiFiService& wifiService,
                                       RemoteCommandStorage& commandStorage,
                                       ConfigurationSyncService& configurationSyncService)
    : _apiClient(apiClient),
      _feedingService(feedingService),
      _wifiService(wifiService),
      _commandStorage(commandStorage),
      _configurationSyncService(configurationSyncService)
{
}

void RemoteStateService::update()
{
    if (!_wifiService.isConnected())
    {
        return;
    }

    // Remote state is best-effort. If the backend is unavailable, do not even
    // enter the polling path. Local feeding/scheduling must remain unaffected.
    if (!_apiClient.isBackendAvailable())
    {
        return;
    }

    const unsigned long now = millis();

    if (now - _lastRequest < POLL_INTERVAL)
    {
        return;
    }

    _lastRequest = now;
    pollMotorState();
}

bool RemoteStateService::loadCommand()
{
    _hasCommand = _commandStorage.load(_command);

    if (_hasCommand)
    {
        Serial.print("[RemoteStateService] Comando recuperado. ID: ");
        Serial.println(_command.commandId);
    }

    return true;
}

void RemoteStateService::pollMotorState()
{
    bool motorState = false;
    int portions = 1;
    String commandId;
    uint32_t configRevision = 0;

    if (!_apiClient.getMotorState(motorState, portions, commandId, configRevision))
    {
        Serial.println("[RemoteStateService] Error al consultar motor state.");
        return;
    }

    _configurationSyncService.update(configRevision);

    if (!commandId.isEmpty())
    {
        if (_hasCommand && commandId == _command.commandId)
        {
            if (_command.status == RemoteCommandStatus::Completed)
            {
                // Comando ya procesado. No hacer nada.
            }
            else if (_command.status == RemoteCommandStatus::Executing)
            {
                _remoteFeedInProgress = true;
                _activeCommandId = commandId;
            }
            else if (_command.status == RemoteCommandStatus::Pending)
            {
                processCommand(commandId, _command.portions);
            }
        }
        else
        {
            processCommand(commandId, portions);
        }
    }

    if (_remoteFeedInProgress && !_feedingService.isFeeding())
    {
        if (millis() - _lastConfirmationAttempt >= CONFIRMATION_RETRY_INTERVAL)
        {
            _lastConfirmationAttempt = millis();

            if (!_apiClient.isBackendAvailable())
            {
                Serial.println("[RemoteStateService] Backend no disponible. Confirmacion pendiente.");
                return;
            }

            Serial.println("[RemoteStateService] Confirmando alimentacion remota...");

            if (_apiClient.completeMotorCommand(_activeCommandId))
            {
                Serial.println("[RemoteStateService] Comando remoto confirmado.");

                _command.status = RemoteCommandStatus::Completed;

                if (!_commandStorage.save(_command))
                {
                    Serial.println("[RemoteStateService] Error guardando comando completado.");
                    return;
                }

                _remoteFeedInProgress = false;
                _activeCommandId = "";
            }
            else
            {
                Serial.println("[RemoteStateService] Confirmacion fallida. Reintentando...");
            }
        }
    }
}

bool RemoteStateService::processCommand(const String& commandId, int portions)
{
    if (commandId.isEmpty())
    {
        return false;
    }

    RemoteCommand command;

    command.commandId = commandId;
    command.portions = portions;
    command.status = RemoteCommandStatus::Pending;

    if (!_commandStorage.save(command))
    {
        Serial.println("[RemoteStateService] No se pudo persistir el comando.");
        return false;
    }

    _command = command;
    _hasCommand = true;

    Serial.print("[RemoteStateService] Nueva orden recibida. ID: ");
    Serial.println(commandId);

    Serial.print("[RemoteStateService] Porciones: ");
    Serial.println(portions);

    if (!_feedingService.feed(portions, FeedingSource::Remote))
    {
        Serial.println("[RemoteStateService] No se pudo iniciar la alimentacion.");
        return false;
    }

    command.status = RemoteCommandStatus::Executing;

    if (!_commandStorage.save(command))
    {
        Serial.println("[RemoteStateService] Error persistiendo estado executing.");
        return false;
    }

    _command = command;
    _remoteFeedInProgress = true;
    _activeCommandId = commandId;
    _lastConfirmationAttempt = 0;

    return true;
}
