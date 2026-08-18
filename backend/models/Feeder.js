const mongoose = require("mongoose");

const FeederSchema = new mongoose.Schema({
  feederId: { type: String, required: true, unique: true },
  userId: { type: String, default: null },
  feederName: { type: String, default: ' ' },
  feederLogo: { type: String, default: ' ' },
  feederAsign: { type: Boolean, default: false },
  feederQuantity: { type: Number, default: 0 },
  lastConection: { type: Date, default: Date.now },
  motorInfo: {
    startHours: {
      type: [Date]
    },
    motorState: {
      type: Boolean,
      default: false
    },
    portions: {
      type: Number,
      default: 1
    },
    commandId: {
      type: String,
      default: null
    }
  },
  configuration: {
    revision: {
      type: Number,
      default: 1,
      min: 1
    },
    stepsPerFeed: {
      type: Number,
      default: 2048,
      min: 1,
      max: 10240
    },
    schedules: {
      type: [
        {
          hour: {
            type: Number,
            required: true,
            min: 0,
            max: 23
          },
          minute: {
            type: Number,
            required: true,
            min: 0,
            max: 59
          },
          portions: {
            type: Number,
            required: true,
            min: 1,
            max: 5
          },
          enabled: {
            type: Boolean,
            default: false
          }
        }
      ],
      default: () => Array.from({ length: 5 }, () => ({
        hour: 0,
        minute: 0,
        portions: 1,
        enabled: false
      }))
    }
  },
  feederHistory: {
    type: [
      {
        eventId: {
          type: String,
          default: null
        },
        fecha: {
          type: Date,
          default: Date.now
        },
        portions: {
          type: Number,
          default: 1
        },
        source: {
          type: String,
          enum: ["physical", "scheduled", "remote", "legacy"],
          default: "legacy"
        }
      }
    ],
    default: []
  }
});

// Los eventos de alimentación deben originarse en FeedingService y sincronizarse
// mediante eventId. Evita que una confirmación de commandId genere un segundo
// evento anónimo en el historial.
FeederSchema.pre("updateOne", function () {
  const update = this.getUpdate();
  const historyPush = update?.$push?.feederHistory;

  if (historyPush && !historyPush.eventId) {
    delete update.$push.feederHistory;

    if (Object.keys(update.$push).length === 0) {
      delete update.$push;
    }
  }
});

module.exports = mongoose.model("Feeder", FeederSchema);
