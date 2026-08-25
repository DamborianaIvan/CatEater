const express = require("express");
const router = express.Router();

const verifyToken = require("../middlewares/authMiddleware");
const pairingRateLimit = require("../middlewares/pairingRateLimitMiddleware");
const devicePairingController = require("../controllers/devicePairingController");

router.post(
  "/devices/pair",
  verifyToken,
  pairingRateLimit,
  devicePairingController.pairDevice
);

router.delete(
  "/devices/:feederId/pair",
  verifyToken,
  devicePairingController.unpairDevice
);

module.exports = router;
