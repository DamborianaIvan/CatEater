const Feeder = require("../models/Feeder");
const {
  generatePairingToken,
  generatePairingCode,
  hashPairingCredential
} = require("../utils/pairingCredential");

const PAIRING_CODE_PATTERN = /^[ABCDEFGHJKLMNPQRSTUVWXYZ23456789]{8}$/;
const PAIRING_TOKEN_PATTERN = /^[a-f0-9]{64}$/;

const createPairingError = (message, code) => {
  const error = new Error(message);
  error.code = code;
  return error;
};

const validatePairingCredential = ({ pairingToken, pairingCode }) => {
  if (!pairingToken && !pairingCode) {
    throw createPairingError("Credencial de pairing requerida.", "PAIRING_CREDENTIAL_REQUIRED");
  }

  if (pairingToken && pairingCode) {
    throw createPairingError("Utilice una sola credencial de pairing.", "MULTIPLE_PAIRING_CREDENTIALS");
  }

  if (pairingToken) {
    if (typeof pairingToken !== "string" || !PAIRING_TOKEN_PATTERN.test(pairingToken)) {
      throw createPairingError("Formato de pairing inválido.", "INVALID_PAIRING_FORMAT");
    }

    return {
      type: "token",
      hash: hashPairingCredential(pairingToken)
    };
  }

  const normalizedCode = typeof pairingCode === "string"
    ? pairingCode.trim().toUpperCase()
    : "";

  if (!PAIRING_CODE_PATTERN.test(normalizedCode)) {
    throw createPairingError("Formato de pairing inválido.", "INVALID_PAIRING_FORMAT");
  }

  return {
    type: "code",
    hash: hashPairingCredential(normalizedCode)
  };
};

const pairDevice = async ({ userId, pairingToken, pairingCode } = {}) => {
  if (!userId) {
    throw createPairingError("Usuario requerido.", "USER_REQUIRED");
  }

  const credential = validatePairingCredential({ pairingToken, pairingCode });

  const query = credential.type === "token"
    ? { "pairing.tokenHash": credential.hash }
    : { "pairing.codeHash": credential.hash };

  const feeder = await Feeder.findOne(query)
    .select("+pairing.tokenHash +pairing.codeHash");

  if (!feeder) {
    throw createPairingError("Credencial de pairing inválida.", "INVALID_PAIRING_CREDENTIAL");
  }

  if (feeder.isAssigned()) {
    throw createPairingError("El dispositivo ya está asignado.", "FEEDER_ALREADY_ASSIGNED");
  }

  if (!feeder.hasActivePairing()) {
    throw createPairingError("La credencial de pairing ya no está activa.", "PAIRING_NOT_ACTIVE");
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
    throw createPairingError("Usuario requerido.", "USER_REQUIRED");
  }

  if (!feederId || typeof feederId !== "string" || feederId.trim() === "") {
    throw createPairingError("feederId requerido.", "FEEDER_ID_REQUIRED");
  }

  const feeder = await Feeder.findOne({ feederId: feederId.trim() })
    .select("+pairing.tokenHash +pairing.codeHash");

  if (!feeder) {
    throw createPairingError("Comedero no encontrado.", "FEEDER_NOT_FOUND");
  }

  if (!feeder.isAssigned()) {
    throw createPairingError("El dispositivo no está asignado.", "FEEDER_NOT_ASSIGNED");
  }

  if (feeder.userId.toString() !== userId.toString()) {
    throw createPairingError("El dispositivo no te pertenece.", "FEEDER_NOT_OWNER");
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
