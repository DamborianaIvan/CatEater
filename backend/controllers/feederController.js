const Feeder = require("../models/Feeder");
const crypto = require("crypto");
require('../jobs/motorStatusJob');
const {
  generateDeviceCredential,
  hashDeviceCredential
} = require("../utils/deviceCredential");

const MAX_SCHEDULES = 5;
const MIN_STEPS_PER_FEED = 1;
const MAX_STEPS_PER_FEED = 10240;
const MIN_PORTIONS = 1;
const MAX_PORTIONS = 5;

const createDefaultSchedules = () =>
  Array.from({ length: MAX_SCHEDULES }, () => ({
    hour: 0,
    minute: 0,
    portions: 1,
    enabled: false
  }));

const validateConfiguration = (configuration) => {
  if (!configuration || typeof configuration !== "object") {
    return "La configuración es obligatoria.";
  }

  const {
    stepsPerFeed,
    schedules
  } = configuration;

  if (
    !Number.isInteger(stepsPerFeed) ||
    stepsPerFeed < MIN_STEPS_PER_FEED ||
    stepsPerFeed > MAX_STEPS_PER_FEED
  ) {
    return `stepsPerFeed debe estar entre ${MIN_STEPS_PER_FEED} y ${MAX_STEPS_PER_FEED}.`;
  }

  if (!Array.isArray(schedules)) {
    return "schedules debe ser un arreglo.";
  }

  if (schedules.length !== MAX_SCHEDULES) {
    return `schedules debe contener exactamente ${MAX_SCHEDULES} elementos.`;
  }

  for (const schedule of schedules) {
    if (
      !Number.isInteger(schedule.hour) ||
      schedule.hour < 0 ||
      schedule.hour > 23
    ) {
      return "Hora de schedule inválida.";
    }

    if (
      !Number.isInteger(schedule.minute) ||
      schedule.minute < 0 ||
      schedule.minute > 59
    ) {
      return "Minuto de schedule inválido.";
    }

    if (
      !Number.isInteger(schedule.portions) ||
      schedule.portions < MIN_PORTIONS ||
      schedule.portions > MAX_PORTIONS
    ) {
      return "Cantidad de porciones inválida.";
    }

    if (typeof schedule.enabled !== "boolean") {
      return "enabled debe ser boolean.";
    }
  }

  return null;
};

//get configuracion dle feeder
const getRemoteConfiguration = async (req, res) => {
  const { feederId } = req.params;

  if (!feederId || feederId.trim() === "") {
    return res.status(400).json({
      error: "feederId es requerido"
    });
  }

  try {
    const feeder = await Feeder.findOne({ feederId });

    if (!feeder) {
      return res.status(404).json({
        error: "Feeder no encontrado"
      });
    }

    let configuration = feeder.configuration;

    // Compatibilidad con feeders creados antes de esta configuración.
    if (!configuration) {
      configuration = {
        revision: 1,
        stepsPerFeed: 2048,
        schedules: createDefaultSchedules()
      };
    }
    const schedules = configuration.schedules.map((schedule) => ({
    hour: schedule.hour,
    minute: schedule.minute,
    portions: schedule.portions,
    enabled: schedule.enabled
  }));

  return res.status(200).json({
    revision: configuration.revision,
    stepsPerFeed: configuration.stepsPerFeed,
    schedules
  });

  } catch (error) {
    console.error(
      "Error al obtener configuración del feeder:",
      error
    );

    return res.status(500).json({
      error: "Error interno del servidor"
    });
  }
};

