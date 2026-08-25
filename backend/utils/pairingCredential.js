const crypto = require("crypto");

const PAIRING_CODE_LENGTH = 8;
const PAIRING_CODE_ALPHABET = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";

const generatePairingToken = () => {
  return crypto.randomBytes(32).toString("hex");
};

const generatePairingCode = () => {
  const bytes = crypto.randomBytes(PAIRING_CODE_LENGTH);
  let code = "";

  for (const byte of bytes) {
    code += PAIRING_CODE_ALPHABET[byte % PAIRING_CODE_ALPHABET.length];
  }

  return code;
};

const hashPairingCredential = (credential) => {
  return crypto.createHash("sha256").update(credential, "utf8").digest("hex");
};

module.exports = {
  generatePairingToken,
  generatePairingCode,
  hashPairingCredential
};
