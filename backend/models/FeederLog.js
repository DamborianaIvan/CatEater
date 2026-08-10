const mongoose = require("mongoose");

const FeederLogSchema = new mongoose.Schema({
  feederId: { type: String, required: true },
  timestamp: { type: Date, default: Date.now },
  amount: { type: Number, required: true }
});

module.exports = mongoose.model("FeederLog", FeederLogSchema);
