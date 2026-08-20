const express = require("express");
const router = express.Router();
const deviceController = require("../controllers/deviceController");

// Enrollment inicial del dispositivo.
// La x-api-key se utiliza únicamente como credencial de bootstrap.
router.post("/feeders/enroll", deviceController.enrollDevice);

module.exports = router;
