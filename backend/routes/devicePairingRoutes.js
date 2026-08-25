const express = require("express");
const router = express.Router();

const verifyToken = require("../middlewares/authMiddleware");
const devicePairingController = require("../controllers/devicePairingController");

router.post(
  "/devices/pair",
  verifyToken,
  devicePairingController.pairDevice
);

router.delete(
  "/devices/:feederId/pair",
  verifyToken,
  devicePairingController.unpairDevice
);
module.exports = router;