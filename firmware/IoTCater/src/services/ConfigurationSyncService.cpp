#include "services/ConfigurationSyncService.h"

ConfigurationSyncService::ConfigurationSyncService(ApiClient& apiClient,
                                                   ConfigurationStorage& configurationStorage,
                                                   ConfigurationRevisionStorage& revisionStorage,
                                                   Configuration& configuration)
    : _apiClient(apiClient),
      _configurationStorage(configurationStorage),
      _revisionStorage(revisionStorage),
      _configuration(configuration)
{
}

void ConfigurationSyncService::begin()
{
    if (!_revisionStorage.load(_localRevision))
    {
        Serial.println(
            "[ConfigurationSyncService] Error cargando revision. "
            "Usando revision 0.");

        _localRevision = 0;
    }

    Serial.print("[ConfigurationSyncService] Revision local: ");

    Serial.println(_localRevision);
}

void ConfigurationSyncService::update(uint32_t remoteRevision)
{
    if (remoteRevision <= _localRevision)
    {
        return;
    }

    Serial.print("[ConfigurationSyncService] Nueva revision remota: ");

    Serial.println(remoteRevision);

    applyRemoteConfiguration(remoteRevision);
}

bool ConfigurationSyncService::applyRemoteConfiguration(uint32_t remoteRevision)
{
    Configuration newConfiguration;
    uint32_t receivedRevision = 0;

    if (!_apiClient.getRemoteConfiguration(newConfiguration, receivedRevision))
    {
        Serial.println("[ConfigurationSyncService] Error obteniendo configuracion remota.");

        return false;
    }

    if (receivedRevision != remoteRevision)
    {
        Serial.println(
            "[ConfigurationSyncService] La revision cambio durante "
            "la consulta.");

        return false;
    }

    if (receivedRevision <= _localRevision)
    {
        return true;
    }

    if (!_configurationStorage.saveConfiguration(newConfiguration))
    {
        Serial.println(
            "[ConfigurationSyncService] Configuracion invalida "
            "o no pudo persistirse.");

        return false;
    }

    if (!_revisionStorage.save(receivedRevision))
    {
        Serial.println("[ConfigurationSyncService] Error guardando revision.");

        return false;
    }

    _configuration = newConfiguration;

    _localRevision = receivedRevision;

    Serial.print("[ConfigurationSyncService] Configuracion aplicada. Revision: ");

    Serial.println(_localRevision);

    return true;
}