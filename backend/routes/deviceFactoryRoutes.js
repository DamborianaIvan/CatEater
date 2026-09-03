const express = require("express");
const router = express.Router();
const verifyToken = require("../middlewares/authMiddleware");
const requireAdmin = require("../middlewares/adminMiddleware");
const deviceFactoryController = require("../controllers/deviceFactoryController");

/**
 * @swagger
 * tags:
 *   - name: Device Factory
 *     description: Operaciones administrativas de dispositivos antes y después de su entrega.
 *
 * /admin/devices:
 *   post:
 *     summary: Crear dispositivo en fábrica
 *     description: Registra un feeder sin propietario y genera las credenciales necesarias para su posterior vinculación. Solo administradores. Las credenciales se muestran una única vez.
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
 *       400:
 *         description: feederId faltante o inválido.
 *       401:
 *         description: JWT faltante o inválido.
 *       403:
 *         description: El usuario autenticado no es administrador.
 *       409:
 *         description: Ya existe un dispositivo con ese feederId.
 *       500:
 *         description: Error interno del servidor.
 */
router.post(
  "/admin/devices",
  verifyToken,
  requireAdmin,
  deviceFactoryController.createFactoryDevice
);

/**
 * @swagger
 * /admin/devices/{feederId}/reset-credential:
 *   post:
 *     summary: Regenerar credencial de dispositivo
 *     description: Genera una nueva deviceCredential y reemplaza la credencial anterior del feeder. La nueva credencial se devuelve una única vez y debe ser reprovisionada en el dispositivo físico. Solo administradores.
 *     tags: [Device Factory]
 *     security:
 *       - bearerAuth: []
 *     parameters:
 *       - in: path
 *         name: feederId
 *         required: true
 *         schema:
 *           type: string
 *         description: Identificador único del dispositivo.
 *         example: ESP8266-001
 *     responses:
 *       200:
 *         description: Credencial regenerada correctamente.
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *               required: [feederId, deviceCredential]
 *               properties:
 *                 feederId:
 *                   type: string
 *                 deviceCredential:
 *                   type: string
 *                   description: Nueva credencial individual del dispositivo. Se muestra una única vez.
 *       400:
 *         description: feederId faltante o inválido.
 *       401:
 *         description: JWT faltante o inválido.
 *       403:
 *         description: El usuario autenticado no es administrador.
 *       404:
 *         description: Dispositivo no encontrado.
 *       500:
 *         description: Error interno del servidor.
 */
router.post(
  "/admin/devices/:feederId/reset-credential",
  verifyToken,
  requireAdmin,
  deviceFactoryController.resetFactoryDeviceCredential
);

module.exports = router;
