const express = require("express");
const router = express.Router();
const verifyToken = require("../middlewares/authMiddleware");
const deviceFactoryController = require("../controllers/deviceFactoryController");

/**
 * @swagger
 * tags:
 *   - name: Device Factory
 *     description: Alta administrativa de dispositivos antes de su entrega al usuario.
 *
 * /admin/devices:
 *   post:
 *     summary: Crear dispositivo en fábrica
 *     description: Registra un feeder sin propietario y genera las credenciales necesarias para su posterior vinculación. La credencial de dispositivo y las credenciales de pairing se muestran una única vez en la respuesta.
 *     tags: [Device Factory]
 *     security:
 *       - bearerAuth: []
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
 *                 minLength: 1
 *                 description: Identificador único del dispositivo.
 *                 example: ESP8266-001
 *               feederName:
 *                 type: string
 *                 description: Nombre inicial. Si se omite o está vacío se utiliza CatFeeder.
 *                 example: Comedero Cocina
 *     responses:
 *       201:
 *         description: Dispositivo creado correctamente.
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *               required: [feederId, feederName, deviceCredential, pairingToken, pairingCode, qrPayload]
 *               properties:
 *                 feederId:
 *                   type: string
 *                 feederName:
 *                   type: string
 *                 deviceCredential:
 *                   type: string
 *                   description: Credencial individual que debe conservar el firmware.
 *                 pairingToken:
 *                   type: string
 *                   pattern: '^[a-f0-9]{64}$'
 *                 pairingCode:
 *                   type: string
 *                   pattern: '^[ABCDEFGHJKLMNPQRSTUVWXYZ23456789]{8}$'
 *                 qrPayload:
 *                   type: object
 *                   properties:
 *                     type:
 *                       type: string
 *                       example: catfeeder-pairing
 *                     token:
 *                       type: string
 *       400:
 *         description: feederId faltante o inválido.
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/ErrorDevice'
 *       401:
 *         description: JWT faltante o inválido.
 *       409:
 *         description: Ya existe un dispositivo con ese feederId.
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/ErrorDevice'
 *       500:
 *         description: Error interno del servidor.
 */
router.post(
  "/admin/devices",
  verifyToken,
  deviceFactoryController.createFactoryDevice
);

module.exports = router;