// Registrar comedero (desde NodeMCU)
const registerFeeder = async (req, res) => {
const apiKey = req.headers['x-api-key'];

  if (apiKey !== process.env.NODEMCU_API_KEY) {
    return res.status(401).json({ error: 'No autorizado - API Key inválida' });
  }

  const feederInfo = req.body;
  console.log("[REGISTER] feederId:", feederInfo.feederId);
  console.log("[REGISTER] MongoDB:", Feeder.db.name);
  if (!feederInfo || Object.keys(feederInfo).length === 0) {
    return res.status(400).json({ error: 'Datos faltantes' });
  }

  try {
    const existedFeeder = await Feeder.findOne({
  feederId: feederInfo.feederId
});

console.log(
  "[REGISTER] feeder encontrado:",
  existedFeeder?.feederId
);
    const existingFeeder = await Feeder.findOne({ feederId: feederInfo.feederId });
    if (existingFeeder) {
      return res.status(409).json({ error: 'Ya existe un dispositivo con ese feederId' });
    }

    const newFeeder = new Feeder({
        feederId: feederInfo.feederId,
        feederName: feederInfo.feederName
    });

    await newFeeder.save();

    return res.status(201).json({ message: 'Comedero guardado correctamente' });
  } catch (error) {
    console.error('Error al guardar comedero:', error);
    return res.status(500).json({ error: 'Hubo un error con el servidor' });
  }
};

//enrolar feeder leugo del register
const enrollDevice = async (req, res) => {
  const apiKey = req.headers["x-api-key"];
  const { feederId } = req.body || {};

  if (apiKey !== process.env.NODEMCU_API_KEY) {
    return res.status(401).json({
      error: "No autorizado - API Key inválida"
    });
  }

  if (!feederId || typeof feederId !== "string" || feederId.trim() === "") {
    return res.status(400).json({
      error: "feederId es requerido"
    });
  }

  try {
    const feeder = await Feeder.findOne({ feederId }).select("+deviceCredentialHash");

    if (!feeder) {
      return res.status(404).json({
        error: "Feeder no encontrado"
      });
    }

    if (feeder.deviceCredentialHash) {
      return res.status(409).json({
        error: "El dispositivo ya está enrolado"
      });
    }

    const deviceCredential = generateDeviceCredential();
    feeder.deviceCredentialHash = hashDeviceCredential(deviceCredential);

    await feeder.save();

    return res.status(201).json({
      message: "Dispositivo enrolado correctamente",
      deviceCredential
    });
  } catch (error) {
    console.error("Error al enrolar dispositivo:", error);

    return res.status(500).json({
      error: "Error interno del servidor"
    });
  }
};

// Obtener todos los comederos
const getAllFeeders = async (req, res) => {
  try {
    const feeders = await Feeder.find();
    res.json(feeders);
  } catch (error) {
    res.status(400).json({ message: "Error al obtener los comederos", error });
  }
};

// Obtener comederos del usuario
const getMyFeeders = async (req, res) => {
    // Verificar que el usuario esté autenticado y que exista su _id
    if (!req.user || !req.user._id) {
      return res.status(401).json({ message: "Acceso no autorizado - usuario no autenticado." });
    }
    try {
      const feeders = await Feeder.find({ userId: req.user._id });
      // Verificar si se encontraron comederos para el usuario
      if (!feeders || feeders.length === 0) {
        return res.status(200).json({ message: "No se encontraron comederos para este usuario.", feeders:[] });
      }
      res.json(feeders);
    } catch (error) {
      res.status(500).json({ message: "Error al obtener los comederos", error: error.message });
    }
  };
  
// Obtener un comedero por feederId y userId
const getFeederById = async (req, res) => {
    const { feederId } = req.params;
  
    // Validar que feederId exista y no esté vacío
    if (!feederId || feederId.trim() === "") {
      return res.status(400).json({ message: "El parámetro feederId es obligatorio y no puede estar vacío." });
    }
  
    // Validar que req.user esté definido (esto debería estar garantizado por el middleware)
    if (!req.user || !req.user._id) {
      return res.status(401).json({ message: "Acceso no autorizado - usuario no identificado." });
    }
  
    try {
      const feeder = await Feeder.findOne({
        feederId,
        userId: req.user._id
      });
  
      if (!feeder) {
        return res.status(404).json({ message: "Comedero no encontrado para el usuario especificado." });
      }
  
      res.json(feeder);
    } catch (error) {
      res.status(500).json({ message: "Error al obtener comedero", error: error.message });
    }
  };
  
