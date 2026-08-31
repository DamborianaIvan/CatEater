const express = require("express");
const router = express.Router();
const authController = require("../controllers/authController");

/**
 * @swagger
 * tags:
 *   - name: Auth
 *     description: Autenticación y recuperación de cuentas de usuario.
 *
 * components:
 *   securitySchemes:
 *     bearerAuth:
 *       type: http
 *       scheme: bearer
 *       bearerFormat: JWT
 *       description: JWT obtenido mediante POST /login. Expira una hora después de su emisión.
 *     deviceCredential:
 *       type: apiKey
 *       in: header
 *       name: x-device-credential
 *       description: Credencial individual del dispositivo, generada durante el alta de fábrica.
 *   schemas:
 *     Error:
 *       type: object
 *       properties:
 *         message:
 *           type: string
 *         error:
 *           type: string
 *     ErrorDevice:
 *       type: object
 *       properties:
 *         error:
 *           type: string
 *     RegisterRequest:
 *       type: object
 *       required: [email, password, name, surname]
 *       properties:
 *         email:
 *           type: string
 *           format: email
 *           example: usuario@example.com
 *         password:
 *           type: string
 *           format: password
 *           minLength: 8
 *           description: Debe contener al menos 8 caracteres y una letra mayúscula.
 *           example: Contraseña123
 *         name:
 *           type: string
 *           example: Juan
 *         surname:
 *           type: string
 *           example: Pérez
 *         emailReceiver:
 *           type: boolean
 *           description: Preferencia de recepción de emails.
 *           example: false
 *     LoginRequest:
 *       type: object
 *       required: [email, password]
 *       properties:
 *         email:
 *           type: string
 *           format: email
 *           example: usuario@example.com
 *         password:
 *           type: string
 *           format: password
 *           example: Contraseña123
 *     LoginResponse:
 *       type: object
 *       required: [token]
 *       properties:
 *         token:
 *           type: string
 *           description: JWT de autenticación con una vigencia de una hora.
 *     ForgotPasswordRequest:
 *       type: object
 *       required: [email]
 *       properties:
 *         email:
 *           type: string
 *           format: email
 *           example: usuario@example.com
 *     ResetPasswordRequest:
 *       type: object
 *       required: [token, password]
 *       properties:
 *         token:
 *           type: string
 *           description: Token recibido en el enlace de recuperación.
 *         password:
 *           type: string
 *           format: password
 *           description: Nueva contraseña.
 *           example: NuevaClave123
 */

/**
 * @swagger
 * /register:
 *   post:
 *     summary: Registrar usuario
 *     tags: [Auth]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             $ref: '#/components/schemas/RegisterRequest'
 *     responses:
 *       201:
 *         description: Usuario registrado exitosamente.
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *               properties:
 *                 message:
 *                   type: string
 *                   example: Usuario registrado exitosamente
 *       400:
 *         description: Datos inválidos, email ya registrado o contraseña que no cumple los requisitos.
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Error'
 */
router.post("/register", authController.register);

/**
 * @swagger
 * /login:
 *   post:
 *     summary: Iniciar sesión
 *     tags: [Auth]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             $ref: '#/components/schemas/LoginRequest'
 *     responses:
 *       200:
 *         description: Inicio de sesión exitoso.
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/LoginResponse'
 *       400:
 *         description: Email inválido, usuario inexistente o contraseña incorrecta.
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Error'
 */
router.post("/login", authController.login);

/**
 * @swagger
 * /forgot-password:
 *   post:
 *     summary: Solicitar recuperación de contraseña
 *     description: Genera un token de recuperación con vigencia de una hora y solicita el envío del enlace por EmailJS.
 *     tags: [Auth]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             $ref: '#/components/schemas/ForgotPasswordRequest'
 *     responses:
 *       200:
 *         description: Solicitud procesada y email enviado.
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *               properties:
 *                 message:
 *                   type: string
 *                   example: Email enviado con el enlace para recuperar contraseña
 *       400:
 *         description: Falta el email.
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Error'
 *       404:
 *         description: Usuario no encontrado.
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Error'
 *       500:
 *         description: No se pudo enviar el email.
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Error'
 */
router.post("/forgot-password", authController.forgotPassword);

/**
 * @swagger
 * /reset-password:
 *   post:
 *     summary: Restablecer contraseña
 *     tags: [Auth]
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             $ref: '#/components/schemas/ResetPasswordRequest'
 *     responses:
 *       200:
 *         description: Contraseña restablecida exitosamente.
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *               properties:
 *                 message:
 *                   type: string
 *                   example: Contraseña restablecida exitosamente
 *       400:
 *         description: Token o contraseña faltantes, o token inválido/expirado.
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/Error'
 */
router.post("/reset-password", authController.resetPassword);

module.exports = router;
