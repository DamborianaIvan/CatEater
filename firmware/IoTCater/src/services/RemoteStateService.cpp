#include "services/RemoteStateService.h"

RemoteStateService::RemoteStateService(ApiClient& apiClient, FeedingService& feedingService,
                                       WiFiService& wifiService,
                                       RemoteCommandStorage& commandStorage,
                                       ConfigurationSyncService& configurationSyncService,
                                       DiagnosticService& diagnostics)
    : _apiClient(apiClient),
      _feedingService(feedingService),
      _wifiService(wifiService),
      _commandStorage(commandStorage),
      _configurationSyncService(configurationSyncService),
      _diagnostics(diagnostics)
{
}

void RemoteStateService::update()
{
    if (!_wifiService.isConnected() || !_apiClient.isBackendAvailable())
        return;

    const unsigned long now = millis();
    if (now - _lastRequest < POLL_INTERVAL)
        return;

    _lastRequest = now;
    pollMotorState();
}

bool RemoteStateService::loadCommand()
{
    _hasCommand = _commandStorage.load(_command);
    return true;
}

void RemoteStateService::pollMotorState()
{
    bool motorState = false;
    int portions = 1;
    String commandId;
    uint32_t configRevision = 0;

    Serial.println("[RemoteStateService] Consultando estado remoto...");

    if (!_apiClient.getMotorState(motorState, portions, commandId, configRevision))
    {
        Serial.println("[RemoteStateService] ERROR: fallo GET /feeders/motor-state.");
        _diagnostics.record("REMOTE_STATE_ERROR");
        return;
    }

    Serial.printf("[RemoteStateService] Backend: motor=%s portions=%d commandId=%s configRevision=%lu\n",
                  motorState ? "ON" : "OFF", portions, commandId.c_str(),
                  static_cast<unsigned long>(configRevision));

    _configurationSyncService.update(configRevision);

    if (!commandId.isEmpty())
    {
        if (_hasCommand && commandId == _command.commandId)
        {
            if (_command.status == RemoteCommandStatus::Executing)
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
        if (millis() - _lastConfirmationAttempt < CONFIRMATION_RETRY_INTERVAL)
            return;

        _lastConfirmationAttempt = millis();

        if (!_apiClient.isBackendAvailable())
            return;

        if (_apiClient.completeMotorCommand(_activeCommandId))
        {
            _command.status = RemoteCommandStatus::Completed;

            if (!_commandStorage.save(_command))
            {
                _diagnostics.record("REMOTE_COMMAND_STATE_ERROR", _activeCommandId.c_str());
                return;
            }

            _remoteFeedInProgress = false;
            _activeCommandId = "";
        }
        else
        {
            _diagnostics.record("REMOTE_COMMAND_CONFIRM_ERROR", _activeCommandId.c_str());
        }
    }
}

bool RemoteStateService::processCommand(const String& commandId, int portions)
{
    if (commandId.isEmpty())
        return false;

    RemoteCommand command;
    command.commandId = commandId;
    command.portions = portions;
    command.status = RemoteCommandStatus::Pending;

    if (!_commandStorage.save(command))
    {
        _diagnostics.record("REMOTE_COMMAND_STORAGE_ERROR", commandId.c_str());
        return false;
    }

    _command = command;
    _hasCommand = true;

    if (!_feedingService.feed(portions, FeedingSource::Remote))
    {
        _diagnostics.record("REMOTE_FEED_ERROR", commandId.c_str());
        return false;
    }

    command.status = RemoteCommandStatus::Executing;

    if (!_commandStorage.save(command))
    {
        _diagnostics.record("REMOTE_COMMAND_STATE_ERROR", commandId.c_str());
        return false;
    }

    _command = command;
    _remoteFeedInProgress = true;
    _activeCommandId = commandId;
    _lastConfirmationAttempt = 0;

    return true;
}
