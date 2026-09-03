#include "services/ConfigurationSyncService.h"

ConfigurationSyncService::ConfigurationSyncService(ApiClient& apiClient,
                                                   ConfigurationStorage& configurationStorage,
                                                   ConfigurationRevisionStorage& revisionStorage,
                                                   Configuration& configuration, Motor& motor)
    : _apiClient(apiClient),
      _configurationStorage(configurationStorage),
      _revisionStorage(revisionStorage),
      _configuration(configuration),
      _motor(motor)
{
}

void ConfigurationSyncService::begin()
{
    if (!_revisionStorage.load(_localRevision))
    {
        Serial.println("[ConfigurationSyncService] Error cargando revision. Usando revision 0.");
        _localRevision = 0;
    }

    Serial.printf("[ConfigurationSyncService] Revision local: %lu\n",
                  static_cast<unsigned long>(_localRevision));
}

void ConfigurationSyncService::update(uint32_t remoteRevision)
{
    Serial.printf("[ConfigurationSyncService] Revision recibida: remota=%lu, local=%lu\n",
                  static_cast<unsigned long>(remoteRevision),
                  static_cast<unsigned long>(_localRevision));

    if (remoteRevision == 0)
    {
        Serial.println("[ConfigurationSyncService] ADVERTENCIA: backend devolvio revision 0.");
        return;
    }

    if (remoteRevision < _localRevision)
    {
        Serial.println("[ConfigurationSyncService] Revision remota atrasada. No se aplica.");
        return;
    }

    if (remoteRevision == _localRevision)
    {
        Serial.println("[ConfigurationSyncService] Configuracion sincronizada.");
        return;
    }

    Serial.printf("[ConfigurationSyncService] Nueva revision remota: %lu\n",
                  static_cast<unsigned long>(remoteRevision));

    if (!applyRemoteConfiguration(remoteRevision))
        Serial.println("[ConfigurationSyncService] ERROR: no se pudo aplicar la configuracion remota.");
}

bool ConfigurationSyncService::applyRemoteConfiguration(uint32_t remoteRevision)
{
    Configuration newConfiguration;
    uint32_t receivedRevision = 0;

    Serial.println("[ConfigurationSyncService] Solicitando configuracion completa al backend...");

    if (!_apiClient.getRemoteConfiguration(newConfiguration, receivedRevision))
    {
        Serial.println("[ConfigurationSyncService] Error obteniendo configuracion remota.");
        return false;
    }

    Serial.printf("[ConfigurationSyncService] Respuesta: revision=%lu, stepsPerFeed=%d\n",
                  static_cast<unsigned long>(receivedRevision), newConfiguration.stepsPerFeed);

    if (receivedRevision != remoteRevision)
    {
        Serial.printf("[ConfigurationSyncService] ERROR: revision esperada=%lu, recibida=%lu\n",
                      static_cast<unsigned long>(remoteRevision),
                      static_cast<unsigned long>(receivedRevision));
        return false;
    }

    if (receivedRevision <= _localRevision)
    {
        Serial.println("[ConfigurationSyncService] La configuracion ya estaba aplicada.");
        return true;
    }

    if (!_configurationStorage.saveConfiguration(newConfiguration))
    {
        Serial.println("[ConfigurationSyncService] ERROR: configuracion invalida o no pudo persistirse.");
        return false;
    }

    if (!_revisionStorage.save(receivedRevision))
    {
        Serial.println("[ConfigurationSyncService] ERROR: configuracion guardada pero fallo persistencia de revision.");
        return false;
    }

    _configuration = newConfiguration;

    if (!_motor.setStepsPerFeed(newConfiguration.stepsPerFeed))
    {
        Serial.println("[ConfigurationSyncService] ERROR: fallo aplicando stepsPerFeed al motor.");
        return false;
    }

    _localRevision = receivedRevision;

    Serial.printf("[ConfigurationSyncService] Configuracion aplicada correctamente. Revision=%lu, stepsPerFeed=%d\n",
                  static_cast<unsigned long>(_localRevision), newConfiguration.stepsPerFeed);

    return true;
}
