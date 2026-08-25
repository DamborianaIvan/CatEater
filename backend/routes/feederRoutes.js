const express = require('express');
const router = express.Router();
const feederController = require('../controllers/feederController');
const verifyToken = require('../middlewares/authMiddleware'); // Middleware para proteger rutas (JWT)
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

// Obtener comederos desde NodeMCU por feederId
/**
 * @swagger
 * /feeders/global/{feederId}:
 *   get:
 *     summary: Obtener información de un comedero desde NodeMCU
 *     description: Este endpoint permite obtener los datos básicos del comedero (`feederName`, `feederLogo`, `feederQuantity`, `lastConection`) utilizando el `feederId`. También verifica si el motor debe activarse basándose en las horas programadas.
 *     tags:
 *       - Feeders
 *     parameters:
 *       - in: header
 *         name: x-api-key
 *         required: true
 *         schema:
 *           type: string
 *         description: API Key para autorización desde el NodeMCU
 *       - in: path
 *         name: feederId
 *         required: true
 *         schema:
 *           type: string
 *         description: ID del comedero a consultar
 *     responses:
 *       200:
 *         description: Comedero obtenido correctamente
 *         content:
 *           application/json:
 *             example:
 *               message: "Comedero obtenido correctamente"
 *               feederQuantity: 2
 *               feederName: "Comedero Cocina"
 *               feederLogo: "cat"
 *               lastConection: "2025-04-10T17:22:36.829Z"
 *
 *       400:
 *         description: El campo feederId está vacío o malformado
 *         content:
 *           application/json:
 *             example:
 *               message: "El campo 'feederId' no puede estar vacío"
 *
 *       401:
 *         description: API Key no enviada
 *         content:
 *           application/json:
 *             example:
 *               message: "Falta la API Key"
 *
 *       403:
 *         description: API Key inválida
 *         content:
 *           application/json:
 *             example:
 *               message: "API Key inválida"
 *
 *       404:
 *         description: No se encontró un comedero con ese ID
 *         content:
 *           application/json:
 *             example:
 *               errorCode: 404
 *               message: "No se encontró un comedero con esa ID"
 *
 *       500:
 *         description: Error interno al obtener o modificar datos del comedero
 *         content:
 *           application/json:
 *             examples:
 *               errorObtencion:
 *                 summary: Error al obtener comedero
 *                 value:
 *                   message: "Error al obtener comedero"
 *                   error: "Mensaje de error del servidor"
 *               errorActualizacion:
 *                 summary: Error al modificar estado del motor
 *                 value:
 *                   message: "No se pudo modificar estado del motor"
 *                   error: "Mensaje de error del servidor"
 */
router.get('/feeders/global/:feederId', authenticateDevice, feederController.getGlobalFeederById);

// Obtener todos los comederos (solo para admin o debugging si lo necesitás)
/**
 * @swagger
 * /feeders:
 *   get:
 *     summary: Obtener todos los comederos registrados
 *     tags: [Feeders]
 *     responses:
 *       200:
 *         description: Lista de comederos
 *         content:
 *           application/json:
 *             example:
 *               - feederId: MAC123
 *                 name: Cocina
 *                 userId: null
 *       400:
 *         description: Error al obtener los comederos
 *         content:
 *           application/json:
 *             example:
 *               message: Error al obtener los comederos
 */
router.get('/feeders', verifyToken, feederController.getAllFeeders);