// Obtener feeder por feederId (desde NodeMCU)
const getGlobalFeederById = async (req, res) => {
  const feederId = req.params.feederId;

  if (!feederId || feederId.trim() === "") return res.status(400).json({ message: "El campo 'feederId' no puede estar vacío" });

  try {
    const feeder = await Feeder.findOne({ feederId });

    if (!feeder) {
      return res.status(404).json({
        errorCode: 404,
        message: "No se encontró un comedero con esa ID"
      });
    }

    const now = new Date(); // hora actual en UTC
    let motorShouldRun = false;

    // Comprobar si la hora actual coincide con alguna programada
    for (const scheduledDate of feeder.motorInfo.startHours) {
      if (!scheduledDate) continue;

      const sched = new Date(scheduledDate);
      if (!null &&
        now.getUTCHours() === sched.getUTCHours() &&
        now.getUTCMinutes() === sched.getUTCMinutes()
      ) {
        motorShouldRun = true;
        break;
      }
    }
    try {
      //Modifico el estado del motor a true
      if (motorShouldRun){
        const updatedFeeder = await Feeder.findOneAndUpdate(
          { feederId },
          {
            $set: {
              'motorInfo.motorState': motorShouldRun,
              lastConexion: Date.now()
            }
          },
          { new: true }
        );
      }
    } catch (error) {
      console.error(error);
      return res.status(500).json({ message: "No se pudo modificar estado del motor", error: error.message });
    }
  return res.status(200).json({
    message: "Comedero obtenido correctamente",
    feederQuantity: feeder.feederQuantity,
    feederName:feeder.feederName,
    feederLogo:feeder.feederLogo,
    lastConection: feeder.lastConection        
  });
  } catch (error) {
    console.error(error);
    return res.status(500).json({ message: "Error al obtener comedero", error: error.message });
  }
};

//Asignar comedero
const assignFeeder = async (req, res) => {
  try {
    const { feederId, feederName, feederLogo } = req.body;
    const userId = req.user?._id; // viene del token decodificado por middleware

    // Validaciones básicas de campos
    if (!feederId || typeof feederId !== "string" || feederId.trim() === "") {
      return res.status(400).json({ message: "El campo 'feederId' es requerido y debe ser un string válido." });
    }

    if (!feederName || typeof feederName !== "string" || feederName.trim() === "") {
      return res.status(400).json({ message: "El campo 'feederName' es requerido y debe ser un string válido." });
    }

    if (!feederLogo || typeof feederLogo !== "string") {
      return res.status(400).json({ message: "El campo 'feederLogo' es requerido y debe ser un string." });
    }

    if (!userId) {
      return res.status(401).json({ message: "No se pudo obtener el usuario autenticado." });
    }

    // Buscar el comedero por ID
    const feeder = await Feeder.findOne({ feederId });

    if (!feeder) {
      return res.status(404).json({ message: "Comedero no encontrado." });
    }

    // Validar si ya está asignado a otro usuario
    if (feeder.userId && feeder.userId.toString() !== userId.toString()) {
      return res.status(403).json({ message: "Este comedero ya está asignado a otro usuario." });
    }

    // Validar si ya está asignado a este mismo usuario
    if (feeder.userId && feeder.userId.toString() === userId.toString()) {
      return res.status(400).json({ message: "Este comedero ya está asignado a vos." });
    }

    // Asignar el comedero
    feeder.userId = userId;
    feeder.feederName = feederName;
    feeder.feederLogo = feederLogo;
    feeder.feederAsign = true;
    feeder.lastConexion = Date.now();

    await feeder.save();

    res.status(200).json({
      message: "Comedero asignado correctamente",
      informacion: {
        userId,
        feederName,
        feederLogo,
        feederId
      }
    });

  } catch (err) {
    console.error("Error al asignar comedero:", err);
    res.status(500).json({ message: "Error interno del servidor", error: err.message });
  }
};

