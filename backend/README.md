# CatFeeder — Backend

API REST central del sistema **CatFeeder**. Gestiona usuarios, dispositivos,
pairing, configuración remota, órdenes de alimentación, conectividad e
historial.

El backend está diseñado para convivir con dos clientes:

- **Aplicación web/móvil:** autenticada mediante JWT.
- **Firmware ESP8266/NodeMCU:** autenticado mediante una credencial individual
  de dispositivo (`x-device-credential`).

> **Fuente de verdad:** el comportamiento implementado en `routes/`,
> controladores, middlewares, servicios y modelos. Swagger se genera a partir
> de las anotaciones de las rutas y se sirve en `/api-docs`.

## Índice

- [Arquitectura](#arquitectura)
- [Tecnologías](#tecnologías)
- [Ejecución](#ejecución)
- [Variables de entorno](#variables-de-entorno)
- [Autenticación](#autenticación)
- [Ciclo de vida del dispositivo](#ciclo-de-vida-del-dispositivo)
- [Referencia de la API](#referencia-de-la-api)
- [Configuración remota](#configuración-remota)
- [Motor y alimentación](#motor-y-alimentación)
- [Historial](#historial)
- [Modelo de datos](#modelo-de-datos)
- [Integración con el firmware](#integración-con-el-firmware)
- [Swagger](#swagger)
- [Estructura](#estructura)
- [Notas de implementación](#notas-de-implementación)

## Arquitectura

```text
                    ┌─────────────────────┐
                    │    MongoDB / Mongoose│
                    └──────────▲──────────┘
                               │
                    ┌──────────┴──────────┐
                    │    Express API      │
                    │    CatFeeder        │
                    └───────▲───────▲─────┘
                            │       │
              Bearer JWT    │       │ x-device-credential
                            │       │
                    ┌───────┘       └────────┐
                    │                        │
              ┌─────┴─────┐           ┌─────┴─────┐
              │ Frontend  │           │ ESP8266   │
              │ Web/Móvil │           │ NodeMCU   │
              └───────────┘           └───────────┘
```

El backend no mantiene una conexión persistente con el dispositivo para enviar
órdenes. La interacción del firmware con el backend se basa en consultas y
confirmaciones HTTP.

Flujo principal:

1. Se crea un dispositivo en fábrica mediante `/admin/devices`.
2. El backend genera una credencial individual para el dispositivo y
   credenciales de pairing.
3. El usuario inicia sesión y obtiene un JWT.
4. El usuario vincula el dispositivo mediante un `pairingToken` o
   `pairingCode`.
5. El usuario puede consultar y modificar el comedero asociado.
6. Las órdenes de alimentación se persisten en el backend.
7. El firmware consulta el estado remoto, ejecuta la orden y confirma su
   finalización.
8. Los eventos de alimentación pueden sincronizarse mediante `eventId` para
   evitar duplicados.

## Tecnologías

- Node.js
- Express 5
- MongoDB / Mongoose
- JSON Web Token (`jsonwebtoken`)
- bcryptjs
- Axios
- Luxon
- Swagger JSDoc / Swagger UI
- CORS
- EmailJS para recuperación de contraseña

## Ejecución

### Requisitos

- Node.js LTS
- MongoDB accesible desde el backend

### Instalación

```bash
npm install
```

### Ejecución

```bash
node index.js
```

El servidor utiliza `PORT` si está definido y, de lo contrario, `5000`.

La documentación Swagger queda disponible en:

```text
http://localhost:5000/api-docs
```

En Render, utilizar la URL pública correspondiente al servicio desplegado.

## Variables de entorno

Utilizar `.env.example` como referencia y **no versionar secretos reales**.

```dotenv
MONGO_URI=mongodb+srv://<usuario>:<password>@<cluster>/<database>?retryWrites=true&w=majority
JWT_SECRET=<secreto-largo-y-aleatorio>
PORT=5000
FRONTEND_URL=http://localhost:5173

EMAILJS_SERVICE_ID=service_xxx
EMAILJS_TEMPLATE_ID=template_xxx
EMAILJS_USER_ID=public_key_xxx
```

`NODEMCU_API_KEY` pertenece al diseño anterior de autenticación compartida.
Los endpoints actuales de dispositivo utilizan la credencial individual
`x-device-credential` generada durante el alta de fábrica.

## Autenticación

### Usuario — JWT

`POST /login` devuelve:

```json
{
  "token": "<jwt>"
}
```

Las rutas protegidas por usuario reciben:

```http
Authorization: Bearer <token>
```

El JWT se firma con `JWT_SECRET` y tiene una vigencia de una hora.

### Dispositivo — credencial individual

Los endpoints protegidos por firmware utilizan:

```http
x-device-credential: <deviceCredential>
```

La credencial se genera durante el alta de fábrica y el backend almacena su
hash. El middleware recupera el hash y compara la credencial recibida de forma
segura.

### No mezclar mecanismos

```text
Frontend ── JWT ──────────────────> endpoints de usuario
Firmware ── x-device-credential ──> endpoints de dispositivo
```

Una credencial de dispositivo no sustituye al JWT y un JWT no sustituye la
credencial del dispositivo.

## Ciclo de vida del dispositivo

### 1. Alta de fábrica

`POST /admin/devices` crea un feeder no asignado y genera las credenciales
necesarias para instalarlo.

Request:

```json
{
  "feederId": "esp8266-001",
  "feederName": "CatFeeder"
}
```

La operación genera una credencial individual de dispositivo y credenciales de
pairing. Los valores sensibles deben entregarse de forma segura durante la
instalación y no deben registrarse en logs.

### 2. Pairing

El usuario autenticado puede vincular un dispositivo utilizando exactamente una
de estas credenciales:

```json
{
  "pairingToken": "<token>"
}
```

o:

```json
{
  "pairingCode": "AB2CD3EF"
}
```

El token es hexadecimal de 64 caracteres. El código tiene 8 caracteres y se
normaliza a mayúsculas.

Una vez consumido el pairing:

- el dispositivo queda asignado al usuario;
- las credenciales de pairing quedan invalidadas;
- no pueden reutilizarse para otra vinculación.

El endpoint de pairing está protegido contra intentos repetidos: permite hasta
10 intentos por usuario/IP cada 15 minutos y devuelve `429` con `Retry-After`
cuando se supera el límite.

### 3. Desvinculación

`DELETE /devices/{feederId}/pair` permite al propietario desvincular el
comedero.

Al desvincular:

- `userId` vuelve a `null`;
- `feederAsign` vuelve a `false`;
- se generan nuevas credenciales de pairing.

La credencial individual del dispositivo no se regenera como parte del pairing.

## Referencia de la API

Todos los cuerpos utilizan `Content-Type: application/json`.

### Autenticación y cuenta

| Método | Ruta | Auth | Descripción |
| --- | --- | --- | --- |
| `POST` | `/register` | Pública | Registra un usuario. |
| `POST` | `/login` | Pública | Autentica un usuario y devuelve un JWT. |
| `POST` | `/forgot-password` | Pública | Genera un token de recuperación y solicita el envío por email. |
| `POST` | `/reset-password` | Pública | Restablece la contraseña usando un token vigente. |

### Dispositivos

| Método | Ruta | Auth | Descripción |
| --- | --- | --- | --- |
| `POST` | `/admin/devices` | Administración | Alta de un dispositivo en fábrica. |
| `POST` | `/devices/pair` | JWT | Vincula un dispositivo mediante token o código. |
| `DELETE` | `/devices/:feederId/pair` | JWT | Desvincula un dispositivo propio y genera nuevas credenciales de pairing. |

### Feeders — usuario

| Método | Ruta | Auth | Descripción |
| --- | --- | --- | --- |
| `GET` | `/feeders` | JWT | Lista feeders. |
| `GET` | `/feeders/my` | JWT | Lista feeders del usuario autenticado. |
| `GET` | `/feeders/:feederId` | JWT | Obtiene un feeder propio. |
| `POST` | `/feeder/edit` | JWT | Actualiza nombre e icono de un feeder propio. |
| `GET` | `/feeder/state/:feederId` | JWT | Consulta el estado del motor de un feeder propio. |
| `GET` | `/feeder/dates/:feederId` | JWT | Obtiene la programación del feeder. |
| `POST` | `/feeder/add-hour` | JWT | Agrega fechas futuras de alimentación. |
| `GET` | `/feeder/:feederId/historial` | JWT | Obtiene el historial del feeder. |
| `DELETE` | `/feeders/:feederId` | JWT | Elimina un feeder. |
| `POST` | `/feeder/start` | JWT | Genera una orden remota de alimentación. |

### Feeders — dispositivo

| Método | Ruta | Auth | Descripción |
| --- | --- | --- | --- |
| `GET` | `/feeders/global/:feederId` | Device Credential | Obtiene información global y actualiza la conexión del dispositivo. |
| `GET` | `/feeder/motor-state/:feederId` | Device Credential | Obtiene estado del motor, porciones, `commandId` y revisión de configuración. |
| `POST` | `/feeder/complete` | Device Credential | Confirma la finalización de una orden mediante `commandId`. |
| `POST` | `/feeders/history` | Device Credential | Sincroniza un evento mediante `eventId`. |
| `POST` | `/feeders/heartbeat` | Device Credential | Actualiza la última conexión del dispositivo. |
| `GET` | `/feeders/config/:feederId` | Device Credential | Obtiene la configuración remota. |

## Configuración remota

La configuración remota utiliza una **revisión incremental**.

```json
{
  "revision": 3,
  "stepsPerFeed": 2048,
  "schedules": [
    {
      "hour": 8,
      "minute": 30,
      "portions": 1,
      "enabled": true
    }
  ]
}
```

La configuración válida contiene exactamente **5 schedules**.

| Campo | Restricción |
| --- | --- |
| `revision` | Entero >= 1. |
| `stepsPerFeed` | Entero entre 1 y 10240. |
| `schedules` | Exactamente 5 elementos. |
| `hour` | Entero 0–23. |
| `minute` | Entero 0–59. |
| `portions` | Entero 1–5. |
| `enabled` | Boolean. |

`PUT /feeders/:feederId/config` compara la configuración recibida con la
actual. Si no hay cambios, conserva la revisión; si hay cambios, incrementa la
revisión en uno.

El firmware puede utilizar `revision` para detectar que debe sincronizar una
nueva configuración.

## Motor y alimentación

### Crear una orden

`POST /feeder/start` recibe:

```json
{
  "feederId": "esp8266-001",
  "portions": 2
}
```

`portions` es opcional y su valor por defecto es `1`.

Una orden aceptada queda identificada mediante un `commandId` único. El
backend persiste el estado necesario para que el firmware pueda detectarla.

### Consulta desde el firmware

El dispositivo consulta:

```text
GET /feeder/motor-state/:feederId
```

La respuesta incluye los datos necesarios para ejecutar la orden, por ejemplo:

```json
{
  "motorState": true,
  "portions": 2,
  "commandId": "<uuid>",
  "configRevision": 3
}
```

### Confirmación

Una vez finalizada la alimentación, el firmware llama:

```text
POST /feeder/complete
```

con:

```json
{
  "feederId": "esp8266-001",
  "commandId": "<uuid>"
}
```

El backend valida el `commandId` antes de marcar la orden como completada.
Esto permite que el firmware reintente una confirmación sin generar una nueva
orden.

## Historial

Los eventos sincronizados por el firmware utilizan un `eventId` estable para
permitir operaciones idempotentes.

Ejemplo conceptual:

```json
{
  "eventId": "evt-000001",
  "feederId": "esp8266-001",
  "timestamp": 1750000000,
  "portions": 1,
  "source": "scheduled"
}
```

Fuentes soportadas:

- `physical`
- `scheduled`
- `remote`
- `legacy`

El timestamp de sincronización del firmware se interpreta como Unix time en
segundos.

Si el `eventId` ya fue procesado, el backend evita crear un duplicado.

## Heartbeat

El firmware puede informar periódicamente su conectividad mediante:

```text
POST /feeders/heartbeat
```

El backend identifica el feeder mediante la credencial del dispositivo y
actualiza `lastConection`.

Respuesta exitosa:

```json
{
  "message": "Heartbeat recibido"
}
```

## Modelo de datos

### User

```text
name                  String
surname               String
email                 String, único
password              String, hash bcrypt
emailReceiver         Boolean
resetPasswordToken    String
resetPasswordExpires  Date
```

### Feeder

```text
feederId               String, único
userId                 String | null
feederName             String
feederLogo             String
feederAsign            Boolean
feederQuantity         Number
lastConection          Date
deviceCredentialHash   String
pairing.tokenHash      String
pairing.codeHash       String
pairing.usedAt         Date | null
```

### Motor

```text
motorInfo.startHours   Date[]
motorInfo.motorState    Boolean
motorInfo.portions      Number
motorInfo.commandId     String | null
```

### Configuration

```text
configuration.revision       Number
configuration.stepsPerFeed   Number
configuration.schedules      Array[5]
```

Cada schedule contiene:

```text
hour       Number 0..23
minute     Number 0..59
portions   Number 1..5
enabled    Boolean
```

Las credenciales y sus hashes no deben formar parte de las respuestas normales
del feeder.

## Integración con el firmware

Flujo recomendado:

```text
┌───────────────────────┐
│ Arranque del ESP      │
└──────────┬────────────┘
           │
           ▼
  ¿Tiene credencial?
      │             │
     NO            SÍ
      │             │
      ▼             ▼
 Configurar        Heartbeat
      │             │
      └──────┬──────┘
             ▼
     Consultar estado remoto
             │
             ▼
      ¿motorState = true?
        │             │
       NO            SÍ
        │             │
        │             ▼
        │       Ejecutar motor
        │             │
        │             ▼
        │       POST /feeder/complete
        │             │
        └──────┬──────┘
               ▼
        Sincronizar eventos
```

### Polling

El backend utiliza un modelo de polling. El firmware debe definir el intervalo
según latencia, consumo y requisitos de respuesta.

### Configuración

El firmware debe conservar su `revision` local. Cuando la revisión remota sea
mayor o diferente, debe descargar y aplicar la configuración correspondiente.

### Historial offline

El firmware puede conservar eventos localmente cuando no haya conectividad y
sincronizarlos posteriormente. Cada evento debe mantener un `eventId` estable.

## Swagger

Swagger UI está disponible en:

```text
/api-docs
```

La especificación se genera mediante `swagger-jsdoc` a partir de las
anotaciones ubicadas en `routes/`.

La documentación debe mantenerse sincronizada con el contrato HTTP:

```text
Código real
    ↓
routes + controllers + middleware + services
    ↓
Swagger
    ↓
Frontend / Firmware
```

Cuando se modifique una ruta, actualizar en el mismo cambio su anotación
Swagger y este README si cambia el contrato externo.

## Estructura

```text
backend/
├── configs/
│   └── db.js
├── controllers/
│   ├── authController.js
│   ├── deviceFactoryController.js
│   ├── devicePairingController.js
│   └── feederController.js
├── jobs/
│   └── motorStatusJob.js
├── middlewares/
│   ├── authMiddleware.js
│   ├── deviceAuthMiddleware.js
│   └── pairingRateLimitMiddleware.js
├── models/
│   ├── User.js
│   └── Feeder.js
├── routes/
│   ├── authRoutes.js
│   ├── deviceFactoryRoutes.js
│   ├── devicePairingRoutes.js
│   └── feederRoutes.js
├── services/
│   └── devicePairingServices.js
├── utils/
│   ├── deviceCredential.js
│   ├── feederResponse.js
│   └── pairingCredential.js
├── index.js
├── package.json
├── .env.example
└── README.md
```

## Notas de implementación

### Nombres históricos

El modelo conserva nombres establecidos durante versiones anteriores, como
`feederAsign` y `lastConection`. No renombrarlos unilateralmente sin una
migración de datos y actualización de clientes.

### `userId`

Actualmente `userId` debe tratarse según el esquema real del modelo. No asumir
una referencia Mongoose distinta a la implementada.

### Credenciales

Las credenciales sensibles se almacenan mediante hash cuando corresponde. No
deben incluirse en logs ni documentarse con valores reales.

### Pairing

El pairing es de un solo uso. Al desvincular un dispositivo se generan nuevas
credenciales de pairing para una futura vinculación.

### Revisión de configuración

`revision` es el mecanismo de sincronización de cambios de configuración entre
backend y firmware. El firmware debe conservar y comparar ese valor.

### Idempotencia

`commandId` identifica órdenes de alimentación y `eventId` identifica eventos
de historial. Son conceptos distintos y no deben intercambiarse.

## Evolución futura

Las siguientes mejoras no forman parte del contrato actual y no deben tratarse
como endpoints existentes hasta que sean implementadas:

- health check dedicado;
- validación centralizada mediante schemas;
- roles administrativos formales;
- rate limiting generalizado;
- rotación/revocación individual de credenciales de dispositivo;
- tests automatizados de integración;
- migraciones de datos para cambios de esquema.

Cuando una mejora pase a producción, actualizar conjuntamente código, Swagger
y README.