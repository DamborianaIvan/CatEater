const express = require("express");
const router = express.Router();
const verifyToken = require("../middlewares/authMiddleware");
const deviceProvisioningController = require("../controllers/deviceProvisioningController");

/**
 * Provisioning administrativo de dispositivos.
 *
 * En esta fase la ruta queda protegida por JWT. La autorización administrativa
 * específica se incorporará junto con el sistema de roles antes de exponer
 * este endpoint fuera del entorno de administración.
 */
router.post(
  "/admin/device-provisioning",
  verifyToken,
  deviceProvisioningController.createDeviceProvisioning
);

module.exports = router;
