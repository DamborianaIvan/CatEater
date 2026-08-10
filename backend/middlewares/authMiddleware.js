// middlewares/authMiddleware.js
const jwt = require("jsonwebtoken");

const verifyToken = (req, res, next) => {
  // Extraer token (suponiendo que usas el formato "Bearer token")
  const authHeader = req.header("Authorization");
  if (!authHeader) return res.status(401).json({ message: "Acceso denegado - token faltante" });
  
  // Si el header viene en formato "Bearer <token>", extraemos el token:
  const token = authHeader.split(" ")[1] || authHeader;
  
  try {
    const verified = jwt.verify(token, process.env.JWT_SECRET);
    req.user = verified;
    next();
  } catch (err) {
    return res.status(400).json({ message: "Token inválido" });
  }
};

module.exports = verifyToken;