// Obtener comederos asignados al usuario logueado
/**
 * @swagger
 * /feeders/my:
 *   get:
 *     summary: Obtener todos los comederos asignados al usuario autenticado
 *     description: Este endpoint permite obtener la lista de comederos asociados al usuario actualmente autenticado mediante JWT.
 *     tags:
 *       - Feeders
 *     security:
 *       - bearerAuth: []
 *     responses:
 *       200:
 *         description: Lista de comederos obtenida exitosamente
 *         content:
 *           application/json:
 *             example:
 *               - _id: "67f707fa9b66a6ee96d447a3"
 *                 feederId: "8"
 *                 userId: "67f6d3af066a14a9d9699bd3"
 *                 feederName: "Patsy.Nikolaus19"
 *                 feederLogo: "fish"
 *                 feederAsign: true
 *                 feederQuantity: 0
 *                 motorInfo:
 *                   motorState: false
 *                   startHours:
 *                     - "2025-04-10T20:48:00.000Z"
 *                     - "2025-04-10T20:49:00.000Z"
 *                 lastConection: "2025-04-10T17:22:36.829Z"
 *                 __v: 0
 *       401:
 *         description: Usuario no autenticado
 *         content:
 *           application/json:
 *             example:
 *               message: "Acceso no autorizado - usuario no autenticado."
 *       404:
 *         description: No se encontraron comederos para el usuario
 *         content:
 *           application/json:
 *             example:
 *               message: "No se encontraron comederos para este usuario."
 *       500:
 *         description: Error interno del servidor al obtener los comederos
 *         content:
 *           application/json:
 *             example:
 *               message: "Error al obtener los comederos"
 *               error: "Mensaje de error del servidor"
 */
router.get('/feeders/my', verifyToken, feederController.getMyFeeders);

// Obtener un comedero específico por feederId para el usuario logueado
/**
 * @swagger
 * /feeders/{feederId}:
 *   get:
 *     summary: Obtener un comedero por feederId para el usuario autenticado
 *     description: Este endpoint devuelve la información de un comedero específico que pertenece al usuario autenticado.
 *     tags:
 *       - Feeders
 *     security:
 *       - bearerAuth: []
 *     parameters:
 *       - in: path
 *         name: feederId
 *         required: true
 *         description: ID del comedero a buscar
 *         schema:
 *           type: string
 *     responses:
 *       200:
 *         description: Comedero encontrado y retornado con éxito
 *         content:
 *           application/json:
 *             example:
 *               _id: "67f707fa9b66a6ee96d447a3"
 *               feederId: "8"
 *               userId: "67f6d3af066a14a9d9699bd3"
 *               feederName: "Patsy.Nikolaus19"
 *               feederLogo: "fish"
 *               feederAsign: true
 *               feederQuantity: 0
 *               motorInfo:
 *                 motorState: false
 *                 startHours:
 *                   - "2025-04-10T20:48:00.000Z"
 *                   - "2025-04-10T20:49:00.000Z"
 *               lastConection: "2025-04-10T17:22:36.829Z"
 *               __v: 0
 *       400:
 *         description: Parámetro feederId faltante o vacío
 *         content:
 *           application/json:
 *             example:
 *               message: "El parámetro feederId es obligatorio y no puede estar vacío."
 *       401:
 *         description: Usuario no autenticado
 *         content:
 *           application/json:
 *             example:
 *               message: "Acceso no autorizado - usuario no identificado."
 *       404:
 *         description: No se encontró el comedero para el usuario
 *         content:
 *           application/json:
 *             example:
 *               message: "Comedero no encontrado para el usuario especificado."
 *       500:
 *         description: Error al consultar el comedero en la base de datos
 *         content:
 *           application/json:
 *             example:
 *               message: "Error al obtener comedero"
 *               error: "Mensaje del error del servidor"
 */
router.get('/feeders/:feederId', verifyToken, feederController.getFeederById);

