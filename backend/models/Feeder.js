const mongoose = require("mongoose");
const { getFeederHistory } = require("../controllers/feederController");

const FeederSchema = new mongoose.Schema({
  feederId: { type: String, required: true, unique: true },
  userId: { type: String, default: null }, // Si querés que sea referencia a User, avisame
  feederName: { type: String, default: ' ' },
  feederLogo: { type: String, default: ' ' },
  feederAsign: { type: Boolean, default: false },
  feederQuantity: { type: Number, default: 0 },
  lastConection: { type: Date, default: Date.now },
  motorInfo: {
    startHours: {
      type: [Date]
    },
    motorState: { type: Boolean, default: false },
    portions: {
        type: Number,
        default: 1
    }
  },
  feederHistory: {
    type: [
      {
        fecha: { type: Date, default: Date.now },
        accion: { type: String, enum: ['encendido', 'apagado'], required: true }
      }
    ],
    default: []
  }
});

module.exports = mongoose.model("Feeder", FeederSchema);
