const devicePairingService = require("../services/devicePairingServices");

const pairDevice = async (req, res) => {
  const userId = req.user?._id;
  const { pairingToken, pairingCode } = req.body;

  try {
    const feeder = await devicePairingService.pairDevice({
      userId,
      pairingToken,
      pairingCode
    });

    return res.status(200).json({
      message: "Dispositivo vinculado correctamente",
      feeder
    });
  } catch (error) {
    switch (error.code) {
      case "USER_REQUIRED":
        return res.status(401).json({ error: "Usuario no autenticado" });
      case "PAIRING_CREDENTIAL_REQUIRED":
        return res.status(400).json({ error: "Credencial de pairing requerida" });
      case "MULTIPLE_PAIRING_CREDENTIALS":
        return res.status(400).json({ error: "Utilice una sola credencial de pairing" });
      case "INVALID_PAIRING_FORMAT":
        return res.status(400).json({ error: "Formato de pairing inválido" });
      case "INVALID_PAIRING_CREDENTIAL":
        return res.status(404).json({ error: "Credencial de pairing inválida" });
      case "FEEDER_ALREADY_ASSIGNED":
        return res.status(409).json({ error: "El dispositivo ya está asignado" });
      case "PAIRING_NOT_ACTIVE":
        return res.status(409).json({ error: "La credencial de pairing ya no está activa" });
      default:
        console.error("Error al vincular dispositivo:", error);
        return res.status(500).json({ error: "Error interno del servidor" });
    }
  }
};

const unpairDevice = async (req, res) => {
  const userId = req.user?._id;
  const { feederId } = req.params;

  try {
    const result = await devicePairingService.unpairDevice({ userId, feederId });

    return res.status(200).json({
      message: "Dispositivo desvinculado correctamente",
      feeder: result
    });
  } catch (error) {
    switch (error.code) {
      case "USER_REQUIRED":
        return res.status(401).json({ error: "Usuario no autenticado" });
      case "FEEDER_ID_REQUIRED":
        return res.status(400).json({ error: "feederId requerido" });
      case "FEEDER_NOT_FOUND":
        return res.status(404).json({ error: "Comedero no encontrado" });
      case "FEEDER_NOT_ASSIGNED":
        return res.status(409).json({ error: "El dispositivo no está asignado" });
      case "FEEDER_NOT_OWNER":
        return res.status(403).json({ error: "El dispositivo no te pertenece" });
      default:
        console.error("Error al desvincular dispositivo:", error);
        return res.status(500).json({ error: "Error interno del servidor" });
    }
  }
};

module.exports = {
  pairDevice,
  unpairDevice
};
