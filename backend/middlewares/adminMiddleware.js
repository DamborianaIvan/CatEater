const requireAdmin = (req, res, next) => {
  if (!req.user || req.user.role !== "admin") {
    return res.status(403).json({
      message: "Acceso restringido a administradores"
    });
  }

  next();
};

module.exports = requireAdmin;