// Asignar horas programadas al feeder (versión dinámica)
const addStartHours = async (req, res) => {
  const { feederId, dates } = req.body;
  const userId = req.user._id;

  if (!Array.isArray(dates) || dates.length === 0) {
    return res.status(400).json({ message: "Se debe enviar un arreglo de fechas." });
  }

  try {
    const feeder = await Feeder.findOne({ feederId, userId });

    if (!feeder) {
      return res.status(404).json({ message: "Comedero no encontrado para el usuario." });
    }

    const now = new Date();
    const existingDates = feeder.motorInfo.startHours.map(date => new Date(date).getTime());

    const newValidDates = dates
      .map(d => new Date(d))
      .filter(date =>
        !isNaN(date) &&                          // fecha válida
        date > now &&                            // no es pasada
        !existingDates.includes(date.getTime())  // no está repetida
      );

    if (newValidDates.length === 0) {
      return res.status(400).json({ message: "No se agregaron fechas válidas." });
    }

    feeder.motorInfo.startHours.push(...newValidDates);
    await feeder.save();

    res.status(200).json({
      message: "Fechas agregadas exitosamente.",
      dates: feeder.motorInfo.startHours
    });
  } catch (error) {
    res.status(500).json({ message: "Error al agregar fechas.", error: error.message });
  }
};

//Desasignar comedero
const unassignFeeder = async (req, res) => {
  try {
    const {feederId}  = req.body;
    const userId = req.user._id;
    
    if (!userId || userId.trim() === "") {
      return res.status(400).json({ message: "El campo 'userId' no puede estar vacío" });
    }

    // Buscar el feeder
    const existingFeeder = await Feeder.findOne({ feederId });

    if (!existingFeeder) {
      return res.status(404).json({ message: "Feeder no encontrado" });
    }

    // Verificar si el feeder ya está desasignado
    if (!existingFeeder.feederAsign || existingFeeder.userId===null) {
      return res.status(400).json({ message: "El feeder no esta asignado a ninguna persona" });
    }

    // Verificar si el feeder le pertenece al usuario que lo quiere desasignar
    if (existingFeeder.userId.toString() !== userId) {
      return res.status(403).json({ message: "Este comedero no te pertenece" });
    }

    // Desasignar el feeder
    const feeder = await Feeder.findOneAndUpdate(
      { feederId },
      {
        $set: {
          userId: null,
          feederName: null,
          feederLogo:null,
          feederAsign: false,
          lastConection: Date.now()
        }
      },
      { new: true }
    );

    return res.status(200).json({
      message: "Feeder desasignado con éxito",
      feeder
    });

  } catch (err) {
    console.error("Error al desasignar feeder:", err);
    res.status(500).json({ message: "Error interno al desasignar el feeder", error: err.message });
  }
};

//Encender motor
const startMotor = async (req, res)=>{
  const { feederId, portions = 1 } = req.body;
  const userId = req.user._id;
  const commandId = crypto.randomUUID();

  try{
    // Buscar el feeder
    const existingFeeder = await Feeder.findOne({ feederId });

    if (!existingFeeder) {
      return res.status(404).json({ message: "Feeder no encontrado" });
    }

    if (!Number.isInteger(portions) || portions <= 0) {
    return res.status(400).json({
        message: "Cantidad de porciones inválida"
    });
}
    // // Validar si el comedero tiene un userId asignado
    // if (!existingFeeder.userId) {
    //   return res.status(403).json({ message: "Este comedero no está asignado a ningún usuario" });
    // }

    // // Verificar si el feeder le pertenece al usuario que lo quiere encender
    // if (existingFeeder.userId.toString() !== userId) {
    //   return res.status(403).json({ message: "Este comedero no te pertenece" });
    // }
    
    // Verificar si ya está encendido
    if (existingFeeder.motorInfo?.motorState === true) {
      return res.status(400).json({ message: "El motor ya está encendido" });
    }

    // Actualizamos info en la BD
    const feeder = await Feeder.findOneAndUpdate(
      { feederId },
      {
        $set: {
          "motorInfo.motorState": true,
          "motorInfo.portions": portions,
          "motorInfo.commandId": commandId,
          lastConection: Date.now()
        }
      },
      { new: true }
    );
    return res.status(200).json({
      message: "Feeder encendido con éxito",
      commandId
    });
    
  }catch(err){
      console.error("Error al encender feeder:", err);
      res.status(500).json({ message: "Error interno al desasignar el feeder", error: err.message });
  }
}

