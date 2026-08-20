const Feeder = require("../models/Feeder");
const {
  generateDeviceCredential,
  hashDeviceCredential
} = require("../utils/deviceCredential");

const enrollDevice = async (req, res) => {
  const apiKey = req.headers["x-api-key"];
  const { feederId } = req.body || {};

  if (apiKey !== process.env.NODEMCU_API_KEY) {
    return res.status(401).json({
      error: "No autorizado - API Key inválida"
    });
  }

  if (!feederId || typeof feederId !== "string" || feederId.trim() === "") {
    return res.status(400).json({
      error: "feederId es requerido"
    });
  }

  try {
    const feeder = await Feeder.findOne({ feederId }).select("+deviceCredentialHash");

    if (!feeder) {
      return res.status(404).json({
        error: "Feeder no encontrado"
      });
    }

    if (feeder.deviceCredentialHash) {
      return res.status(409).json({
        error: "El dispositivo ya está enrolado"
      });
    }

    const deviceCredential = generateDeviceCredential();
    feeder.deviceCredentialHash = hashDeviceCredential(deviceCredential);

    await feeder.save();

    return res.status(201).json({
      message: "Dispositivo enrolado correctamente",
      deviceCredential
    });
  } catch (error) {
    console.error("Error al enrolar dispositivo:", error);

    return res.status(500).json({
      error: "Error interno del servidor"
    });
  }
};

module.exports = {
  enrollDevice
};
