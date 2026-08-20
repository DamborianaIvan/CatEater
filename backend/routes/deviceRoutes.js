const express = require("express");
const router = express.Router();
const deviceController = require("../controllers/deviceController");
const authenticateDevice = require("../middlewares/deviceAuthMiddleware");

// Enrollment inicial del dispositivo.
// La x-api-key se utiliza únicamente como credencial de bootstrap.
router.post("/feeders/enroll", deviceController.enrollDevice);

// Endpoint temporal de prueba para validar autenticación individual del dispositivo.
router.get("/devices/:feederId/auth-test", authenticateDevice, (req, res) => {
  return res.status(200).json({
    authenticated: true,
    feederId: req.device.feederId
  });
});

module.exports = router;