//apagar motor
const stopMotorFromNodemcu = async (req, res)=>{
  const {feederId} = req.body;
  const userId = req.user._id;

  try{
    // Buscar el feeder
    const existingFeeder = await Feeder.findOne({ feederId });

    if (!existingFeeder) {
      return res.status(404).json({ message: "Feeder no encontrado" });
    }

    // Validar si el comedero tiene un userId asignado
    if (!existingFeeder.userId) {
      return res.status(403).json({ message: "Este comedero no está asignado a ningún usuario" });
    }

    // Verificar si el feeder le pertenece al usuario que lo quiere encender
    if (existingFeeder.userId.toString() !== userId) {
      return res.status(403).json({ message: "Este comedero no te pertenece" });
    }
    
    // Verificar si ya está encendido
    if (existingFeeder.motorInfo?.motorState === true) {
      return res.status(400).json({ message: "El motor ya está encendido" });
    }

    // Desasignar el feeder
    const feeder = await Feeder.findOneAndUpdate(
      { feederId },
      {
        $set: {
          'motorInfo.motorState':false,
          lastConection: Date.now()
        }
      },
      { new: true }
    );

    return res.status(200).json({
      message: "Feeder encendedido con éxito"
    });
  }catch(err){
      console.error("Error al encender feeder:", err);
      res.status(500).json({ message: "Error interno al desasignar el feeder", error: err.message });
  }
}

//completar ejecucion de motor
const completeMotorCommand = async (req, res) => {
 
  const { feederId, commandId } = req.body;

  if (!feederId || !commandId) {
    return res.status(400).json({
        error: "feederId y commandId son requeridos"
    });
}
  
  try {
    const feeder = await Feeder.findOne({ feederId });

    if (!feeder) {
      return res.status(404).json({
        error: "Feeder no encontrado"
      });
    }

    if (!feeder.motorInfo?.motorState) {
      if (feeder.motorInfo?.commandId === commandId) {
        return res.status(200).json({
          message: "Orden ya completada"
        });
      }

      return res.status(409).json({
        error: "No hay una orden pendiente para este commandId"
      });
    }

    if (feeder.motorInfo.commandId !== commandId) {
      return res.status(409).json({
        error: "La orden no coincide con la orden activa"
      });
    }
    const portions = feeder.motorInfo?.portions || 1;

    await Feeder.updateOne(
      { feederId },
      {
        $set: {
          "motorInfo.motorState": false,
          lastConection: Date.now()
        },
        $push: {
          feederHistory: {
            fecha: Date.now(),
            portions
          }
        }
      }
    );
    
    return res.status(200).json({
      message: "Orden de alimentación completada"
    });
  } catch (error) {
    console.error("Error al completar orden:", error);

    return res.status(500).json({
      error: "Error interno del servidor"
    });
  }
};