// Asignar comedero al usuario logueado
/**
 * @swagger
 * /feeder/assign:
 *   post:
 *     summary: Asignar un comedero a un usuario autenticado
 *     description: Permite a un usuario autenticado reclamar la propiedad de un comedero disponible.
 *     tags:
 *       - Feeder
 *     security:
 *       - bearerAuth: []
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           example:
 *             feederId: "8"
 *             feederName: "Patsy.Nikolaus19"
 *             feederLogo: "fish"
 *     responses:
 *       200:
 *         description: Comedero asignado correctamente
 *         content:
 *           application/json:
 *             example:
 *               message: "Comedero asignado correctamente"
 *               informacion:
 *                 userId: "6633d3c8cc0eec6baf62e01b"
 *                 feederName: "Patsy.Nikolaus19"
 *                 feederLogo: "fish"
 *                 feederId: "8"
 *       400:
 *         description: Validación incorrecta en los campos de entrada o comedero ya asignado al mismo usuario
 *         content:
 *           application/json:
 *             examples:
 *               feederIdInvalido:
 *                 summary: Campo feederId inválido
 *                 value:
 *                   message: "El campo 'feederId' es requerido y debe ser un string válido."
 *               feederYaAsignado:
 *                 summary: Comedero ya asignado al mismo usuario
 *                 value:
 *                   message: "Este comedero ya está asignado a vos."
 *       401:
 *         description: Usuario no autenticado
 *         content:
 *           application/json:
 *             example:
 *               message: "No se pudo obtener el usuario autenticado."
 *       403:
 *         description: El comedero ya está asignado a otro usuario
 *         content:
 *           application/json:
 *             example:
 *               message: "Este comedero ya está asignado a otro usuario."
 *       404:
 *         description: Comedero no encontrado
 *         content:
 *           application/json:
 *             example:
 *               message: "Comedero no encontrado."
 *       500:
 *         description: Error interno del servidor
 *         content:
 *           application/json:
 *             example:
 *               message: "Error interno del servidor"
 *               error: "Detalle del error interno"
 */
router.post('/feeder/assign', verifyToken, feederController.assignFeeder);

// Desasignar comedero
/**
 * @swagger
 * /feeder/unassign:
 *   post:
 *     summary: Desasignar un comedero del usuario autenticado
 *     description: Permite al usuario liberar un comedero previamente asignado.
 *     tags:
 *       - Feeder
 *     security:
 *       - bearerAuth: []
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           example:
 *             feederId: "8"
 *     responses:
 *       200:
 *         description: Feeder desasignado con éxito
 *         content:
 *           application/json:
 *             example:
 *               message: "Feeder desasignado con éxito"
 *               feeder:
 *                 feederId: "8"
 *                 userId: null
 *                 feederName: null
 *                 feederLogo: null
 *                 feederAsign: false
 *                 lastConection: "2025-04-11T16:30:21.123Z"
 *       400:
 *         description: Campo faltante o el comedero ya está desasignado
 *         content:
 *           application/json:
 *             examples:
 *               userIdVacio:
 *                 summary: userId vacío
 *                 value:
 *                   message: "El campo 'userId' no puede estar vacío"
 *               feederYaDesasignado:
 *                 summary: Feeder ya desasignado
 *                 value:
 *                   message: "El feeder ya está desasignado"
 *       403:
 *         description: El comedero no le pertenece al usuario
 *         content:
 *           application/json:
 *             example:
 *               message: "Este comedero no te pertenece"
 *       404:
 *         description: Comedero no encontrado
 *         content:
 *           application/json:
 *             example:
 *               message: "Feeder no encontrado"
 *       500:
 *         description: Error interno del servidor al intentar desasignar
 *         content:
 *           application/json:
 *             example:
 *               message: "Error interno al desasignar el feeder"
 *               error: "Detalle del error"
 */
router.post('/feeder/unassign', verifyToken, feederController.unassignFeeder);

