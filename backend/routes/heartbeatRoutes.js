const express = require("express");
const router = express.Router();
const heartbeatController = require("../controllers/heartbeatController");
const authenticateDevice = require("../middlewares/deviceAuthMiddleware");

/**
 * @swagger
 * /feeders/heartbeat:
 *   post:
 *     summary: Registrar heartbeat del dispositivo
 *     description: Actualiza la última conexión del feeder autenticado mediante device credential.
 *     tags: [Feeders]
 *     security: [{ deviceCredential: [] }]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             required: [feederId]
 *             properties:
 *               feederId:
 *                 type: string
 *                 example: ESP8266-001
 *     responses:
 *       200:
 *         description: Heartbeat recibido correctamente.
 *       400:
 *         description: Dispositivo no identificado.
 *       401:
 *         description: Credencial de dispositivo faltante o inválida.
 *       404:
 *         description: Feeder no encontrado.
 *       500:
 *         description: Error interno.
 */
router.post("/feeders/heartbeat", authenticateDevice, heartbeatController.sendHeartbeat);

module.exports = router;