//agregar historial guardado en la little.fs
const syncFeedingHistory = async (req, res) => {
  const {
    eventId,
    feederId,
    timestamp,
    portions,
    source
  } = req.body;

  if (
    !eventId ||
    !feederId ||
    timestamp === undefined ||
    timestamp === null ||
    portions === undefined ||
    portions === null ||
    !source
  ) {
    return res.status(400).json({
      error: "Datos de alimentación incompletos"
    });
  }

  try {
    const feeder = await Feeder.findOne({ feederId });
    
    if (!feeder) {
      return res.status(404).json({
        error: "Feeder no encontrado"
      });
    }
    
    //Valisamos duplicado
    const alreadyExists = feeder.feederHistory.some(
      event => event.eventId === eventId
    );

    if (alreadyExists) {
      return res.status(200).json({
        message: "Alimentación ya sincronizada"
      });
    }
    feeder.feederHistory.push({
      eventId,
      fecha: new Date(timestamp * 1000),
      portions,
      source
    });

    await feeder.save();

    return res.status(200).json({
      message: "Alimentación sincronizada"
    });
  } catch (error) {
    console.error(
      "Error al sincronizar alimentación:",
      error
    );

    return res.status(500).json({
      error: "Error interno del servidor"
    });
  }
};

const editFeeder = async (req,res)=> {
  try {
    const { feederId, feederName, feederLogo } = req.body;
    const userId = req.user?._id; // viene del token decodificado por middleware

    // Validaciones básicas de campos
    if (!feederId || typeof feederId !== "string" || feederId.trim() === "") {
      return res.status(400).json({ message: "El campo 'feederId' es requerido y debe ser un string válido." });
    }

    if (!feederName || typeof feederName !== "string" || feederName.trim() === "") {
      return res.status(400).json({ message: "El campo 'feederName' es requerido y debe ser un string válido." });
    }

    if (!feederLogo || typeof feederLogo !== "string") {
      return res.status(400).json({ message: "El campo 'feederLogo' es requerido y debe ser un string." });
    }

    if (!userId) {
      return res.status(401).json({ message: "No se pudo obtener el usuario autenticado." });
    }

    // Buscar el comedero por ID
    const feeder = await Feeder.findOne({ feederId });

    if (!feeder) {
      return res.status(404).json({ message: "Comedero no encontrado." });
    }

    // Validar si ya está asignado a otro usuario
    if (feeder.userId && feeder.userId.toString() !== userId.toString()) {
      return res.status(403).json({ message: "Este comedero esta asignado a otro usuario, no se puede editar." });
    }

    if (feeder.userId == null){
      return res.status(403).json({ message: "Este comedero no esta asignado a ningun usuario, no se puede editar." });
    }
    // Asignar el comedero

    feeder.feederName = feederName;
    feeder.feederLogo = feederLogo;
    feeder.lastConexion = Date.now();

    await feeder.save();

    res.status(200).json({
      message: "Comedero editado correctamente",
      informacion: {
        feederName,
        feederLogo,
        feederId
      }
    });

  } catch (err) {
    console.error("Error al asignar comedero:", err);
    res.status(500).json({ message: "Error interno del servidor", error: err.message });
  }
}

//Get motor status pero solo para NODEMCU
const getMotorStatusNodemcu = async (req, res) =>{
const feederId = req.device.feederId;
  // Validar que feederId exista y no esté vacío
  if (!feederId || feederId.trim() === "") {
    return res.status(400).json({ message: "El parámetro feederId es obligatorio y no puede estar vacío." });
  }

  // // Validar que req.user esté definido (esto debería estar garantizado por el middleware)
  // if (!req.user || !req.user._id) {
  //   return res.status(401).json({ message: "Acceso no autorizado - usuario no identificado." });
  // }

  try {
    const feeder = await Feeder.findOne({
      feederId
    });

    if (!feeder) {
      return res.status(404).json({ message: "Comedero no encontrado para el usuario especificado." });
    }

    res.json({
  motorState: feeder.motorInfo.motorState,
  portions: feeder.motorInfo.portions,
  commandId: feeder.motorInfo.commandId,
  configRevision: feeder.configuration?.revision ?? 1
});
  } catch (error) {
    res.status(500).json({ message: "Error al obtener comedero", error: error.message });
  }

}

