const Feeder = require("../models/Feeder");
const {
  generateDeviceCredential,
  hashDeviceCredential
} = require("../utils/deviceCredential");
require('../jobs/motorStatusJob');

const MAX_SCHEDULES = 5;
const MIN_STEPS_PER_FEED = 1;
const MAX_STEPS_PER_FEED = 10240;
const MIN_PORTIONS = 1;
const MAX_PORTIONS = 5;

const createDefaultSchedules = () => Array.from({ length: MAX_SCHEDULES }, () => ({ hour: 0, minute: 0, portions: 1, enabled: false }));

const validateConfiguration = (configuration) => {
  if (!configuration || typeof configuration !== "object") return "La configuración es obligatoria.";
  const { stepsPerFeed, schedules } = configuration;
  if (!Number.isInteger(stepsPerFeed) || stepsPerFeed < MIN_STEPS_PER_FEED || stepsPerFeed > MAX_STEPS_PER_FEED) return `stepsPerFeed debe estar entre ${MIN_STEPS_PER_FEED} y ${MAX_STEPS_PER_FEED}.`;
  if (!Array.isArray(schedules)) return "schedules debe ser un arreglo.";
  if (schedules.length !== MAX_SCHEDULES) return `schedules debe contener exactamente ${MAX_SCHEDULES} elementos.`;
  for (const schedule of schedules) {
    if (!Number.isInteger(schedule.hour) || schedule.hour < 0 || schedule.hour > 23) return "Hora de schedule inválida.";
    if (!Number.isInteger(schedule.minute) || schedule.minute < 0 || schedule.minute > 59) return "Minuto de schedule inválido.";
    if (!Number.isInteger(schedule.portions) || schedule.portions < MIN_PORTIONS || schedule.portions > MAX_PORTIONS) return "Cantidad de porciones inválida.";
    if (typeof schedule.enabled !== "boolean") return "enabled debe ser boolean.";
  }
  return null;
};

const getFeederConfiguration = async (req, res) => {
  const apiKey = req.headers["x-api-key"];
  const { feederId } = req.params;
  if (apiKey !== process.env.NODEMCU_API_KEY) return res.status(401).json({ error: "No autorizado - API Key inválida" });
  if (!feederId || feederId.trim() === "") return res.status(400).json({ error: "feederId es requerido" });
  try {
    const feeder = await Feeder.findOne({ feederId });
    if (!feeder) return res.status(404).json({ error: "Feeder no encontrado" });
    const configuration = feeder.configuration || { revision: 1, stepsPerFeed: 2048, schedules: createDefaultSchedules() };
    const schedules = configuration.schedules.map((schedule) => ({ hour: schedule.hour, minute: schedule.minute, portions: schedule.portions, enabled: schedule.enabled }));
    return res.status(200).json({ revision: configuration.revision, stepsPerFeed: configuration.stepsPerFeed, schedules });
  } catch (error) {
    console.error("Error al obtener configuración del feeder:", error);
    return res.status(500).json({ error: "Error interno del servidor" });
  }
};

// Registrar/enrolar comedero. La API Key global se utiliza únicamente como bootstrap.
const registerFeeder = async (req, res) => {
  const apiKey = req.headers['x-api-key'];
  if (apiKey !== process.env.NODEMCU_API_KEY) return res.status(401).json({ error: 'No autorizado - API Key inválida' });
  const feederInfo = req.body;
  if (!feederInfo || Object.keys(feederInfo).length === 0) return res.status(400).json({ error: 'Datos faltantes' });

  try {
    const existingFeeder = await Feeder.findOne({ feederId: feederInfo.feederId }).select("+deviceCredentialHash");

    if (existingFeeder) {
      if (existingFeeder.deviceCredentialHash) {
        return res.status(409).json({ error: 'Ya existe un dispositivo con ese feederId' });
      }

      const deviceCredential = generateDeviceCredential();
      existingFeeder.deviceCredentialHash = hashDeviceCredential(deviceCredential);
      await existingFeeder.save();

      return res.status(200).json({
        message: 'Dispositivo enrolado correctamente',
        deviceCredential
      });
    }

    const deviceCredential = generateDeviceCredential();
    const newFeeder = new Feeder({
      feederId: feederInfo.feederId,
      feederName: feederInfo.feederName,
      deviceCredentialHash: hashDeviceCredential(deviceCredential)
    });
    await newFeeder.save();

    return res.status(201).json({ message: 'Comedero guardado correctamente', deviceCredential });
  } catch (error) {
    console.error('Error al registrar/enrolar comedero:', error);
    return res.status(500).json({ error: 'Hubo un error con el servidor' });
  }
};

