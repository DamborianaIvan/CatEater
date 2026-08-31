const express = require("express");
const router = express.Router();
const verifyToken = require("../middlewares/authMiddleware");
const pairingRateLimit = require("../middlewares/pairingRateLimitMiddleware");
const devicePairingController = require("../controllers/devicePairingController");

/**
 * @swagger
 * tags:
 *   - name: Device Pairing
 *     description: Vinculación y desvinculación de dispositivos con usuarios.
 *
 * /devices/pair:
 *   post:
 *     summary: Vincular dispositivo
 *     description: Vincula un dispositivo disponible al usuario autenticado usando exactamente una credencial de pairing: pairingToken o pairingCode. El endpoint tiene un límite de 10 intentos por usuario/IP cada 15 minutos.
 *     tags: [Device Pairing]
 *     security:
 *       - bearerAuth: []
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             properties:
 *               pairingToken:
 *                 type: string
 *                 pattern: '^[a-f0-9]{64}$'
 *                 description: Token hexadecimal de 64 caracteres.
 *               pairingCode:
 *                 type: string
 *                 pattern: '^[ABCDEFGHJKLMNPQRSTUVWXYZ23456789]{8}$'
 *                 description: Código de 8 caracteres; se normaliza a mayúsculas.
 *             minProperties: 1
 *             maxProperties: 1
 *           examples:
 *             token:
 *               value:
 *                 pairingToken: 0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef
 *             code:
 *               value:
 *                 pairingCode: AB2CD3EF
 *     responses:
 *       200:
 *         description: Dispositivo vinculado correctamente.
 *         content:
 *           application/json:
 *             example:
 *               message: Dispositivo vinculado correctamente
 *               feeder:
 *                 feederId: ESP8266-001
 *                 feederName: Comedero Cocina
 *                 userId: 67f6d3af066a14a9d9699bd3
 *       400:
 *         description: Credencial faltante, se enviaron ambas credenciales o el formato es inválido.
 *       401:
 *         description: Usuario no autenticado.
 *       404:
 *         description: Credencial de pairing inválida.
 *       409:
 *         description: El dispositivo ya está asignado o la credencial ya no está activa.
 *       429:
 *         description: Se superó el límite de intentos de pairing. La respuesta incluye Retry-After en segundos.
 *       500:
 *         description: Error interno del servidor.
 *
 * /devices/{feederId}/pair:
 *   delete:
 *     summary: Desvincular dispositivo
 *     description: Libera un dispositivo propiedad del usuario autenticado y genera nuevas credenciales de pairing para una futura vinculación.
 *     tags: [Device Pairing]
 *     security:
 *       - bearerAuth: []
 *     parameters:
 *       - in: path
 *         name: feederId
 *         required: true
 *         schema:
 *           type: string
 *         description: Identificador del dispositivo.
 *     responses:
 *       200:
 *         description: Dispositivo desvinculado correctamente.
 *         content:
 *           application/json:
 *             example:
 *               message: Dispositivo desvinculado correctamente
 *               feeder:
 *                 feederId: ESP8266-001
 *                 feederName: Comedero Cocina
 *                 pairingToken: 0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef
 *                 pairingCode: AB2CD3EF
 *       400:
 *         description: feederId faltante o inválido.
 *       401:
 *         description: Usuario no autenticado.
 *       403:
 *         description: El dispositivo pertenece a otro usuario.
 *       404:
 *         description: Comedero no encontrado.
 *       409:
 *         description: El dispositivo no está asignado.
 *       500:
 *         description: Error interno del servidor.
 */
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
