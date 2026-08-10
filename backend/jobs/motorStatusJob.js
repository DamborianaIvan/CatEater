const Feeder = require('../models/Feeder');
const { DateTime } = require("luxon");

const revisarHoras = async () => {
  try {
    

    const ahora = DateTime.now().setZone("America/Argentina/Buenos_Aires");
    const minutosAhora = ahora.hour * 60 + ahora.minute;
    const segundosAhora = ahora.second;
    const margenSegundos = 10;

    const feeders = await Feeder.find({
      'motorInfo.motorState': false,
      'motorInfo.startHours': { $exists: true, $ne: [] }
    });

    const feedersParaActivar = [];

    for (const feeder of feeders) {
      const ejecucionesHoy = feeder.feederHistory.filter(ej =>
        new Date(ej).toDateString() === ahora.toFormat("yyyy-MM-dd")
      );

      const yaEjecutadoHoy = (hora) => {
        const horaObj = new Date(hora);
        return ejecucionesHoy.some(ej =>
          ej.getHours() === horaObj.getHours() &&
          ej.getMinutes() === horaObj.getMinutes()
        );
      };

      const debeActivarse = feeder.motorInfo.startHours.some(hora => {
        if (!hora || yaEjecutadoHoy(hora)) return false;

        const horaDate = new Date(hora);
        const minutosFeeder = horaDate.getHours() * 60 + horaDate.getMinutes();
        const segundosFeeder = horaDate.getSeconds();

        const diferenciaSegundos = Math.abs(
          (minutosAhora * 60 + segundosAhora) -
          (minutosFeeder * 60 + segundosFeeder)
        );

        return diferenciaSegundos <= margenSegundos;
      });

      if (debeActivarse) feedersParaActivar.push(feeder);
    }

    for (const feeder of feedersParaActivar) {
      feeder.motorInfo.motorState = true;
      feeder.feederHistory.push(ahora);
      await feeder.save();
      console.log(`✅ Motor ENCENDIDO para: ${feeder.feederName}`);

      setTimeout(async () => {
        try {
          const feederToUpdate = await Feeder.findById(feeder._id);
          feederToUpdate.motorInfo.motorState = false;
          await feederToUpdate.save();
          console.log(`🛑 Motor APAGADO automáticamente para: ${feeder.feederName}`);
        } catch (error) {
          console.error('Error apagando motor automáticamente:', error);
        }
      }, 15 * 1000);
    }
  } catch (error) {
    console.error('Error revisando estado del motor:', error);
  }
};

setInterval(revisarHoras, 5 * 1000);
