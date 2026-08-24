const authenticateDeviceBootstrap = async (req, res) => {
  return res.status(410).json({
    error: "El provisioning por bootstrap fue eliminado. El dispositivo debe ser creado y provisionado en fábrica."
  });
};

module.exports = authenticateDeviceBootstrap;
