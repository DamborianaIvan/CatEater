const express = require('express');
const router = express.Router();
const feederController = require('../controllers/feederController');
const verifyToken = require('../middlewares/authMiddleware');
const authenticateDevice = require('../middlewares/deviceAuthMiddleware');

/**
 * @swagger
 * tags:
 *   - name: Feeders
 *     description: Gestión de comederos, motor, configuración e historial.
 *
 * components:
 *   schemas:
 *     Feeder:
 *       type: object
 *       properties:
 *         _id: { type: string, example: 67f707fa9b66a6ee96d447a3 }
 *         feederId: { type: string, example: ESP8266-001 }
 *         userId: { type: string, nullable: true, example: 67f6d3af066a14a9d9699bd3 }
 *         feederName: { type: string, example: Comedero Cocina }
 *         feederLogo: { type: string, example: cat }
 *         feederAsign: { type: boolean, example: true }
 *         feederQuantity: { type: number, example: 0 }
 *         lastConection: { type: string, format: date-time }
 *         motorInfo:
 *           type: object
 *           properties:
 *             startHours:
 *               type: array
 *               items: { type: string, format: date-time }
 *             motorState: { type: boolean, example: false }
 *             portions: { type: integer, minimum: 1, maximum: 5, example: 1 }
 *             commandId: { type: string, nullable: true }
 *         configuration:
 *           $ref: '#/components/schemas/FeederConfiguration'
 *         feederHistory:
 *           type: array
 *           items:
 *             $ref: '#/components/schemas/FeedingEvent'
 *     FeederConfiguration:
 *       type: object
 *       required: [revision, stepsPerFeed, schedules]
 *       properties:
 *         revision: { type: integer, minimum: 1, example: 3 }
 *         stepsPerFeed: { type: integer, minimum: 1, maximum: 10240, example: 2048 }
 *         schedules:
 *           type: array
 *           minItems: 5
 *           maxItems: 5
 *           items:
 *             $ref: '#/components/schemas/Schedule'
 *     Schedule:
 *       type: object
 *       required: [hour, minute, portions, enabled]
 *       properties:
 *         hour: { type: integer, minimum: 0, maximum: 23, example: 8 }
 *         minute: { type: integer, minimum: 0, maximum: 59, example: 30 }
 *         portions: { type: integer, minimum: 1, maximum: 5, example: 1 }
 *         enabled: { type: boolean, example: true }
 *     FeedingEvent:
 *       type: object
 *       properties:
 *         eventId: { type: string, nullable: true, example: evt-000001 }
 *         fecha: { type: string, format: date-time }
 *         portions: { type: integer, minimum: 1, example: 1 }
 *         source: { type: string, enum: [physical, scheduled, remote, legacy], example: remote }
 *     Error:
 *       type: object
 *       properties:
 *         message: { type: string }
 *         error: { type: string }
 *     DeviceError:
 *       type: object
 *       properties:
 *         message: { type: string }
 *         error: { type: string }
 */

/**
 * @swagger
 * /feeders/global/{feederId}:
 *   get:
 *     summary: Obtener información del feeder para el dispositivo
 *     description: Endpoint usado por el firmware. Devuelve información pública del feeder y puede activar la orden del motor si coincide el minuto UTC con un horario legacy.
 *     tags: [Feeders]
 *     security: [{ deviceCredential: [] }]
 *     parameters:
 *       - in: path
 *         name: feederId
 *         required: true
 *         schema: { type: string }
 *     responses:
 *       200:
 *         description: Información obtenida.
 *         content:
 *           application/json:
 *             example: { message: Comedero obtenido correctamente, feederQuantity: 0, feederName: Comedero Cocina, feederLogo: cat, lastConection: '2026-08-31T18:00:00.000Z' }
 *       400: { description: feederId vacío o inválido. }
 *       401: { description: Credencial de dispositivo faltante o inválida. }
 *       404: { description: Feeder no encontrado. }
 *       500: { description: Error interno. }
 */
router.get('/feeders/global/:feederId', authenticateDevice, feederController.getGlobalFeederById);

/**
 * @swagger
 * /feeders:
 *   get:
 *     summary: Obtener todos los feeders
 *     description: Devuelve todos los documentos de Feeder. Actualmente requiere JWT pero no aplica control de rol ni de propietario.
 *     tags: [Feeders]
 *     security: [{ bearerAuth: [] }]
 *     responses:
 *       200:
 *         description: Lista de feeders.
 *         content:
 *           application/json:
 *             schema: { type: array, items: { $ref: '#/components/schemas/Feeder' } }
 *       401: { description: JWT faltante o inválido. }
 *       400: { description: Error al consultar los feeders. }
 */
