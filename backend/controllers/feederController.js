const Feeder = require("../models/Feeder");
const crypto = require("crypto");
require('../jobs/motorStatusJob');


// Registrar comedero (desde NodeMCU)
const registerFeeder = async (req, res) => {
  const apiKey = req.headers['x-api-key'];
  if (apiKey !== process.env.NODEMCU_API_KEY) {
    return res.status(401).json({ error: 'No autorizado - API Key inválida' });
  }

  const feederInfo = req.body;
  if (!feederInfo || Object.keys(feederInfo).length === 0) {
    return res.status(400).json({ error: 'Datos faltantes' });
  }

  try {
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
  const apiKey = req.headers['x-api-key'];
  const feederId = req.params.feederId;

  if (!apiKey) return res.status(401).json({ message: "Falta la API Key" });
  if (apiKey !== process.env.NODEMCU_API_KEY) return res.status(403).json({ message: "API Key inválida" });
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

//apagar motor
const completeMotorCommand = async (req, res) => {
  const apiKey = req.headers["x-api-key"];

  if (apiKey !== process.env.NODEMCU_API_KEY) {
    return res.status(401).json({
      error: "No autorizado - API Key inválida"
    });
  }

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
        return res.status(400).json({
            error: "No hay una orden de alimentación pendiente"
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

//apagar motor
const addHistoryFromNodemcu = async (req, res)=>{
  const {feederId} = req.body;
  const apiKey = req.headers['x-api-key'];
  
  if (!apiKey) return res.status(401).json({ message: "Falta la API Key" });
  if (apiKey !== process.env.NODEMCU_API_KEY) return res.status(403).json({ message: "API Key inválida" });

  try{
    // Buscar el feeder
    const existingFeeder = await Feeder.findOne({ feederId });

    if (!existingFeeder) {
      return res.status(404).json({ message: "Feeder no encontrado" });
    }

    const feeder = await Feeder.findOneAndUpdate(
      { feederId },
      {
        $push: {
          feederHistory: {
            fecha: Date.now(),
            accion: 'encendido'
          }
        }
      },
      { new: true }
    );
    console.log(`Motor encendido automáticamente para feeder ${feederId}`);
    // Configurar apagado automático en 10 segundos
     setTimeout(async () => {
      try {
        await Feeder.findOneAndUpdate(
          { feederId },
          {
            $push: {
              feederHistory: {
                fecha: Date.now(),
                accion: 'apagado'
              }
            }
          }
        );
        console.log(`Motor apagado automáticamente para feeder ${feederId}`);
      } catch (err) {
        console.error(`Error al apagar motor automáticamente: ${err.message}`);
      }
    }, 40000); // 40 segundos en milisegundos

    return res.status(200).json({
      message: "Feeder encendedido con éxito"
    });
  }catch(err){
      console.error("Error al encender feeder:", err);
      res.status(500).json({ message: "Error interno al desasignar el feeder", error: err.message });
  }
}

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
  const apiKey = req.headers['x-api-key'];
  const { feederId } = req.params;

  if (!apiKey) return res.status(401).json({ message: "Falta la API Key" });
  if (apiKey !== process.env.NODEMCU_API_KEY) return res.status(403).json({ message: "API Key inválida" });
  // Obtener un comedero por feederId y userId

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

    res.json({motorState:feeder.motorInfo.motorState,portions:feeder.motorInfo.portions,commandId:feeder.motorInfo.commandId});
  } catch (error) {
    res.status(500).json({ message: "Error al obtener comedero", error: error.message });
  }

}

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
  addHistoryFromNodemcu
};