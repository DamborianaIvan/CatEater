const mongoose = require("mongoose");

const DeviceProvisioningSchema = new mongoose.Schema(
  {
    feederId: {
      type: String,
      required: true,
      unique: true,
      index: true,
      trim: true
    },
    bootstrapSecretHash: {
      type: String,
      required: function () {
        return this.status === "UNPROVISIONED";
      },
      default: null,
      select: false
    },
    status: {
      type: String,
      enum: ["UNPROVISIONED", "ENROLLED"],
      default: "UNPROVISIONED",
      required: true
    },
    enrolledAt: {
      type: Date,
      default: null
    }
  },
  {
    timestamps: true
  }
);

module.exports = mongoose.model("DeviceProvisioning", DeviceProvisioningSchema);
