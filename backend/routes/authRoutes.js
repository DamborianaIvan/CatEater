const express = require("express");
const router = express.Router();
const authController = require("../controllers/authController");
const { forgotPassword, resetPassword } = require("../controllers/authController");
/**
 * @swagger
 * tags:
 *   name: Auth
 *   description: Endpoints sobre la autenticación de usuarios
 */

/**
 * @swagger
 * /register:
 *   post:
 *     summary: Registra un nuevo usuario
 *     tags:
 *       - Auth
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             required:
 *               - email
 *               - password
 *               - name
 *               - surname
 *             properties:
 *               email:
 *                 type: string
 *                 example: usuario@example.com
 *               password:
 *                 type: string
 *                 example: Contraseña123
 *               name:
 *                 type: string
 *                 example: Juan
 *               surname:
 *                 type: string
 *                 example: Pérez
 *               emailReceiver:
 *                 type: boolean
 *                 example: false
 *     responses:
 *       201:
 *         description: Usuario registrado exitosamente
 *         content:
 *           application/json:
 *             example:
 *               message: Usuario registrado exitosamente
 *       
 *       400:
 *         description: Error en los datos enviados
 *         content:
 *           application/json:
 *             examples:
 *               CamposObligatoriosFaltantes:
 *                 summary: Faltan campos obligatorios
 *                 value:
 *                   message: "Todos los campos son obligatorios"
 *               EmailInvalido:
 *                 summary: Email con formato incorrecto
 *                 value:
 *                   message: "Email inválido"
 *               EmailYaRegistrado:
 *                 summary: Email ya existe en la base de datos
 *                 value:
 *                   message: "El email ya está registrado"
 *               ContraseñaInvalida:
 *                 summary: Contraseña no cumple con requisitos mínimos
 *                 value:
 *                   message: "La contraseña no cumple con los requisitos."
 *               ErrorRegistro:
 *                 summary: Error inesperado al registrar
 *                 value:
 *                   message: "Error al registrar usuario"
 *                   error: {}
 */
router.post("/register", authController.register);

/**
 * @swagger
 * /login:
 *   post:
 *     summary: Inicia sesión de un usuario registrado
 *     tags:
 *       - Auth
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             required:
 *               - email
 *               - password
 *             properties:
 *               email:
 *                 type: string
 *                 example: usuario@example.com
 *               password:
 *                 type: string
 *                 example: Contraseña123
 *     responses:
 *       200:
 *         description: Inicio de sesión exitoso
 *         content:
 *           application/json:
 *             example:
 *               token: eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...
 *
 *       400:
 *         description: Error en las credenciales o validación
 *         content:
 *           application/json:
 *             examples:
 *               EmailInvalido:
 *                 summary: Email con formato incorrecto
 *                 value:
 *                   message: "Email inválido"
 *               UsuarioNoEncontrado:
 *                 summary: No existe el usuario con ese email
 *                 value:
 *                   message: "Usuario no encontrado"
 *               ContraseñaIncorrecta:
 *                 summary: Contraseña no coincide con la guardada
 *                 value:
 *                   message: "Contraseña incorrecta"
 */
router.post("/login", authController.login);

// Ruta para solicitar el email con el link de recuperación
router.post("/forgot-password", authController.forgotPassword);

// Ruta para restablecer la contraseña con el token
router.post("/reset-password", authController.resetPassword);

module.exports = router;
