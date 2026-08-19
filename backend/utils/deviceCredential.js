const crypto = require("crypto");

const DEVICE_CREDENTIAL_BYTES = 32;

const generateDeviceCredential = () => {
  return crypto.randomBytes(DEVICE_CREDENTIAL_BYTES).toString("hex");
};

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

module.exports = {
  generateDeviceCredential,
  hashDeviceCredential,
  verifyDeviceCredential
};
