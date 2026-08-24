const crypto = require("crypto");
const Feeder = require("../models/Feeder");

const DEVICE_CREDENTIAL_HEADER = "x-device-credential";

const hashDeviceCredential = (credential) => {
  return crypto.createHash("sha256").update(credential, "utf8").digest("hex");
};

const verifyDeviceCredential = (credential, expectedHash) => {
  if (!credential || !expectedHash) {
    return false;
  }

  const receivedHash = hashDeviceCredential(credential);
  const receivedBuffer = Buffer.from(receivedHash, "hex");
  const expectedBuffer = Buffer.from(expectedHash, "hex");

  if (receivedBuffer.length !== expectedBuffer.length) {
    return false;
  }

  return crypto.timingSafeEqual(receivedBuffer, expectedBuffer);
};

const getFeederId = (req) => {
  return req.body?.feederId || req.params?.feederId;
};

const authenticateDevice = async (req, res, next) => {
  const credential = req.header(DEVICE_CREDENTIAL_HEADER);
  const feederId = getFeederId(req);

  if (!credential) {
    return res.status(401).json({
      message: "Credencial de dispositivo faltante."
    });
  }

  if (!feederId || typeof feederId !== "string" || feederId.trim() === "") {
    return res.status(400).json({
      message: "El campo 'feederId' es requerido y debe ser un string válido."
    });
  }

  try {
    const feeder = await Feeder.findOne({ feederId }).select("+deviceCredentialHash");
    
  ///TESTTTTTT
console.log("[DEVICE AUTH] feederId:", feederId);
console.log("[DEVICE AUTH] credential recibida:", !!credential);
console.log("[DEVICE AUTH] MongoDB:", Feeder.db.name);

const feeder1 = await Feeder.findOne({ feederId });

console.log(
  "[DEVICE AUTH] feeder1 encontrado:",
  feeder1 ? feeder1.feederId : "undefined"
);
console.log(
  "[DEVICE AUTH] credential hash:",
  hashDeviceCredential(credential)
);

console.log(
  "[DEVICE AUTH] feeder hash:",
  feeder?.deviceCredentialHash
);
///TESTTTTTT


    if (!feeder) {
      return res.status(404).json({
        message: "Comedero no encontrado."
      });
    }

    if (!verifyDeviceCredential(credential, feeder.deviceCredentialHash)) {
      return res.status(401).json({
        message: "Credencial de dispositivo inválida."
      });
    }

    req.device = {
      feederId: feeder.feederId
    };

    next();
  } catch (error) {
    console.error("Error en autenticación del dispositivo:", error);

    return res.status(500).json({
      message: "Error interno al validar la autenticación del dispositivo."
    });
  }
};

module.exports = authenticateDevice;