const getAllFeeders = async (req, res) => {
  try { res.json(await Feeder.find()); }
  catch (error) { res.status(400).json({ message: "Error al obtener los comederos", error }); }
};

const getMyFeeders = async (req, res) => {
  if (!req.user || !req.user._id) return res.status(401).json({ message: "Acceso no autorizado - usuario no autenticado." });
  try {
    const feeders = await Feeder.find({ userId: req.user._id });
    if (!feeders || feeders.length === 0) return res.status(200).json({ message: "No se encontraron comederos para este usuario.", feeders:[] });
    res.json(feeders);
  } catch (error) { res.status(500).json({ message: "Error al obtener los comederos", error: error.message }); }
};

const getFeederById = async (req, res) => {
  const { feederId } = req.params;
  if (!feederId || feederId.trim() === "") return res.status(400).json({ message: "El parámetro feederId es obligatorio y no puede estar vacío." });
  if (!req.user || !req.user._id) return res.status(401).json({ message: "Acceso no autorizado - usuario no identificado." });
  try {
    const feeder = await Feeder.findOne({ feederId, userId: req.user._id });
    if (!feeder) return res.status(404).json({ message: "Comedero no encontrado para el usuario especificado." });
    res.json(feeder);
  } catch (error) { res.status(500).json({ message: "Error al obtener comedero", error: error.message }); }
};

const getGlobalFeederById = async (req, res) => {
  const apiKey = req.headers['x-api-key'];
  const feederId = req.params.feederId;
  if (!apiKey) return res.status(401).json({ message: "Falta la API Key" });
  if (apiKey !== process.env.NODEMCU_API_KEY) return res.status(403).json({ message: "API Key inválida" });
  if (!feederId || feederId.trim() === "") return res.status(400).json({ message: "El campo 'feederId' no puede estar vacío" });
  try {
    const feeder = await Feeder.findOne({ feederId });
    if (!feeder) return res.status(404).json({ message: "No se encontró un comedero con esa ID" });
    return res.status(200).json(feeder);
  } catch (error) { return res.status(500).json({ message: "Error al obtener comedero", error: error.message }); }
};

const enrollDevice = async (req, res) => {
  const apiKey = req.headers["x-api-key"];
  const { feederId } = req.body || {};
  if (apiKey !== process.env.NODEMCU_API_KEY) return res.status(401).json({ error: "No autorizado - API Key inválida" });
  if (!feederId || typeof feederId !== "string" || feederId.trim() === "") return res.status(400).json({ error: "feederId es requerido" });
  try {
    const feeder = await Feeder.findOne({ feederId }).select("+deviceCredentialHash");
    if (!feeder) return res.status(404).json({ error: "Feeder no encontrado" });
    if (feeder.deviceCredentialHash) return res.status(409).json({ error: "El dispositivo ya está enrolado" });
    const deviceCredential = generateDeviceCredential();
    feeder.deviceCredentialHash = hashDeviceCredential(deviceCredential);
    await feeder.save();
    return res.status(201).json({ message: "Dispositivo enrolado correctamente", deviceCredential });
  } catch (error) {
    console.error("Error al enrolar dispositivo:", error);
    return res.status(500).json({ error: "Error interno del servidor" });
  }
};

module.exports = { validateConfiguration, getFeederConfiguration, registerFeeder, enrollDevice, getAllFeeders, getMyFeeders, getFeederById, getGlobalFeederById };
