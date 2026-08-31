const express = require("express");
const router = express.Router();
const verifyToken = require("../middlewares/authMiddleware");
const pairingRateLimit = require("../middlewares/pairingRateLimitMiddleware");
const devicePairingController = require("../controllers/devicePairingController");

/**
 * @swagger
 * tags:
 *   name: Device Pairing
 *   description: Vinculación y desvinculación de dispositivos con usuarios.
 */

/**
 * @swagger
 * /devices/pair:
 *   post:
 *     summary: Vincular dispositivo
 *     description: Vincula un dispositivo disponible al usuario autenticado usando exactamente una credencial de pairing. Se puede utilizar pairingToken o pairingCode, pero no ambos. El endpoint tiene un límite de 10 intentos por usuario/IP cada 15 minutos.
 *     tags:
 *       - Device Pairing
 *     security:
 *       - bearerAuth: []
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             $ref: '#/components/schemas/PairDeviceRequest'
 *           examples:
 *             token:
 *               summary: Pairing mediante token
 *               value:
 *                 pairingToken: 0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef
 *             code:
 *               summary: Pairing mediante código
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
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Error'
 *       401:
 *         description: Usuario no autenticado.
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Error'
 *       404:
 *         description: Credencial de pairing inválida.
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Error'
 *       409:
 *         description: El dispositivo ya está asignado o la credencial ya no está activa.
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Error'
 *       429:
 *         description: Se superó el límite de intentos de pairing.
 *         headers:
 *           Retry-After:
 *             description: Segundos que deben transcurrir antes de volver a intentar.
 *             schema:
 *               type: integer
 *               example: 742
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Error'
 *       500:
 *         description: Error interno del servidor.
 */
router.post(
  "/devices/pair",
  verifyToken,
  pairingRateLimit,
  devicePairingController.pairDevice
);

/**
 * @swagger
 * /devices/{feederId}/pair:
 *   delete:
 *     summary: Desvincular dispositivo
 *     description: Libera un dispositivo propiedad del usuario autenticado y genera nuevas credenciales de pairing para una futura vinculación.
 *     tags:
 *       - Device Pairing
 *     security:
 *       - bearerAuth: []
 *     parameters:
 *       - in: path
 *         name: feederId
 *         required: true
 *         description: Identificador único del dispositivo.
 *         schema:
 *           type: string
 *         example: ESP8266-001
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
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Error'
 *       401:
 *         description: Usuario no autenticado.
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Error'
 *       403:
 *         description: El dispositivo pertenece a otro usuario.
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Error'
 *       404:
 *         description: Comedero no encontrado.
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Error'
 *       409:
 *         description: El dispositivo no está asignado.
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Error'
 *       500:
 *         description: Error interno del servidor.
 */
router.delete(
  "/devices/:feederId/pair",
  verifyToken,
  devicePairingController.unpairDevice
);

/**
 * @swagger
 * components:
 *   schemas:
 *     PairDeviceRequest:
 *       type: object
 *       description: Debe contener exactamente una de las dos credenciales de pairing.
 *       minProperties: 1
 *       maxProperties: 1
 *       properties:
 *         pairingToken:
 *           type: string
 *           pattern: "^[a-f0-9]{64}$"
 *           description: Token hexadecimal de 64 caracteres.
 *           example: 0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef
 *         pairingCode:
 *           type: string
 *           pattern: "^[ABCDEFGHJKLMNPQRSTUVWXYZ23456789]{8}$"
 *           description: Código de pairing de 8 caracteres. Se normaliza a mayúsculas.
 *           example: AB2CD3EF
 */

module.exports = router;
