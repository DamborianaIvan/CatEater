const crypto = require("crypto");
const DeviceProvisioning = require("../models/DeviceProvisioning");

const BOOTSTRAP_SECRET_BYTES = 32;

const hashBootstrapSecret = (secret) => {
  return crypto.createHash("sha256").update(secret).digest("hex");
};

const generateBootstrapSecret = () => {
  return `BSTR_${crypto.randomBytes(BOOTSTRAP_SECRET_BYTES).toString("hex")}`;
};

const createDeviceProvisioning = async (req, res) => {
  const { feederId } = req.body;

  if (!feederId || typeof feederId !== "string" || feederId.trim() === "") {
    return res.status(400).json({
      error: "feederId es requerido y debe ser un string válido"
    });
  }

  const normalizedFeederId = feederId.trim();

  try {
    const existingProvisioning = await DeviceProvisioning.findOne({
      feederId: normalizedFeederId
    });

    if (existingProvisioning) {
      return res.status(409).json({
        error: "Ya existe un provisioning para ese feederId"
      });
    }

    const bootstrapSecret = generateBootstrapSecret();
    const bootstrapSecretHash = hashBootstrapSecret(bootstrapSecret);

    await DeviceProvisioning.create({
      feederId: normalizedFeederId,
      bootstrapSecretHash,
      status: "UNPROVISIONED"
    });

    return res.status(201).json({
      feederId: normalizedFeederId,
      bootstrapSecret
    });
  } catch (error) {
    if (error.code === 11000) {
      return res.status(409).json({
        error: "Ya existe un provisioning para ese feederId"
      });
    }

    console.error("Error al crear provisioning del dispositivo:", error);

    return res.status(500).json({
      error: "Error interno del servidor"
    });
  }
};

module.exports = {
  createDeviceProvisioning
};