router.get('/feeders', verifyToken, feederController.getAllFeeders);

/**
 * @swagger
 * /feeders/my:
 *   get:
 *     summary: Obtener mis feeders
 *     tags: [Feeders]
 *     security: [{ bearerAuth: [] }]
 *     responses:
 *       200:
 *         description: Feeders pertenecientes al usuario autenticado.
 *         content:
 *           application/json:
 *             schema: { type: array, items: { $ref: '#/components/schemas/Feeder' } }
 *       401: { description: Usuario no autenticado. }
 *       500: { description: Error interno. }
 */
router.get('/feeders/my', verifyToken, feederController.getMyFeeders);

/**
 * @swagger
 * /feeders/{feederId}:
 *   get:
 *     summary: Obtener un feeder propio
 *     tags: [Feeders]
 *     security: [{ bearerAuth: [] }]
 *     parameters:
 *       - in: path
 *         name: feederId
 *         required: true
 *         schema: { type: string }
 *     responses:
 *       200:
 *         description: Feeder encontrado.
 *         content:
 *           application/json:
 *             schema: { $ref: '#/components/schemas/Feeder' }
 *       400: { description: feederId faltante o vacío. }
 *       401: { description: Usuario no autenticado. }
 *       404: { description: El feeder no pertenece al usuario o no existe. }
 *       500: { description: Error interno. }
 */
router.get('/feeders/:feederId', verifyToken, feederController.getFeederById);

/**
 * @swagger
 * /feeder/start:
 *   post:
 *     summary: Solicitar alimentación remota
 *     description: Crea una orden remota activando motorInfo.motorState y devuelve un commandId. El firmware debe consultar este estado y confirmar la orden mediante /feeder/complete.
 *     tags: [Feeders]
 *     security: [{ bearerAuth: [] }]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             required: [feederId]
 *             properties:
 *               feederId: { type: string, example: ESP8266-001 }
 *               portions: { type: integer, minimum: 1, example: 1 }
 *     responses:
 *       200:
 *         description: Orden creada.
 *         content:
 *           application/json:
 *             example: { message: Feeder encendido con éxito, commandId: 550e8400-e29b-41d4-a716-446655440000 }
 *       400: { description: Porciones inválidas o motor ya encendido. }
 *       401: { description: JWT faltante o inválido. }
 *       403: { description: Feeder sin asignar o perteneciente a otro usuario. }
 *       404: { description: Feeder no encontrado. }
 *       500: { description: Error interno. }
 */
router.post('/feeder/start', verifyToken, feederController.startMotor);

/**
 * @swagger
 * /feeder/complete:
 *   post:
 *     summary: Confirmar finalización de una orden
 *     description: Endpoint exclusivo del dispositivo. Valida feederId y commandId, apaga la orden activa y no crea un evento anónimo en el historial.
 *     tags: [Feeders]
 *     security: [{ deviceCredential: [] }]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             required: [feederId, commandId]
 *             properties:
 *               feederId: { type: string, example: ESP8266-001 }
 *               commandId: { type: string, format: uuid }
 *     responses:
 *       200: { description: Orden completada o ya completada. }
 *       400: { description: feederId o commandId faltante. }
 *       401: { description: Credencial de dispositivo faltante o inválida. }
 *       404: { description: Feeder no encontrado. }
 *       409: { description: No existe una orden pendiente o el commandId no coincide. }
 *       500: { description: Error interno. }
 */
router.post('/feeder/complete', authenticateDevice, feederController.completeMotorCommand);

/**
 * @swagger
 * /feeder/edit:
 *   post:
 *     summary: Editar feeder
 *     tags: [Feeders]
 *     security: [{ bearerAuth: [] }]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             required: [feederId, feederName, feederLogo]
 *             properties:
 *               feederId: { type: string, example: ESP8266-001 }
 *               feederName: { type: string, example: Comedero Cocina }
 *               feederLogo: { type: string, example: cat }
 *     responses:
 *       200: { description: Feeder editado correctamente. }
 *       400: { description: Campos requeridos inválidos. }
 *       401: { description: Usuario no autenticado. }
 *       403: { description: Feeder sin asignar o perteneciente a otro usuario. }
 *       404: { description: Feeder no encontrado. }
 *       500: { description: Error interno. }
 */
router.post('/feeder/edit', verifyToken, feederController.editFeeder);

