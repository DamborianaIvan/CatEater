// middlewares/authMiddleware.js
const jwt = require("jsonwebtoken");
const Feeder = require("../models/Feeder");

const requiresFeederOwnership = (req) => {
  return (
    (req.method === "POST" && req.path === "/feeder/start") ||
    (req.method === "PUT" && /^\/feeders\/[^/]+\/config$/.test(req.path))
  );
};

const getFeederId = (req) => {
  if (req.method === "POST" && req.path === "/feeder/start") {
    return req.body?.feederId;
  }

  return req.params?.feederId;
};

const verifyFeederOwnership = async (req, res) => {
  const feederId = getFeederId(req);

  if (!feederId || typeof feederId !== "string" || feederId.trim() === "") {
    return {
      allowed: false,
      response: res.status(400).json({
        message: "El campo 'feederId' es requerido y debe ser un string válido."
      })
    };
  }

  const feeder = await Feeder.findOne({ feederId });

  if (!feeder) {
    return {
      allowed: false,
      response: res.status(404).json({
        message: "Comedero no encontrado."
      })
    };
  }

  if (!feeder.userId) {
    return {
      allowed: false,
      response: res.status(403).json({
        message: "Este comedero no está asignado a ningún usuario."
      })
    };
  }

  if (feeder.userId.toString() !== req.user._id.toString()) {
    return {
      allowed: false,
      response: res.status(403).json({
        message: "Este comedero no te pertenece."
      })
    };
  }

  return { allowed: true };
};

const verifyToken = async (req, res, next) => {
  const authHeader = req.header("Authorization");

  if (!authHeader) {
    return res.status(401).json({
      message: "Acceso denegado - token faltante"
    });
  }

  const token = authHeader.split(" ")[1] || authHeader;

  try {
    const verified = jwt.verify(token, process.env.JWT_SECRET);
    req.user = verified;

    if (requiresFeederOwnership(req)) {
      const ownership = await verifyFeederOwnership(req, res);

      if (!ownership.allowed) {
        return;
      }
    }

    next();
  } catch (err) {
    console.error("Error en autenticación:", err);

    if (err.name === "JsonWebTokenError" || err.name === "TokenExpiredError") {
      return res.status(400).json({ message: "Token inválido" });
    }

    return res.status(500).json({
      message: "Error interno al validar la autenticación"
    });
  }
};

module.exports = verifyToken;
