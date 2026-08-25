const express = require('express');
const router = express.Router();
const feederController = require('../controllers/feederController');
const verifyToken = require('../middlewares/authMiddleware');
const authenticateDevice = require('../middlewares/deviceAuthMiddleware');

/**
 * @swagger
 * tags:
 *   name: Feeders
 *   description: Endpoints que trabajan sobre las globales de feeders
 */

/**
 * @swagger
 * tags:
 *   name: Feeder
 *   description: Endpoints que trabajan sobre las acciones del feeder.
 */

// Obtener comederos desde el dispositivo autenticado
/**
 * @swagger
 * /feeders/global/{feederId}:
 *   get:
 *     summary: Obtener información de un comedero desde el dispositivo
 *     tags:
 *       - Feeders
 */
router.get('/feeders/global/:feederId', authenticateDevice, feederController.getGlobalFeederById);

// Obtener todos los comederos
router.get('/feeders', verifyToken, feederController.getAllFeeders);

// Obtener comederos asignados al usuario logueado
router.get('/feeders/my', verifyToken, feederController.getMyFeeders);

// Obtener un comedero específico para el usuario logueado
router.get('/feeders/:feederId', verifyToken, feederController.getFeederById);

// Asignar comedero al usuario logueado
router.post('/feeder/assign', verifyToken, feederController.assignFeeder);

// Desasignar comedero
router.post('/feeder/unassign', verifyToken, feederController.unassignFeeder);

// Encender comedero
router.post('/feeder/start', verifyToken, feederController.startMotor);

// Completar ejecución de motor desde el dispositivo
router.post('/feeder/complete', authenticateDevice, feederController.completeMotorCommand);

// Editar comedero
router.post('/feeder/edit', verifyToken, feederController.editFeeder);

// Obtener estado del motor desde el dispositivo
router.get('/feeders/motor-state/:feederId', authenticateDevice, feederController.getMotorStatusNodemcu);

// Obtener estado del motor para el usuario
router.get('/feeder/state/:feederId', verifyToken, feederController.getMotorStatus);

// Obtener fechas del comedero
router.get('/feeder/dates/:feederId', verifyToken, feederController.getFechasByFeederId);

// Agregar horas programadas
router.post('/feeder/add-hour', verifyToken, feederController.addStartHours);

// Eliminar comedero
router.delete('/feeder/:feederId', verifyToken, feederController.deleteFeeder);

// Obtener historial
router.get('/feeder/:feederId/historial', verifyToken, feederController.getFeederHistory);

// Sincronizar historial desde el dispositivo
router.post('/feeders/history', authenticateDevice, feederController.syncFeedingHistory);

// Heartbeat del dispositivo
router.post('/feeders/heartbeat', authenticateDevice, feederController.heartbeat);

// Configuración remota del dispositivo
router.get('/feeders/config/:feederId', authenticateDevice, feederController.getRemoteConfiguration);

// Actualizar configuración desde el usuario
router.put('/feeders/:feederId/config', verifyToken, feederController.updateFeederConfiguration);

module.exports = router;