const updateFeederConfiguration = async (req, res) => {
  const { feederId } = req.params;

  const validationError = validateConfiguration(req.body);

  if (validationError) {
    return res.status(400).json({
      message: validationError
    });
  }

  try {
    const feeder = await Feeder.findOne({
      feederId
    });

    if (!feeder) {
      return res.status(404).json({
        message: "Comedero no encontrado para el usuario."
      });
    }

    const currentConfiguration = feeder.configuration || {
      revision: 1,
      stepsPerFeed: 2048,
      schedules: createDefaultSchedules()
    };

    const newConfiguration = {
      stepsPerFeed: req.body.stepsPerFeed,
      schedules: req.body.schedules
    };

    const configurationChanged =
      currentConfiguration.stepsPerFeed !==
        newConfiguration.stepsPerFeed ||
      JSON.stringify(currentConfiguration.schedules) !==
        JSON.stringify(newConfiguration.schedules);

    if (!configurationChanged) {
      return res.status(200).json({
        message: "La configuración no tuvo cambios.",
        revision: currentConfiguration.revision,
        configuration: currentConfiguration
      });
    }

    const newRevision =
      (currentConfiguration.revision || 1) + 1;

    feeder.configuration = {
      revision: newRevision,
      stepsPerFeed: newConfiguration.stepsPerFeed,
      schedules: newConfiguration.schedules
    };

    await feeder.save();

    return res.status(200).json({
      message: "Configuración actualizada correctamente.",
      revision: newRevision,
      configuration: feeder.configuration
    });

  } catch (error) {
    console.error(
      "Error al actualizar configuración:",
      error
    );

    return res.status(500).json({
      message: "Error interno al actualizar configuración."
    });
  }
};

//Get motor status pero solo para NODEMCU
const getMotorStatus = async (req, res) =>{

  const { feederId } = req.params;
  const userId = req.user._id;

  // Validar que feederId exista y no esté vacío
  if (!feederId || feederId.trim() === "") {
    return res.status(400).json({ message: "El parámetro feederId es obligatorio y no puede estar vacío." });
  }

  // Validar que req.user esté definido (esto debería estar garantizado por el middleware)
  if (!req.user || !req.user._id) {
    return res.status(401).json({ message: "Acceso no autorizado - usuario no identificado." });
  }

  try {
    const feeder = await Feeder.findOne({
      feederId,
      userId
    });

    if (!feeder) {
      return res.status(404).json({ message: "Comedero no encontrado para el usuario especificado." });
    }

    res.status(200).json({motorState:feeder.motorInfo.motorState});
  } catch (error) {
    res.status(500).json({ message: "Error al obtener comedero", error: error.message });
  }

}

const getFechasByFeederId = async (req, res)=>{
  const { feederId } = req.params;
  const userId = req.user._id;

  // Validar que feederId exista y no esté vacío
  if (!feederId || feederId.trim() === "") {
    return res.status(400).json({ message: "El parámetro feederId es obligatorio y no puede estar vacío." });
  }

  try {
    const feeder = await Feeder.findOne({
      userId,
      feederId
    });

    if (!feeder) {
      return res.status(404).json({ message: "Comedero no encontrado para el usuario especificado." });
    }

    res.status(200).json({dates:feeder.motorInfo.startHours});
  } catch (error) {
    res.status(500).json({ message: "Error al obtener comedero", error: error.message });
  }
}

// Eliminar comederO, SIRVE SOLO PARA DEV
const deleteFeeder = async (req, res) => {
  const { feederId } = req.params;

  if (!feederId || typeof feederId !== "string" || feederId.trim() === "") {
    return res.status(400).json({ message: "El parámetro feederId es obligatorio y debe ser una cadena no vacía" });
  }

  try {
    const deletedFeeder = await Feeder.findOneAndDelete({ feederId });

    if (!deletedFeeder) {
      return res.status(404).json({ message: "Comedero no encontrado o ya eliminado" });
    }

    res.json({ message: "Comedero eliminado correctamente" });
  } catch (error) {
    console.error("Error al eliminar comedero:", error);
    res.status(500).json({ message: "Error al eliminar comedero", error: error.message });
  }
};

