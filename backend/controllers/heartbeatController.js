const Feeder = require("../models/Feeder");

const sendHeartbeat = async (req, res) => {
  const feederId = req.device?.feederId;

  if (!feederId) {
    return res.status(400).json({
      message: "No se pudo identificar el dispositivo."
    });
  }

  try {
    const feeder = await Feeder.findOneAndUpdate(
      { feederId },
      { $set: { lastConection: new Date() } },
      { new: true, select: "feederId lastConection" }
    );

    if (!feeder) {
      return res.status(404).json({
        message: "Comedero no encontrado."
      });
    }

    return res.status(200).json({
      message: "Heartbeat recibido correctamente.",
      feederId: feeder.feederId,
      lastConection: feeder.lastConection
    });
  } catch (error) {
    console.error("Error al procesar heartbeat:", error);
    return res.status(500).json({
      message: "Error interno del servidor."
    });
  }
};

module.exports = {
  sendHeartbeat
};
