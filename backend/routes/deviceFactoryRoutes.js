const express = require("express");
const router = express.Router();
const verifyToken = require("../middlewares/authMiddleware");
const deviceFactoryController = require("../controllers/deviceFactoryController");

/**
 * Alta administrativa de dispositivos antes de su entrega al usuario.
 * La credential devuelta debe almacenarse únicamente en el dispositivo
 * durante el proceso de fabricación.
 */
router.post(
  "/admin/devices",
  verifyToken,
  deviceFactoryController.createFactoryDevice
);

module.exports = router;