//Encender comedero
/**
 * @swagger
 * /feeder/start:
 *   post:
 *     summary: Enciende el motor de un comedero
 *     description: Permite al usuario autenticado encender el motor de un comedero asignado.
 *     tags:
 *       - Feeder
 *     security:
 *       - bearerAuth: []
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           example:
 *             feederId: "8"
 *     responses:
 *       200:
 *         description: Motor del comedero encendido con éxito
 *         content:
 *           application/json:
 *             example:
 *               message: "Feeder encendedido con éxito"
 *       400:
 *         description: El motor ya está encendido
 *         content:
 *           application/json:
 *             example:
 *               message: "El motor ya está encendido"
 *       403:
 *         description: El comedero no está asignado o pertenece a otro usuario
 *         content:
 *           application/json:
 *             examples:
 *               sinAsignar:
 *                 summary: Comedero no asignado
 *                 value:
 *                   message: "Este comedero no está asignado a ningún usuario"
 *               noTePertenece:
 *                 summary: Comedero pertenece a otro usuario
 *                 value:
 *                   message: "Este comedero no te pertenece"
 *       404:
 *         description: Comedero no encontrado
 *         content:
 *           application/json:
 *             example:
 *               message: "Feeder no encontrado"
 *       500:
 *         description: Error interno del servidor
 *         content:
 *           application/json:
 *             example:
 *               message: "Error interno al desasignar el feeder"
 *               error: "Detalle del error"
 */
router.post('/feeder/start', verifyToken,feederController.startMotor);

//Frenar motor
/**
 * @swagger
 * /feeder/stop:
 *   post:
 *     summary: Apaga el motor de un comedero
 *     description: Permite al usuario autenticado apagar el motor de un comedero que le pertenece.
 *     tags:
 *       - Feeder
 *     security:
 *       - bearerAuth: []
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           example:
 *             feederId: "8"
 *     responses:
 *       200:
 *         description: Motor del comedero apagado con éxito
 *         content:
 *           application/json:
 *             example:
 *               message: "Feeder apagado con éxito"
 *       400:
 *         description: El motor ya está apagado
 *         content:
 *           application/json:
 *             example:
 *               message: "El motor ya está apagado"
 *       403:
 *         description: El comedero no está asignado o pertenece a otro usuario
 *         content:
 *           application/json:
 *             examples:
 *               sinAsignar:
 *                 summary: Comedero no asignado
 *                 value:
 *                   message: "Este comedero no está asignado a ningún usuario"
 *               noTePertenece:
 *                 summary: Comedero pertenece a otro usuario
 *                 value:
 *                   message: "Este comedero no te pertenece"
 *       404:
 *         description: Comedero no encontrado
 *         content:
 *           application/json:
 *             example:
 *               message: "Feeder no encontrado"
 *       500:
 *         description: Error interno del servidor
 *         content:
 *           application/json:
 *             example:
 *               message: "Error interno al desasignar el feeder"
 *               error: "Detalle del error"
 */
router.post('/feeder/complete', authenticateDevice, feederController.completeMotorCommand);


//Editar comedero
/**
 * @swagger
 * /feeder/edit:
 *   post:
 *     summary: Editar comedero
 *     description: Permite editar los datos (nombre y logo) de un comedero siempre que esté asignado al usuario autenticado.
 *     tags:
 *       - Feeder
 *     security:
 *       - bearerAuth: []
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           example:
 *             feederId: "8"
 *             feederName: "Comedero de Luna"
 *             feederLogo: "logo8.png"
 *     responses:
 *       200:
 *         description: Comedero editado correctamente
 *         content:
 *           application/json:
 *             example:
 *               message: "Comedero editado correctamente"
 *               informacion:
 *                 userId: "661526d645e78cf30225f9dd"
 *                 feederName: "Comedero de Luna"
 *                 feederLogo: "logo8.png"
 *                 feederId: "8"
 *       400:
 *         description: Error de validación en los campos requeridos
 *         content:
 *           application/json:
 *             examples:
 *               sinFeederId:
 *                 summary: Falta el feederId
 *                 value:
 *                   message: "El campo 'feederId' es requerido y debe ser un string válido."
 *               sinFeederName:
 *                 summary: Falta el nombre
 *                 value:
 *                   message: "El campo 'feederName' es requerido y debe ser un string válido."
 *               sinFeederLogo:
 *                 summary: Falta el logo
 *                 value:
 *                   message: "El campo 'feederLogo' es requerido y debe ser un string."
 *       401:
 *         description: Usuario no autenticado
 *         content:
 *           application/json:
 *             example:
 *               message: "No se pudo obtener el usuario autenticado."
 *       403:
 *         description: El comedero está asignado a otro usuario
 *         content:
 *           application/json:
 *             example:
 *               message: "Este comedero esta asignado a otro usuario, no se puede editar."
 *       404:
 *         description: Comedero no encontrado
 *         content:
 *           application/json:
 *             example:
 *               message: "Comedero no encontrado."
 *       500:
 *         description: Error interno del servidor
 *         content:
 *           application/json:
 *             example:
 *               message: "Error interno del servidor"
 *               error: "Detalle del error"
 */
