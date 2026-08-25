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

module.exports = {
  createFactoryDevice
};
