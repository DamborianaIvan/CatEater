require("dotenv").config();
const express = require("express");
const cors = require("cors");
const connectDB = require("./configs/db");
const swaggerJsDoc = require("swagger-jsdoc");
const swaggerUi = require("swagger-ui-express");

const authRoutes = require("./routes/authRoutes");
const feederRoutes = require("./routes/feederRoutes");
const deviceFactoryRoutes = require("./routes/deviceFactoryRoutes");
const devicePairingRoutes = require("./routes/devicePairingRoutes");

const app = express();
connectDB();

app.use(cors());
app.use(express.json());

app.use("/", authRoutes);
app.use("/", feederRoutes);
app.use("/", deviceFactoryRoutes);
app.use("/", devicePairingRoutes);

const swaggerOptions = {
  swaggerDefinition: {
    openapi: "3.0.0",
    info: {
      title: "CatFeeder API",
      version: "1.2.0",
      description:
        "API REST del sistema CatFeeder. Documentación generada a partir de las rutas y controladores vigentes del backend.",
    },
    servers: [
      {
        url: "{serverUrl}",
        description: "Servidor CatFeeder",
        variables: {
          serverUrl: {
            default: "http://localhost:5000",
            description:
              "URL base del backend. En producción reemplazar por la URL pública de Render.",
          },
        },
      },
    ],
    tags: [
      { name: "Auth", description: "Autenticación y recuperación de cuentas." },
      { name: "Feeders", description: "Gestión de comederos y comunicación con dispositivos." },
      { name: "Device Factory", description: "Alta administrativa de dispositivos." },
      { name: "Device Pairing", description: "Vinculación y desvinculación de dispositivos." },
    ],
  },
  apis: ["./routes/*.js"],
};

const swaggerDocs = swaggerJsDoc(swaggerOptions);
app.use("/api-docs", swaggerUi.serve, swaggerUi.setup(swaggerDocs));

const PORT = process.env.PORT || 5000;
app.listen(PORT, () => console.log(`Servidor corriendo en el puerto ${PORT}`));