router.post('/feeder/edit', verifyToken, feederController.editFeeder);

//Obtener estado motor (Solo nodemcu)
/**
 * @swagger
 * /feeder/motor-state/{feederId}:
 *   get:
 *     summary: Obtener el estado del motor de un comedero (solo para NODEMCU)
 *     description: Este endpoint devuelve el estado del motor de un comedero, pero solo se permite el acceso mediante una API Key válida para NODEMCU.
 *     tags:
 *       - Feeder
 *     parameters:
 *       - name: feederId
 *         in: path
 *         required: true
 *         description: ID único del comedero
 *         schema:
 *           type: string
 *     security:
 *       - apiKeyAuth: []
 *     responses:
 *       200:
 *         description: Estado del motor del comedero obtenido correctamente
 *         content:
 *           application/json:
 *             example:
 *               motorState: true
 *       400:
 *         description: El parámetro `feederId` es obligatorio y no puede estar vacío
 *         content:
 *           application/json:
 *             example:
 *               message: "El parámetro feederId es obligatorio y no puede estar vacío."
 *       401:
 *         description: Falta la API Key en los headers
 *         content:
 *           application/json:
 *             example:
 *               message: "Falta la API Key"
 *       403:
 *         description: API Key inválida
 *         content:
 *           application/json:
 *             example:
 *               message: "API Key inválida"
 *       404:
 *         description: No se encontró el comedero con el ID proporcionado
 *         content:
 *           application/json:
 *             example:
 *               message: "Comedero no encontrado para el usuario especificado."
 *       500:
 *         description: Error interno al intentar obtener el estado del comedero
 *         content:
 *           application/json:
 *             example:
 *               message: "Error al obtener comedero"
 *               error: "Detalle del error"
 */
router.get('/feeders/motor-state/:feederId', authenticateDevice, feederController.getMotorStatusNodemcu);

//Obtener estado motor
/**
 * @swagger
 * /feeders/state/{feederId}:
 *   get:
 *     summary: Obtener el estado del motor de un comedero
 *     description: Este endpoint devuelve el estado del motor de un comedero. Se requiere el `feederId` como parámetro en la URL.
 *     tags:
 *       - Feeder
 *     parameters:
 *       - name: feederId
 *         in: path
 *         required: true
 *         description: ID único del comedero
 *         schema:
 *           type: string
 *     responses:
 *       200:
 *         description: Estado del motor del comedero obtenido correctamente
 *         content:
 *           application/json:
 *             example:
 *               motorState: true
 *       400:
 *         description: El parámetro `feederId` es obligatorio y no puede estar vacío
 *         content:
 *           application/json:
 *             example:
 *               message: "El parámetro feederId es obligatorio y no puede estar vacío."
 *       404:
 *         description: No se encontró el comedero con el ID proporcionado
 *         content:
 *           application/json:
 *             example:
 *               message: "Comedero no encontrado para el usuario especificado."
 *       500:
 *         description: Error interno al intentar obtener el estado del comedero
 *         content:
 *           application/json:
 *             example:
 *               message: "Error al obtener comedero"
 *               error: "Detalle del error"
 */
router.get('/feeder/state/:feederId', verifyToken, feederController.getMotorStatus);

