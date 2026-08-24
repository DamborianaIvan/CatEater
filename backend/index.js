require("dotenv").config();
const express = require("express");
const cors = require("cors");
const connectDB = require("./configs/db");
const swaggerJsDoc = require('swagger-jsdoc');
const swaggerUi = require('swagger-ui-express');

const authRoutes = require("./routes/authRoutes");
const feederRoutes = require("./routes/feederRoutes");
const deviceFactoryRoutes = require("./routes/deviceFactoryRoutes");

const app = express();
connectDB();

app.use(cors());
app.use(express.json());

app.use("/", authRoutes);
app.use("/", feederRoutes);
app.use("/", deviceFactoryRoutes);

const swaggerOptions = {
    swaggerDefinition: {
      openapi: '3.0.0',
      info: {
        title: 'API de Comederos Automáticos',
        version: '1.0.0',
        description: 'Documentación de la API con Swagger',
      },
      servers: [
        {
          url: 'http://localhost:5000',
        },
      ],
    },
    apis: ['./routes/*.js'],
  };

const swaggerDocs = swaggerJsDoc(swaggerOptions);
app.use('/api-docs', swaggerUi.serve, swaggerUi.setup(swaggerDocs));

const PORT = process.env.PORT || 5000;
app.listen(PORT, () => console.log(`Servidor corriendo en el puerto ${PORT}`));
