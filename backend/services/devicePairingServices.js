const Feeder = require("../models/Feeder");
const {
  generatePairingToken,
  generatePairingCode,
  hashPairingCredential
} = require("../utils/pairingCredential");

const pairDevice = async ({ userId, pairingToken, pairingCode } = {}) => {
  if (!userId) {
    const error = new Error("Usuario requerido.");
    error.code = "USER_REQUIRED";
    throw error;
  }

  if (!pairingToken && !pairingCode) {
    const error = new Error("Credencial de pairing requerida.");
    error.code = "PAIRING_CREDENTIAL_REQUIRED";
    throw error;
  }

  if (pairingToken && pairingCode) {
    const error = new Error("Utilice una sola credencial de pairing.");
    error.code = "MULTIPLE_PAIRING_CREDENTIALS";
    throw error;
  }

  const credential = pairingToken || pairingCode;
  const credentialHash = hashPairingCredential(credential);

  const query = pairingToken
    ? { "pairing.tokenHash": credentialHash }
    : { "pairing.codeHash": credentialHash };

  const feeder = await Feeder.findOne(query).select("+pairing.tokenHash +pairing.codeHash");

  if (!feeder) {
    const error = new Error("Credencial de pairing inválida.");
    error.code = "INVALID_PAIRING_CREDENTIAL";
    throw error;
  }

  if (feeder.isAssigned()) {
    const error = new Error("El dispositivo ya está asignado.");
    error.code = "FEEDER_ALREADY_ASSIGNED";
    throw error;
  }

  if (!feeder.hasActivePairing()) {
    const error = new Error("La credencial de pairing ya no está activa.");
    error.code = "PAIRING_NOT_ACTIVE";
    throw error;
  }

  feeder.userId = userId;
  feeder.feederAsign = true;

  feeder.consumePairing();

  await feeder.save();

  return {
    feederId: feeder.feederId,
    feederName: feeder.feederName,
    userId: feeder.userId
  };
};

const unpairDevice = async ({ userId, feederId } = {}) => {
  if (!userId) {
    const error = new Error("Usuario requerido.");
    error.code = "USER_REQUIRED";
    throw error;
  }

  if (!feederId || typeof feederId !== "string" || feederId.trim() === "") {
    const error = new Error("feederId requerido.");
    error.code = "FEEDER_ID_REQUIRED";
    throw error;
  }

  const feeder = await Feeder.findOne({ feederId: feederId.trim() })
    .select("+pairing.tokenHash +pairing.codeHash");

  if (!feeder) {
    const error = new Error("Comedero no encontrado.");
    error.code = "FEEDER_NOT_FOUND";
    throw error;
  }

  if (!feeder.isAssigned()) {
    const error = new Error("El dispositivo no está asignado.");
    error.code = "FEEDER_NOT_ASSIGNED";
    throw error;
  }

  if (feeder.userId.toString() !== userId.toString()) {
    const error = new Error("El dispositivo no te pertenece.");
    error.code = "FEEDER_NOT_OWNER";
    throw error;
  }

  const pairingToken = generatePairingToken();
  const pairingCode = generatePairingCode();

  feeder.userId = null;
  feeder.feederAsign = false;

  feeder.pairing.tokenHash = hashPairingCredential(pairingToken);
  feeder.pairing.codeHash = hashPairingCredential(pairingCode);
  feeder.pairing.usedAt = null;

  await feeder.save();

  return {
    feederId: feeder.feederId,
    feederName: feeder.feederName,
    pairingToken,
    pairingCode
  };
};
module.exports = {
  pairDevice,
  unpairDevice
};