/**
 * @swagger
 * /feeders/motor-state/{feederId}:
 *   get:
 *     summary: Consultar orden del motor desde el dispositivo
 *     description: El feederId se toma de la credencial del dispositivo; el parámetro de ruta se mantiene por compatibilidad y es validado por el middleware.
 *     tags: [Feeders]
 *     security: [{ deviceCredential: [] }]
 *     parameters:
 *       - in: path
 *         name: feederId
 *         required: true
 *         schema: { type: string }
 *     responses:
 *       200:
 *         description: Estado remoto y metadatos de la orden.
 *         content:
 *           application/json:
 *             example: { motorState: true, portions: 1, commandId: '550e8400-e29b-41d4-a716-446655440000', configRevision: 3 }
 *       400: { description: feederId inválido. }
 *       401: { description: Credencial de dispositivo faltante o inválida. }
 *       404: { description: Feeder no encontrado. }
 *       500: { description: Error interno. }
 */
router.get('/feeders/motor-state/:feederId', authenticateDevice, feederController.getMotorStatusNodemcu);

/**
 * @swagger
 * /feeder/state/{feederId}:
 *   get:
 *     summary: Obtener estado del motor
 *     tags: [Feeders]
 *     security: [{ bearerAuth: [] }]
 *     parameters:
 *       - in: path
 *         name: feederId
 *         required: true
 *         schema: { type: string }
 *     responses:
 *       200:
 *         description: Estado actual.
 *         content:
 *           application/json:
 *             example: { motorState: false }
 *       400: { description: feederId inválido. }
 *       401: { description: Usuario no autenticado. }
 *       404: { description: Feeder no encontrado para el usuario. }
 *       500: { description: Error interno. }
 */
router.get('/feeder/state/:feederId', verifyToken, feederController.getMotorStatus);

/**
 * @swagger
 * /feeder/dates/{feederId}:
 *   get:
 *     summary: Obtener horarios legacy del feeder
 *     description: Devuelve motorInfo.startHours. Esta ruta pertenece al mecanismo de horarios anterior; la configuración vigente usa configuration.schedules.
 *     tags: [Feeders]
 *     security: [{ bearerAuth: [] }]
 *     parameters:
 *       - in: path
 *         name: feederId
 *         required: true
 *         schema: { type: string }
 *     responses:
 *       200:
 *         description: Fechas programadas.
 *         content:
 *           application/json:
 *             example: { dates: ['2026-09-01T08:00:00.000Z'] }
 *       400: { description: feederId inválido. }
 *       401: { description: Usuario no autenticado. }
 *       404: { description: Feeder no encontrado para el usuario. }
 *       500: { description: Error interno. }
 */
router.get('/feeder/dates/:feederId', verifyToken, feederController.getFechasByFeederId);

/**
 * @swagger
 * /feeder/add-hour:
 *   post:
 *     summary: Agregar horarios legacy
 *     description: Agrega fechas ISO-8601 futuras y no duplicadas a motorInfo.startHours. Para la configuración actual del firmware usar PUT /feeders/{feederId}/config.
 *     tags: [Feeders]
 *     security: [{ bearerAuth: [] }]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             required: [feederId, dates]
 *             properties:
 *               feederId: { type: string, example: ESP8266-001 }
 *               dates:
 *                 type: array
 *                 items: { type: string, format: date-time }
 *                 example: ['2026-09-01T08:00:00.000Z']
 *     responses:
 *       200: { description: Fechas agregadas. }
 *       400: { description: Datos inválidos o ninguna fecha válida para agregar. }
 *       401: { description: Usuario no autenticado. }
 *       404: { description: Feeder no encontrado para el usuario. }
 *       500: { description: Error interno. }
 */
router.post('/feeder/add-hour', verifyToken, feederController.addStartHours);

/**
 * @swagger
 * /feeder/{feederId}:
 *   delete:
 *     summary: Eliminar feeder
 *     description: Elimina el documento por feederId. Actualmente requiere JWT pero el controlador no valida que el feeder pertenezca al usuario.
 *     tags: [Feeders]
 *     security: [{ bearerAuth: [] }]
 *     parameters:
 *       - in: path
 *         name: feederId
 *         required: true
 *         schema: { type: string }
 *     responses:
 *       200: { description: Feeder eliminado. }
 *       400: { description: feederId inválido. }
 *       401: { description: Usuario no autenticado. }
 *       404: { description: Feeder no encontrado o ya eliminado. }
 *       500: { description: Error interno. }
 */
router.delete('/feeder/:feederId', verifyToken, feederController.deleteFeeder);

/**
 * @swagger
 * /feeder/{feederId}/historial:
 *   get:
 *     summary: Obtener historial de alimentación
 *     description: Devuelve el historial transformado para la interfaz, ordenado del más reciente al más antiguo.
 *     tags: [Feeders]
 *     security: [{ bearerAuth: [] }]
 *     parameters:
 *       - in: path
 *         name: feederId
 *         required: true
 *         schema: { type: string }
 *     responses:
 *       200:
 *         description: Historial formateado.
 *         content:
 *           application/json:
 *             example:
 *               - { id: '1', fecha: 'lun, 31 ago', hora: '13:20', cantidad: '20g', accion: 'Comedero activado' }
 *       401: { description: Usuario no autenticado. }
 *       404: { description: Feeder no encontrado para el usuario. }
 *       500: { description: Error interno. }
 */