//Obtener fechas users
/**
 * @swagger
 * /feeders/dates/{feederId}:
 *   get:
 *     summary: Obtener las fechas de inicio del motor de un comedero
 *     description: Este endpoint devuelve las fechas de inicio del motor de un comedero, que están almacenadas en el campo `startHours` del comedero. Se requiere el `feederId` como parámetro en la URL.
 *     tags:
 *       - Feeder
 *     parameters:
 *       - name: feederId
 *         in: path
 *         required: true
 *         description: ID único del comedero
 *         schema:
 *           type: string
 *     responses:
 *       200:
 *         description: Fechas de inicio del motor obtenidas correctamente
 *         content:
 *           application/json:
 *             example:
 *               dates: ["2025-04-12T08:00:00", "2025-04-13T08:00:00"]
 *       400:
 *         description: El parámetro `feederId` es obligatorio y no puede estar vacío
 *         content:
 *           application/json:
 *             example:
 *               message: "El parámetro feederId es obligatorio y no puede estar vacío."
 *       404:
 *         description: No se encontró el comedero con el ID proporcionado
 *         content:
 *           application/json:
 *             example:
 *               message: "Comedero no encontrado para el usuario especificado."
 *       500:
 *         description: Error interno al intentar obtener las fechas del comedero
 *         content:
 *           application/json:
 *             example:
 *               message: "Error al obtener comedero"
 *               error: "Detalle del error"
 */
router.get('/feeder/dates/:feederId', verifyToken, feederController.getFechasByFeederId);

//Agregar Hora Comedero
/**
 * @swagger
 * /feeder/add-hour:
 *   post:
 *     summary: Asignar horas programadas al comedero (versión dinámica)
 *     description: Este endpoint permite asignar nuevas horas de inicio al comedero, asegurándose de que las fechas sean válidas, no repetidas y futuras. Se requiere el `feederId` en la URL y un arreglo de fechas en el cuerpo de la solicitud.
 *     tags:
 *       - Feeder
 *     parameters:
 *       - name: feederId
 *         in: path
 *         required: true
 *         description: ID único del comedero
 *         schema:
 *           type: string
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             properties:
 *               feederId:
 *                 type: string
 *                 description: ID del comedero
 *               dates:
 *                 type: array
 *                 items:
 *                   type: string
 *                   format: date-time
 *                 description: Fechas a asignar al comedero
 *           example:
 *             feederId: "feeder123"
 *             dates: ["2025-04-12T08:00:00", "2025-04-13T08:00:00"]
 *     responses:
 *       200:
 *         description: Fechas agregadas correctamente al comedero
 *         content:
 *           application/json:
 *             example:
 *               message: "Fechas agregadas exitosamente."
 *               dates: ["2025-04-12T08:00:00", "2025-04-13T08:00:00"]
 *       400:
 *         description: Error en el formato o fechas inválidas
 *         content:
 *           application/json:
 *             example:
 *               message: "Se debe enviar un arreglo de fechas."
 *       404:
 *         description: No se encontró el comedero para el usuario
 *         content:
 *           application/json:
 *             example:
 *               message: "Comedero no encontrado para el usuario."
 *       500:
 *         description: Error interno al intentar agregar fechas al comedero
 *         content:
 *           application/json:
 *             example:
 *               message: "Error al agregar fechas."
 *               error: "Detalle del error"
 */
router.post('/feeder/add-hour', verifyToken, feederController.addStartHours);

//Delete feeder
router.delete('/feeder/:feederId', verifyToken, feederController.deleteFeeder);

//Obtener historial de ejecs
router.get('/feeder/:feederId/historial', verifyToken, feederController.getFeederHistory);

//Agrega historial que se mantiene en la persinstencia en modo offline
router.post('/feeders/history',authenticateDevice,feederController.syncFeedingHistory);

//Comunica el estado del dispositivo hacia el backend
router.post("/feeders/heartbeat", authenticateDevice,feederController.heartbeat);

router.get('/feeders/config/:feederId',authenticateDevice,feederController.getRemoteConfiguration);

//Edita configuracion defeeder
router.put('/feeders/:feederId/config',verifyToken,feederController.updateFeederConfiguration);

module.exports = router;
