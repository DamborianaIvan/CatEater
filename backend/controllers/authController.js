const User = require("../models/User");
const bcrypt = require("bcryptjs");
const jwt = require("jsonwebtoken");
const validator = require("validator");
const axios = require("axios");
const crypto = require("crypto");
require('dotenv').config();

const sendResetEmail = async ({ email, name, resetLink }) => {
  const serviceId = process.env.EMAILJS_SERVICE_ID;
  const templateId = process.env.EMAILJS_TEMPLATE_ID;
  const userId = process.env.EMAILJS_USER_ID;

  const templateParams = {
    user_email: email,
    user_name: name || "usuario",
    reset_link: resetLink,
  };

  const data = {
    service_id: serviceId,
    template_id: templateId,
    user_id: userId, // IMPORTANTE: clave pública
    template_params: templateParams,
  };
  console.log({
    serviceId,
    templateId,
    userId,
    email,
    name,
    resetLink,
  });

  try {
    const response = await axios.post("https://api.emailjs.com/api/v1.0/email/send", data, {
      headers: {
        "Content-Type": "application/json",
      },
    });

    return response.data;
  } catch (err) {
    console.error("Detalles del error EmailJS:", err.response?.data || err.message);
    throw new Error("Error al enviar el email con EmailJS");
  }
};

exports.register = async (req, res) => {
  const { email, password, name, surname, emailReceiver } = req.body;

  if (!email || !password || !name || !surname) {
    return res.status(400).json({ message: "Todos los campos son obligatorios" });
  }

  if (!validator.isEmail(email)) {
    return res.status(400).json({ message: "Email inválido" });
  }

  const existingUser = await User.findOne({ email });
  if (existingUser) {
    return res.status(400).json({ message: "El email ya está registrado" });
  }

  if (password.length < 8 || !/[A-Z]/.test(password)) {
    return res.status(400).json({ message: "La contraseña no cumple con los requisitos." });
  }

  const hashedPassword = await bcrypt.hash(password, 10);

  try {
    const newUser = new User({ email, password: hashedPassword, name, surname, emailReceiver });
    await newUser.save();
    res.status(201).json({ message: "Usuario registrado exitosamente" });
  } catch (error) {
    res.status(400).json({ message: "Error al registrar usuario", error });
  }
};

exports.login = async (req, res) => {
  const { email, password } = req.body;

  if (!validator.isEmail(email)) {
    return res.status(400).json({ message: "Email inválido" });
  }

  const user = await User.findOne({ email });
  if (!user) return res.status(400).json({ message: "Usuario no encontrado" });

  const validPassword = await bcrypt.compare(password, user.password);
  if (!validPassword) return res.status(400).json({ message: "Contraseña incorrecta" });

  const token = jwt.sign({ _id: user._id }, process.env.JWT_SECRET, { expiresIn: "1h" });
  res.json({ token });
};

exports.forgotPassword = async (req, res) => {
  const { email } = req.body;

  if (!email) return res.status(400).json({ message: "Email requerido" });

  const user = await User.findOne({ email });
  if (!user) return res.status(404).json({ message: "Usuario no encontrado" });

  const token = crypto.randomBytes(32).toString("hex");
  const expiration = Date.now() + 3600000; // 1 hora

  user.resetPasswordToken = token;
  user.resetPasswordExpires = expiration;
  await user.save();

  const resetLink = `${process.env.API_URL}/${token}`;

  try {
    await sendResetEmail({
      email: user.email,
      name: user.name,
      resetLink,
    });

    res.json({ message: "Email enviado con el enlace para recuperar contraseña" });
  } catch (error) {
    console.error("Error al enviar email:", error.message);
    res.status(500).json({ message: "Error al enviar el email" });
  }
};

exports.resetPassword = async (req, res) => {
  const { token, password } = req.body;

  if (!token || !password) {
    return res.status(400).json({ message: "Token y nueva contraseña requeridos" });
  }

  const user = await User.findOne({
    resetPasswordToken: token,
    resetPasswordExpires: { $gt: Date.now() },
  });

  if (!user) {
    return res.status(400).json({ message: "Token inválido o expirado" });
  }

  const hashedPassword = await bcrypt.hash(password, 10);
  user.password = hashedPassword;
  user.resetPasswordToken = undefined;
  user.resetPasswordExpires = undefined;

  await user.save();
  res.json({ message: "Contraseña restablecida exitosamente" });
};