router.get('/feeder/:feederId/historial', verifyToken, feederController.getFeederHistory);

/**
 * @swagger
 * /feeders/history:
 *   post:
 *     summary: Sincronizar un evento de alimentación
 *     description: Endpoint del dispositivo para subir un evento persistido localmente. Usa eventId para garantizar idempotencia.
 *     tags: [Feeders]
 *     security: [{ deviceCredential: [] }]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             required: [eventId, feederId, timestamp, portions, source]
 *             properties:
 *               eventId: { type: string, example: evt-000001 }
 *               feederId: { type: string, example: ESP8266-001 }
 *               timestamp: { type: integer, format: int64, description: Unix timestamp en segundos, example: 1788193200 }
 *               portions: { type: integer, minimum: 1, example: 1 }
 *               source: { type: string, enum: [physical, scheduled, remote, legacy], example: remote }
 *     responses:
 *       200:
 *         description: Evento sincronizado o ya existente.
 *       400: { description: Datos de alimentación incompletos. }
 *       401: { description: Credencial de dispositivo faltante o inválida. }
 *       404: { description: Feeder no encontrado. }
 *       500: { description: Error interno. }
 */
router.post('/feeders/history', authenticateDevice, feederController.syncFeedingHistory);

/**
 * @swagger
 * /feeders/heartbeat:
 *   post:
 *     summary: Registrar heartbeat del dispositivo
 *     description: Actualiza lastConection del feeder autenticado.
 *     tags: [Feeders]
 *     security: [{ deviceCredential: [] }]
 *     responses:
 *       200:
 *         description: Heartbeat recibido.
 *         content:
 *           application/json:
 *             example: { message: Heartbeat recibido }
 *       401: { description: Credencial de dispositivo faltante o inválida. }
 *       404: { description: Feeder no encontrado. }
 *       500: { description: Error interno. }
 */
router.post('/feeders/heartbeat', authenticateDevice, feederController.heartbeat);

/**
 * @swagger
 * /feeders/config/{feederId}:
 *   get:
 *     summary: Obtener configuración remota
 *     description: Endpoint del dispositivo. Devuelve la revisión, stepsPerFeed y exactamente cinco schedules. Si no existe configuración persistida se entrega la configuración por defecto.
 *     tags: [Feeders]
 *     security: [{ deviceCredential: [] }]
 *     parameters:
 *       - in: path
 *         name: feederId
 *         required: true
 *         schema: { type: string }
 *     responses:
 *       200:
 *         description: Configuración remota.
 *         content:
 *           application/json:
 *             schema: { $ref: '#/components/schemas/FeederConfiguration' }
 *       400: { description: feederId inválido. }
 *       401: { description: Credencial de dispositivo faltante o inválida. }
 *       404: { description: Feeder no encontrado. }
 *       500: { description: Error interno. }
 */
router.get('/feeders/config/:feederId', authenticateDevice, feederController.getRemoteConfiguration);

/**
 * @swagger
 * /feeders/{feederId}/config:
 *   put:
 *     summary: Actualizar configuración del feeder
 *     description: Reemplaza de forma completa la configuración remota. El body debe contener exactamente cinco schedules. Si no hay cambios, conserva la revisión actual; si hay cambios, incrementa revision en uno.
 *     tags: [Feeders]
 *     security: [{ bearerAuth: [] }]
 *     parameters:
 *       - in: path
 *         name: feederId
 *         required: true
 *         schema: { type: string }
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             required: [stepsPerFeed, schedules]
 *             properties:
 *               stepsPerFeed: { type: integer, minimum: 1, maximum: 10240, example: 2048 }
 *               schedules:
 *                 type: array
 *                 minItems: 5
 *                 maxItems: 5
 *                 items: { $ref: '#/components/schemas/Schedule' }
 *     responses:
 *       200:
 *         description: Configuración actualizada o sin cambios.
 *         content:
 *           application/json:
 *             example:
 *               message: Configuración actualizada correctamente.
 *               revision: 2
 *               configuration:
 *                 revision: 2
 *                 stepsPerFeed: 2048
 *                 schedules: []
 *       400: { description: Configuración inválida. }
 *       401: { description: Usuario no autenticado. }
 *       404: { description: Feeder no encontrado. }
 *       500: { description: Error interno. }
 */
router.put('/feeders/:feederId/config', verifyToken, feederController.updateFeederConfiguration);

module.exports = router;
