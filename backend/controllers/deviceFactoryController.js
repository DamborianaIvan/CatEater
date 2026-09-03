const Feeder = require("../models/Feeder");
const {
  generateDeviceCredential,
  hashDeviceCredential
} = require("../utils/deviceCredential");

const {
  generatePairingToken,
  generatePairingCode,
  hashPairingCredential
} = require("../utils/pairingCredential");

const createFactoryDevice = async (req, res) => {
  const { feederId, feederName } = req.body;

  if (!feederId || typeof feederId !== "string" || feederId.trim() === "") {
    return res.status(400).json({
      error: "feederId es requerido y debe ser un string válido"
    });
  }

  const normalizedFeederId = feederId.trim();
  const normalizedFeederName =
    typeof feederName === "string" && feederName.trim() !== ""
      ? feederName.trim()
      : "CatFeeder";

  try {
    const existingFeeder = await Feeder.findOne({ feederId: normalizedFeederId });

    if (existingFeeder) {
      return res.status(409).json({
        error: "Ya existe un dispositivo con ese feederId"
      });
    }

    const deviceCredential = generateDeviceCredential();
    const pairingToken = generatePairingToken();
    const pairingCode = generatePairingCode();

    const feeder = await Feeder.create({
      feederId: normalizedFeederId,
      feederName: normalizedFeederName,
      userId: null,
      feederAsign: false,
      deviceCredentialHash: hashDeviceCredential(deviceCredential),
      pairing: {
        tokenHash: hashPairingCredential(pairingToken),
        codeHash: hashPairingCredential(pairingCode),
        usedAt: null
      }
    });

    return res.status(201).json({
      feederId: feeder.feederId,
      feederName: feeder.feederName,
      deviceCredential,
      pairingToken,
      pairingCode,
      qrPayload: {
        type: "catfeeder-pairing",
        token: pairingToken
      }
    });
  } catch (error) {
    if (error.code === 11000) {
      return res.status(409).json({
        error: "Ya existe un dispositivo con ese feederId"
      });
    }

    console.error("Error al crear dispositivo de fábrica:", error);

    return res.status(500).json({
      error: "Error interno del servidor"
    });
  }
};

const resetFactoryDeviceCredential = async (req, res) => {
  const { feederId } = req.params;

  if (!feederId || typeof feederId !== "string" || feederId.trim() === "") {
    return res.status(400).json({
      error: "feederId es requerido y debe ser un string válido"
    });
  }

  try {
    const feeder = await Feeder.findOne({ feederId: feederId.trim() }).select(
      "+deviceCredentialHash"
    );

    if (!feeder) {
      return res.status(404).json({
        error: "Dispositivo no encontrado"
      });
    }

    const deviceCredential = generateDeviceCredential();
    feeder.deviceCredentialHash = hashDeviceCredential(deviceCredential);
    await feeder.save();

    return res.status(200).json({
      feederId: feeder.feederId,
      deviceCredential
    });
  } catch (error) {
    console.error("Error al regenerar device credential:", error);

    return res.status(500).json({
      error: "Error interno del servidor"
    });
  }
};

module.exports = {
  createFactoryDevice,
  resetFactoryDeviceCredential
};
