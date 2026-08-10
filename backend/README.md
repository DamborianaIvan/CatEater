# Cat Feeder — Backend

API REST para un comedero automático de mascotas. Centraliza usuarios, la
asignación de cada dispositivo, el estado del motor, los horarios de comida y
el historial de activaciones. Está pensada para que convivan dos clientes:

- una aplicación de usuario que se autentica con JWT;
- un NodeMCU/ESP que se identifica mediante una API key compartida.

> **Estado del proyecto:** este README documenta el comportamiento del código
> actual, incluyendo sus limitaciones. Antes de reutilizarlo en producción,
> revisar la sección [Aspectos a corregir](#aspectos-a-corregir-antes-de-producción).

## Índice

- [Arquitectura y flujo](#arquitectura-y-flujo)
- [Tecnologías](#tecnologías)
- [Inicio rápido](#inicio-rápido)
- [Configuración](#configuración)
- [Modelo de datos](#modelo-de-datos)
- [Autenticación](#autenticación)
- [Referencia de la API](#referencia-de-la-api)
- [Integración del dispositivo](#integración-del-dispositivo)
- [Programación e historial](#programación-e-historial)
- [Mapa del código](#mapa-del-código)
- [Aspectos a corregir antes de producción](#aspectos-a-corregir-antes-de-producción)
- [Guía para agentes de IA](#guía-para-agentes-de-ia)

## Arquitectura y flujo

```text
App móvil/web ── Bearer JWT ──┐
                              │
                              ▼
                         Express API ─── MongoDB
                              ▲
                              │
NodeMCU/ESP ─ x-api-key ──────┘
```

1. El dispositivo se registra con un `feederId` único.
2. Un usuario crea su cuenta, inicia sesión y recibe un JWT de una hora.
3. El usuario reclama (`assign`) un comedero disponible y puede cambiar su
   nombre, icono y horarios.
4. La aplicación solicita activar o detener el motor; el dispositivo consulta
   el estado del motor y actúa en consecuencia.
5. El dispositivo puede informar una activación para añadirla al historial.

La API no tiene prefijo global: las rutas son, por ejemplo, `/login` y
`/feeders/my`. La documentación Swagger se sirve en `/api-docs`.

## Tecnologías

- Node.js con Express 5.
- MongoDB con Mongoose.
- JWT (`jsonwebtoken`) y contraseñas con hash bcrypt.
- CORS habilitado para todos los orígenes.
- Luxon para la zona horaria del job programado.
- Swagger UI generado desde anotaciones en `routes/`.
- EmailJS vía Axios para recuperación de contraseña.

## Inicio rápido

### Requisitos

- Node.js (se recomienda una versión LTS actual).
- Una instancia accesible de MongoDB.

### Instalación y ejecución

```bash
npm install
node index.js
```

No hay un script `start` o `dev` definido actualmente en `package.json`. El
servidor usa `PORT` o, si no existe, el puerto `5000`.

Al iniciarse, primero intenta conectar a MongoDB. Si falla la conexión, el
proceso termina. Con una ejecución local correcta, la documentación queda en:

```text
http://localhost:5000/api-docs
```

## Configuración

Crear un archivo `.env` en la raíz del backend. Nunca subirlo al repositorio.

```dotenv
# Obligatorias para el servidor
MONGO_URI=mongodb://127.0.0.1:27017/cat-feeder
JWT_SECRET=generar-un-secreto-largo-y-aleatorio
NODEMCU_API_KEY=clave-compartida-con-el-dispositivo
PORT=5000

# Necesarias únicamente para recuperar contraseñas por EmailJS
EMAILJS_SERVICE_ID=service_xxx
EMAILJS_TEMPLATE_ID=template_xxx
EMAILJS_USER_ID=public_key_xxx
API_URL=http://localhost:3000/reset-password
```

`API_URL` debe ser la URL base del frontend que muestra el formulario de nueva
contraseña; el backend agrega `/<token>` al final. Aunque el `.env` existente
solo contiene las primeras cuatro variables, el controlador de recuperación
también necesita las variables de EmailJS y `API_URL`.

## Modelo de datos

### `User`

| Campo | Tipo | Uso |
| --- | --- | --- |
| `name`, `surname` | String | Datos obligatorios de perfil. |
| `email` | String único | Identificador de inicio de sesión. |
| `password` | String | Hash bcrypt; nunca se persiste la contraseña plana. |
| `emailReceiver` | Boolean | Preferencia opcional. |
| `resetPasswordToken`, `resetPasswordExpires` | String / Date | Recuperación de contraseña, válida por una hora. |

### `Feeder`

| Campo | Tipo | Uso |
| --- | --- | --- |
| `feederId` | String único | Identificador físico/lógico enviado por el dispositivo. |
| `userId` | String o `null` | Usuario propietario. Actualmente no es una referencia Mongoose. |
| `feederName`, `feederLogo` | String | Personalización que define el usuario. |
| `feederAsign` | Boolean | Indica si fue asignado. La grafía es parte del esquema actual. |
| `feederQuantity` | Number | Cantidad/medición reportada; no hay endpoint que hoy la actualice. |
| `lastConection` | Date | Última conexión, con esa grafía en el esquema. |
| `motorInfo.startHours` | `Date[]` | Fechas y horas futuras programadas. |
| `motorInfo.motorState` | Boolean | Orden persistida que consulta el dispositivo. |
| `feederHistory` | Array | Eventos `{ fecha, accion }`, donde `accion` es `encendido` o `apagado`. |

También existe el modelo `FeederLog` (`feederId`, `timestamp`, `amount`), pero
no es utilizado por las rutas ni los controladores actuales.

## Autenticación

### JWT para app de usuario

Después de `POST /login`, enviar el token en todas las rutas protegidas:

```http
Authorization: Bearer <token>
```

El payload contiene `_id` del usuario y el token vence en una hora. El
middleware acepta también un token sin el prefijo `Bearer`.

### API key para dispositivo

Las rutas de firmware verifican exactamente este encabezado:

```http
x-api-key: <NODEMCU_API_KEY>
```

No sustituir una forma de autenticación por la otra: un JWT no autoriza al
dispositivo y la API key no autoriza las acciones de un usuario.

## Referencia de la API

Los cuerpos se envían como JSON. Las rutas marcadas **JWT** requieren
`Authorization`; las marcadas **dispositivo** requieren `x-api-key`.

### Usuarios

| Método y ruta | Aut. | Body / resultado |
| --- | --- | --- |
| `POST /register` | Pública | `{ email, password, name, surname, emailReceiver? }`. Contraseña: mínimo 8 caracteres y una mayúscula. |
| `POST /login` | Pública | `{ email, password }` → `{ token }`. |
| `POST /forgot-password` | Pública | `{ email }`; genera un token de una hora y usa EmailJS. |
| `POST /reset-password` | Pública | `{ token, password }`; restablece la contraseña. |

Ejemplo de registro:

```bash
curl -X POST http://localhost:5000/register \
  -H "Content-Type: application/json" \
  -d '{"name":"Ada","surname":"Lovelace","email":"ada@example.com","password":"ClaveSegura1"}'
```

### Comederos y dispositivo

| Método y ruta | Aut. | Propósito |
| --- | --- | --- |
| `POST /feeders/register` | Dispositivo | Registra un comedero. Body: `{ feederId, name }`; el esquema no persiste `name`, sino que luego se usa `feederName`. |
| `GET /feeders/global/:feederId` | Dispositivo | Devuelve `feederQuantity`, `feederName`, `feederLogo` y `lastConection`. Puede marcar el motor si la hora UTC coincide con un horario. |
| `GET /feeder/motor-state/:feederId` | Dispositivo | Devuelve `{ motorState: boolean }`; es el endpoint de consulta de la orden del motor. |
| `POST /feeder/history/add` | Dispositivo | Body: `{ feederId }`; añade un evento de encendido y agenda uno de apagado 40 segundos después. |

### Gestión desde la aplicación

| Método y ruta | Propósito |
| --- | --- |
| `GET /feeders` | Lista todos los comederos. **JWT**, sin restricción de propietario ni rol administrador. |
| `GET /feeders/my` | Lista los comederos del usuario autenticado. **JWT**. |
| `GET /feeders/:feederId` | Obtiene un comedero propio. **JWT**. |
| `POST /feeder/assign` | Reclama un comedero: `{ feederId, feederName, feederLogo }`. **JWT**. |
| `POST /feeder/unassign` | Libera un comedero propio: `{ feederId }`. **JWT**. |
| `POST /feeder/edit` | Actualiza nombre e icono de uno propio: `{ feederId, feederName, feederLogo }`. **JWT**. |
| `GET /feeder/state/:feederId` | Obtiene `{ motorState }` de un comedero propio. **JWT**. |
| `POST /feeder/stop` | Solicita detener el motor de un comedero propio: `{ feederId }`. **JWT**. |
| `GET /feeder/dates/:feederId` | Devuelve `{ dates }` para un comedero propio. **JWT**. |
| `POST /feeder/add-hour` | Añade fechas futuras no repetidas: `{ feederId, dates: [ISO-8601, ...] }`. **JWT**. |
| `GET /feeder/:feederId/historial` | Devuelve el historial formateado para UI, más reciente primero. **JWT**. |
| `DELETE /feeders/:feederId` | Elimina por ID. **JWT**, pero no valida propiedad; usar solo en desarrollo. |
| `POST /feeder/start` | Está declarada pero su controlador no implementa respuesta ni modificación: no usar. |

Ejemplo de asignación:

```bash
curl -X POST http://localhost:5000/feeder/assign \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{"feederId":"esp32-001","feederName":"Comedero cocina","feederLogo":"cat"}'
```

## Integración del dispositivo

Secuencia mínima para un firmware compatible:

1. En el primer arranque, hacer `POST /feeders/register` con la API key y un
   `feederId` persistente (idealmente derivado de MAC o chip ID).
2. En un intervalo regular, llamar a
   `GET /feeder/motor-state/:feederId` con la misma API key.
3. Si `motorState` es `true`, accionar el motor según la seguridad física
   requerida y reportar el ciclo con `POST /feeder/history/add`.
4. Opcionalmente consultar `GET /feeders/global/:feederId` para presentar en
   el dispositivo el nombre, icono o cantidad.

El backend no manda comandos al dispositivo: el modelo es *polling*. Ajustar
el intervalo de consulta en firmware según la latencia aceptable y el consumo
de red. La API key actual es compartida para todos los dispositivos; si se
necesita revocación individual, migrar a credenciales por `feederId`.

## Programación e historial

`POST /feeder/add-hour` recibe fechas ISO-8601 completas, las guarda como
`Date` y rechaza valores pasados o ya existentes. No existe una ruta para
eliminar o editar horarios individualmente.

`jobs/motorStatusJob.js` se carga al importar el controlador de feeders e
intenta revisar horarios cada cinco segundos usando la zona
`America/Argentina/Buenos_Aires`. Además, `GET /feeders/global/:feederId`
intenta activar el estado por coincidencia de minuto en UTC. Son dos
mecanismos distintos; antes de extender la agenda conviene consolidarlos en
uno solo y definir la zona horaria como configuración.

El historial que recibe la aplicación se transforma a esta forma:

```json
[
  {
    "id": "1",
    "fecha": "mié, 14 may",
    "hora": "08:30",
    "cantidad": "20g",
    "accion": "Comedero activado"
  }
]
```

La cantidad `20g` está fija en el controlador: no surge de un sensor ni de
`FeederLog`.

## Mapa del código

```text
index.js                         Arranque de Express, MongoDB, CORS y Swagger
configs/db.js                    Conexión Mongoose mediante MONGO_URI
routes/authRoutes.js             Rutas de cuenta y sesión
routes/feederRoutes.js           Rutas de usuario y firmware
controllers/authController.js    Registro, login y recuperación de contraseña
controllers/feederController.js  Lógica de propiedad, motor, agenda e historial
middlewares/authMiddleware.js    Verificación del JWT
models/User.js                   Esquema de usuario
models/Feeder.js                 Esquema principal del comedero
models/FeederLog.js              Esquema no integrado aún
jobs/motorStatusJob.js           Revisión periódica de horarios
```

## Aspectos a corregir antes de producción

Estos puntos son observaciones del código actual, no funcionalidades ya
resueltas:

1. **Arranque manual:** agregar scripts `start`, `dev` y tests al
   `package.json`.
2. **Motor:** implementar `startMotor`; hoy `POST /feeder/start` queda sin
   respuesta. Corregir además mensajes, duración y lógica de `stopMotor`.
3. **Scheduler:** el job inserta valores incompatibles con el esquema de
   `feederHistory` y mezcla zonas horarias/formatos. Debe registrar
   `{ fecha, accion }`, evitar duplicados correctamente y tener pruebas.
4. **Consistencia de campos:** unificar `lastConection` con los usos de
   `lastConexion` (estos últimos no corresponden al esquema) y usar
   `feederName` al registrar en lugar de `name`.
5. **Seguridad:** restringir `GET /feeders` y `DELETE /feeders/:feederId` a un
   rol administrador o al propietario; definir CORS por entorno; no registrar
   secretos, tokens o enlaces de recuperación en consola.
6. **Contraseñas:** aplicar las mismas reglas de complejidad en
   `reset-password` que en `register`.
7. **Errores y validación:** centralizar códigos/respuestas, validar todos los
   cuerpos con un esquema y evitar devolver objetos de error internos.
8. **Diseño de datos:** convertir `userId` a `ObjectId` con `ref: 'User'`;
   conectar o eliminar `FeederLog`; implementar persistencia de cantidad si
   el producto la necesita.
9. **Operación:** agregar health check, logs estructurados, rate limiting,
   HTTPS y rotación de credenciales por dispositivo.

## Guía para agentes de IA

Al modificar o reutilizar este backend, respetar estas decisiones y límites:

- Tomar las rutas reales de `routes/` y los controladores como fuente de
  verdad; las anotaciones Swagger contienen partes desactualizadas.
- No asumir que un endpoint de motor acciona hardware: la API solo persiste un
  estado que el firmware consulta por *polling*.
- Mantener separadas la autenticación de usuario (JWT) y de dispositivo
  (`x-api-key`). No exponer `NODEMCU_API_KEY` a clientes web/móviles.
- Las fechas de `startHours` son instantes `Date`, no horarios recurrentes. Si
  se quieren comidas diarias, diseñar explícitamente recurrencia, zona horaria
  y política de reintentos.
- Verificar propiedad del comedero en cualquier endpoint nuevo que modifique o
  exponga datos de usuario.
- No alterar ni imprimir `.env`; crear un `.env.example` sin secretos si se
  publica el repositorio.
- Antes de cambiar el scheduler o la comunicación con el firmware, probar el
  ciclo completo: alta del dispositivo → asignación → orden de motor → polling
  → historial.

