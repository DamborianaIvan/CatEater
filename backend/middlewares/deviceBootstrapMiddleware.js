const crypto = require("crypto");
const DeviceProvisioning = require("../models/DeviceProvisioning");

const hashBootstrapSecret = (secret) => {
  return crypto.createHash("sha256").update(secret).digest("hex");
};

const authenticateDeviceBootstrap = async (req, res, next) => {
  const bootstrapSecret = req.headers["x-device-bootstrap"];

  if (!bootstrapSecret || typeof bootstrapSecret !== "string") {
    return res.status(401).json({
      error: "Bootstrap credential requerida"
    });
  }

  try {
    const bootstrapSecretHash = hashBootstrapSecret(bootstrapSecret);

    const provisioning = await DeviceProvisioning.findOne({
      bootstrapSecretHash
    });

    if (!provisioning) {
      return res.status(401).json({
        error: "Bootstrap credential inválida"
      });
    }

    if (provisioning.status === "ENROLLED") {
      return res.status(403).json({
        error: "Bootstrap credential ya utilizada"
      });
    }

    req.deviceProvisioning = provisioning;

    next();
  } catch (error) {
    console.error(
      "Error al autenticar bootstrap del dispositivo:",
      error
    );

    return res.status(500).json({
      error: "Error interno del servidor"
    });
  }
};

module.exports = authenticateDeviceBootstrap;