//Obtener historial de un feeder
const getFeederHistory = async (req, res) => {
  try {
    const { feederId } = req.params;
    const userId = req.user._id;

    const feeder = await Feeder.findOne({
      userId,
      feederId
    });

    if (!feeder) {
      return res.status(404).json({
        message: "Comedero no encontrado para el usuario especificado."
      });
    }

    const historial = feeder.feederHistory.map((evento, i) => {
      const fechaObj = new Date(evento.fecha);
      return {
        id: `${i + 1}`,
        fecha: fechaObj.toLocaleDateString("es-AR", {
          weekday: "short",
          day: "2-digit",
          month: "short"
        }),
        hora: fechaObj.toLocaleTimeString("es-AR", {
          hour: "2-digit",
          minute: "2-digit",
          timeZone: "America/Argentina/Buenos_Aires"
        }),
        cantidad: "20g", // Esto podrías hacerlo dinámico si en el futuro lo guardás también
        accion: evento.accion === "encendido"
          ? "Comedero activado"
          : "Comedero apagado"
      };
    });
  

    res.status(200).json(historial.reverse()); // más reciente primero
  } catch (err) {
    console.error("Error al obtener historial:", err);
    res.status(500).json({ message: "Error interno", error: err.message });
  }
};

// Limpiar historial de un feeder
const clearFeederHistory = async (req, res) => {
  try {
    const { feederId } = req.params;
    const userId = req.user._id;

    // 1. Buscar el comedero por ID
    const feeder = await Feeder.findOne({ feederId });

    if (!feeder) {
      return res.status(404).json({ message: "Comedero no encontrado." });
    }

    // 2. Verificar si el comedero tiene asignado un usuario
    if (!feeder.userId) {
      return res.status(403).json({ message: "Este comedero no está asignado a ningún usuario." });
    }

    // 3. Verificar si le pertenece al usuario logueado
    if (feeder.userId.toString() !== userId.toString()) {
      return res.status(403).json({ message: "No tienes permisos para modificar este comedero." });
    }

    // 4. Borrar el historial
    feeder.feederHistory = [];
    await feeder.save();

    res.status(200).json({ message: "Historial eliminado correctamente." });
  } catch (error) {
    console.error("Error al borrar historial:", error);
    res.status(500).json({ message: "Error interno al borrar historial." });
  }
};

const heartbeat = async (req, res) => {
  try {
      const feeder = await Feeder.findOneAndUpdate(
        { feederId: req.device.feederId },
        {
          $set: {
            lastConection: Date.now()
          }
        },
        { new: true }
      );
  
      if (!feeder) {
        return res.status(404).json({
          error: "Feeder no encontrado"
        });
      }
  
      return res.status(200).json({
        message: "Heartbeat recibido"
      });
    } catch (error) {
      console.error("Error al procesar heartbeat del dispositivo:", error);
  
      return res.status(500).json({
        error: "Error interno del servidor"
      });
    }
};



module.exports = {
  registerFeeder,
  getAllFeeders,
  getMyFeeders,
  getFeederById,
  getGlobalFeederById,
  assignFeeder,
  unassignFeeder,
  deleteFeeder,
  addStartHours,
  startMotor,
  editFeeder,
  getMotorStatus,
  getMotorStatusNodemcu,
  completeMotorCommand,
  stopMotorFromNodemcu,
  getFechasByFeederId,
  getFeederHistory,
  syncFeedingHistory,
  heartbeat,
  getRemoteConfiguration,
  updateFeederConfiguration,
  enrollDevice,
};

