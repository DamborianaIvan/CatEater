# 1. Resumen del proyecto

## Objetivo general

Este backend expone una API REST para operar comederos automáticos de mascotas. Centraliza el alta de usuarios, la vinculación de un comedero a un usuario, la personalización del comedero, la programación de fechas de activación, el estado lógico del motor y un historial de encendidos y apagados.

El sistema contempla dos clientes: una aplicación de usuario y un dispositivo NodeMCU/ESP. El backend no acciona hardware directamente: persiste el estado del motor para que el firmware lo consulte.

## Arquitectura general

- Proceso Node.js con Express 5 y cuerpos JSON.
- MongoDB, accedido mediante Mongoose, como persistencia de usuarios y comederos.
- Rutas de autenticación y de comederos montadas en la raíz, sin prefijo común.
- Autenticación separada: JWT para usuarios y una API key compartida para firmware.
- Job cargado al importar el controlador de comederos, que revisa horarios cada cinco segundos.
- Swagger UI en `/api-docs`, generado desde anotaciones de las rutas.
- EmailJS invocado por HTTP mediante Axios para recuperación de contraseña.

## Flujo principal

1. El firmware registra un `feederId` usando la API key compartida.
2. Una persona registra una cuenta e inicia sesión; el login devuelve un JWT de una hora.
3. La persona autenticada asigna un comedero existente y le define nombre e icono.
4. Puede consultar, programar fechas futuras, iniciar o detener el estado lógico del motor y consultar el historial.
5. El firmware consulta por *polling* el estado lógico del motor y puede informar una activación para registrar el historial.

# 2. Responsabilidades del backend

- Registro, validación básica, almacenamiento y autenticación de usuarios.
- Generación y validación de JWT para la aplicación de usuario.
- Recuperación y restablecimiento de contraseñas mediante token temporal y EmailJS.
- Registro de dispositivos/comederos desde firmware.
- Consulta global de datos básicos del comedero para firmware.
- Listado global de comederos autenticado, y consultas de comederos propios.
- Asignación, liberación y edición de la personalización de un comedero.
- Eliminación de comederos mediante una ruta marcada en el código como de desarrollo.
- Persistencia y consulta del estado lógico del motor.
- Inicio, detención y apagado automático del estado lógico del motor en flujos determinados.
- Programación de instantes futuros de alimentación.
- Revisión periódica de horarios mediante un scheduler en proceso.
- Registro y presentación del historial de encendido/apagado.
- Exposición de documentación Swagger.
- Conexión inicial a MongoDB y terminación del proceso si dicha conexión falla.

# 3. Flujo completo del sistema

## Registro del dispositivo

1. El firmware envía `POST /feeders/register` con `x-api-key` y un cuerpo no vacío que contiene, en la práctica, `feederId` y normalmente `name`.
2. El backend compara la clave con `NODEMCU_API_KEY`.
3. Si no existe otro documento con el mismo `feederId`, crea un `Feeder` sin propietario, con los valores por defecto del esquema. El campo `name` enviado no existe en el esquema; por ello no forma parte del documento persistido.

## Registro del usuario

1. La aplicación envía nombre, apellido, email, contraseña y opcionalmente `emailReceiver` a `POST /register`.
2. Se exige email válido, campos obligatorios y contraseña de al menos ocho caracteres con una mayúscula.
3. Se rechaza un email ya existente.
4. La contraseña se guarda con hash bcrypt (10 rondas). La respuesta confirma el registro y no inicia sesión.

## Login

1. La aplicación envía email y contraseña a `POST /login`.
2. El backend valida la forma del email, busca al usuario y compara la contraseña contra el hash bcrypt.
3. Si coincide, firma un JWT con el identificador MongoDB del usuario en `_id` y vencimiento de una hora.
4. El cliente utiliza ese token en las rutas protegidas.

## Asignación del feeder

1. El usuario autenticado envía `feederId`, `feederName` y `feederLogo` a `POST /feeder/assign`.
2. El backend busca el comedero, verifica que no esté asignado y almacena el `_id` del usuario como texto en `userId`.
3. También guarda nombre, icono y `feederAsign: true`.
4. Las consultas posteriores de comederos propios combinan `feederId` y `userId` del JWT.

## Funcionamiento diario

1. La aplicación puede recuperar los comederos propios, consultar su estado, añadir instantes futuros a `motorInfo.startHours` y leer su historial.
2. El job en memoria revisa cada cinco segundos los comederos apagados que tienen horarios. De manera independiente, la consulta global del firmware compara el minuto UTC actual con los horarios guardados y, si coincide, intenta dejar el motor en `true`.
3. El firmware consulta `GET /feeder/motor-state/:feederId` con la API key para leer el valor persistido de `motorInfo.motorState`.

## Activación del motor

- Inicio manual: `POST /feeder/start`, con JWT y propiedad comprobada, cambia el estado a `true`, registra `encendido` y programa en memoria un apagado y evento `apagado` 40 segundos después.
- Detención manual: `POST /feeder/stop`, con JWT y propiedad comprobada, cambia el estado a `false` y registra `apagado`.
- Activación desde horario: la consulta global puede cambiar el estado a `true` cuando coincide el minuto UTC. El job intenta hacer una activación basada en su propia lógica de zona horaria.
- El firmware no recibe una orden saliente: lee el estado mediante polling y decide la acción física.

## Registro del historial

1. El inicio manual agrega `{ fecha, accion: "encendido" }`; su apagado automático agrega `{ fecha, accion: "apagado" }`.
2. La detención manual agrega un evento `apagado`.
3. El firmware puede llamar a `POST /feeder/history/add`: agrega `encendido` y programa solo un registro `apagado` 40 segundos después; ese endpoint no cambia `motorState`.
4. `GET /feeder/:feederId/historial` transforma cada evento a textos en español de Argentina, añade `cantidad: "20g"` fija y devuelve el arreglo en orden inverso.

# 4. Inventario completo de endpoints

La tabla describe las rutas registradas en los archivos de rutas. Todas reciben JSON cuando tienen body. `JWT` implica `Authorization`; `API key` implica `x-api-key`.

| Método | Ruta | Autenticación requerida | Body | Respuesta | Objetivo funcional |
| --- | --- | --- | --- | --- | --- |
| POST | `/register` | No | `email`, `password`, `name`, `surname`, `emailReceiver?` | 201 con confirmación; 400 ante validación o email existente | Crear una cuenta de usuario. |
| POST | `/login` | No | `email`, `password` | 200 `{ token }`; 400 si credenciales o email no son válidos | Autenticar al usuario y emitir JWT. |
| POST | `/forgot-password` | No | `email` | 200 con confirmación de envío; 404 si no existe; 500 si falla EmailJS | Generar token de recuperación temporal y enviar enlace. |
| POST | `/reset-password` | No | `token`, `password` | 200 con confirmación; 400 por token, vencimiento o body inválido | Reemplazar la contraseña y anular el token de recuperación. |
| POST | `/feeders/register` | API key | Cuerpo no vacío; usa `feederId` y acepta `name` | 201 con confirmación; 401, 400, 409 o 500 según caso | Dar de alta un comedero desde firmware. |
| GET | `/feeders/global/:feederId` | API key | Sin body | 200 con mensaje, `feederQuantity`, `feederName`, `feederLogo`, `lastConection`; errores 400/401/403/404/500 | Consultar datos globales del comedero y comprobar coincidencia de horario UTC. |
| GET | `/feeders` | JWT | Sin body | Arreglo de todos los documentos `Feeder` | Listar todos los comederos, sin filtro de propietario. |
| GET | `/feeders/my` | JWT | Sin body | Arreglo de propios, o `{ message, feeders: [] }` si no hay | Listar comederos asociados al usuario. |
| GET | `/feeders/:feederId` | JWT | Sin body | Documento `Feeder` propio; 400/401/404/500 si corresponde | Recuperar un comedero propio. |
| POST | `/feeder/assign` | JWT | `feederId`, `feederName`, `feederLogo` | 200 con confirmación e información asignada | Vincular un comedero disponible al usuario. |
| POST | `/feeder/unassign` | JWT | `feederId` | 200 con confirmación y documento actualizado | Liberar un comedero propio y quitar su nombre/icono. |
| POST | `/feeder/start` | JWT | `feederId` | 200 con confirmación; errores de propiedad, estado o existencia | Dejar activo el estado lógico del motor y registrar su ciclo. |
| POST | `/feeder/stop` | JWT | `feederId` | 200 con confirmación; errores de propiedad, estado o existencia | Dejar inactivo el estado lógico del motor y registrar apagado. |
| POST | `/feeder/edit` | JWT | `feederId`, `feederName`, `feederLogo` | 200 con confirmación e información editada | Cambiar personalización de un comedero propio. |
| GET | `/feeder/motor-state/:feederId` | API key | Sin body | `{ motorState }` | Permitir al firmware consultar el estado lógico del motor. |
| GET | `/feeder/state/:feederId` | JWT | Sin body | `{ motorState }` | Permitir al propietario consultar el estado lógico del motor. |
| GET | `/feeder/dates/:feederId` | JWT | Sin body | `{ dates }` | Devolver los instantes programados de un comedero propio. |
| POST | `/feeder/add-hour` | JWT | `feederId`, `dates` (arreglo) | 200 con `dates` completas; 400/404/500 según caso | Agregar instantes futuros, válidos y no duplicados a un comedero propio. |
| DELETE | `/feeders/:feederId` | JWT | Sin body | Confirmación; 400/404/500 según caso | Eliminar un comedero por `feederId`; no comprueba propiedad. |
| GET | `/feeder/:feederId/historial` | JWT | Sin body | Arreglo de historial formateado, más reciente primero | Consultar historial de un comedero propio. |
| POST | `/feeder/history/add` | API key | `feederId` | 200 con confirmación; 401/403/404/500 según caso | Informar una activación desde firmware y crear eventos de historial. |

La función interna `clearFeederHistory` existe, pero no se exporta ni está conectada a una ruta. La ruta `/feeder/end` aparece comentada y no está expuesta.

# 5. Modelo de datos

## `User`

- **Propósito:** identidad, credenciales y datos de recuperación de la persona usuaria.
- **Campos principales:** `name` y `surname` obligatorios; `email` obligatorio y único; `password` obligatorio; `emailReceiver` opcional; `resetPasswordToken` y `resetPasswordExpires` opcionales.
- **Relaciones:** no contiene referencias Mongoose a comederos. Los `Feeder` asociados guardan el identificador de usuario como texto en `userId`.
- **Observaciones:** la contraseña persistida es el hash bcrypt. El token de recuperación se almacena directamente como texto y se elimina luego de restablecer la contraseña.

## `Feeder`

- **Propósito:** estado principal del dispositivo, su propiedad, configuración, agenda e historial.
- **Campos principales:** `feederId` obligatorio y único; `userId` string con valor inicial `null`; `feederName` y `feederLogo` string con valor inicial de un espacio; `feederAsign` booleano (`false` por defecto); `feederQuantity` numérico (`0`); `lastConection` fecha; `motorInfo.startHours` arreglo de fechas; `motorInfo.motorState` booleano (`false`); `feederHistory` arreglo de subdocumentos.
- **Relaciones:** asociación lógica con `User` mediante `userId`, sin `ObjectId` ni `ref` de Mongoose. No tiene relación de esquema con `FeederLog`.
- **Observaciones:** cada elemento válido de `feederHistory` exige `accion` con valor `encendido` o `apagado` y tiene `fecha`. Parte del código escribe `lastConexion` (con `x`) en lugar de `lastConection` (sin `x`), por lo que no corresponde al campo definido. `feederQuantity` se devuelve al firmware, pero no hay ruta que lo actualice.

## `FeederLog`

- **Propósito:** modelo separado para registros de cantidad por comedero.
- **Campos principales:** `feederId` obligatorio, `timestamp` con valor por defecto `Date.now` y `amount` obligatorio.
- **Relaciones:** relación lógica por `feederId` con `Feeder`; no hay referencia ni uso desde controladores o rutas.
- **Observaciones:** el modelo se declara pero no participa en el flujo expuesto de la API.

# 6. Autenticación

## JWT

- `POST /login` firma con `JWT_SECRET` un payload `{ _id: user._id }` y vencimiento de una hora.
- El middleware `verifyToken` lee `Authorization`. Acepta tanto `Bearer <token>` como el token sin prefijo, verifica la firma con `JWT_SECRET` y coloca el payload en `req.user`.
- La ausencia de encabezado devuelve 401; la verificación fallida devuelve 400. Las rutas protegidas usan el `_id` de `req.user` para filtrar o verificar propiedad cuando su controlador lo implementa.

## API Key

- El firmware presenta `x-api-key` y el controlador compara su valor literal con `NODEMCU_API_KEY`.
- La utilizan registro de dispositivo, consulta global, consulta de estado del motor y agregado de historial.
- No existe middleware compartido para esta clave: cada controlador de firmware repite la comprobación. En el registro, una clave faltante o incorrecta recibe 401; en las otras rutas, ausencia recibe 401 y valor incorrecto 403.
- La clave es común a todos los dispositivos; no se persisten ni se distinguen credenciales por `feederId`.

## Flujo de autenticación

1. La aplicación crea una cuenta y luego hace login para obtener el JWT.
2. Adjunta el JWT en `Authorization` al operar sobre comederos, agenda, motor e historial.
3. El firmware no usa JWT: se autentica con la API key al registrarse y al hacer polling o informar historial.
4. Para recuperar contraseña, se genera un token aleatorio hexadecimal de 32 bytes, se guarda con vencimiento de una hora y se incorpora a un enlace construido con `API_URL`.

# 7. Comunicación con el firmware

## Registro

El firmware registra el comedero mediante `POST /feeders/register` y una API key válida. El identificador único efectivo es `feederId`. Aunque la documentación Swagger menciona `name`, el esquema persiste `feederName`; el registro intenta asignar `name`, campo que no está en el esquema.

## Polling

No hay canal de mensajes saliente, sockets ni llamadas del backend al firmware. El firmware debe consultar periódicamente `GET /feeder/motor-state/:feederId`. La respuesta es el booleano persistido `motorState`.

## Consulta del motor

El endpoint de firmware consulta cualquier `Feeder` que coincida con el `feederId`, independientemente de su asignación, y devuelve el estado de `motorInfo.motorState`. El backend cambia ese valor por inicio o detención manual, por una coincidencia de horario durante la consulta global, o por el job programado si logra guardar sus cambios.

## Historial

Tras una activación física, el firmware puede comunicar `POST /feeder/history/add` con `feederId`. El backend registra un evento `encendido` y, tras 40 segundos en la memoria del proceso, inserta un evento `apagado`. No recibe duración, cantidad ni estado del motor desde el firmware.

## Sincronización

La consulta opcional `GET /feeders/global/:feederId` devuelve cantidad, nombre, icono y última conexión. En la misma consulta, el backend compara la hora y minuto UTC actuales con cada fecha de `startHours`; una coincidencia intenta actualizar `motorState` a `true`. La respuesta de esa misma petición se construye desde el documento recuperado antes de la actualización.

## Estado

El estado operativo compartido con el firmware se reduce a `motorInfo.motorState`. `lastConection` se actualiza durante inicio, detención y liberación; otros caminos usan el nombre no definido `lastConexion`. El firmware no actualiza explícitamente una marca de conexión ni `feederQuantity` mediante las rutas existentes.

# 8. Scheduler

El archivo `jobs/motorStatusJob.js` se importa desde el controlador de comederos, por lo que su `setInterval` queda activo al cargar ese controlador. Ejecuta `revisarHoras` cada cinco segundos.

En cada ejecución busca comederos con el motor apagado y un arreglo no vacío de `startHours`. Obtiene la hora actual con Luxon en la zona `America/Argentina/Buenos_Aires`; compara cada fecha programada con una ventana de diez segundos y, si considera que corresponde, intenta poner el motor en `true`, guardar un elemento en el historial y agendar que vuelva a `false` 15 segundos después.

El comportamiento observado incluye estas particularidades del código actual:

- El control de ejecuciones previas itera los elementos de `feederHistory` como si fueran fechas, pero el esquema define subdocumentos `{ fecha, accion }`.
- Para activar, inserta directamente una fecha Luxon en `feederHistory`, mientras el esquema exige un subdocumento con `accion`; al guardar puede producirse un error de validación que es capturado por el job.
- La comparación de horarios mezcla la hora zonificada de Luxon con `Date#getHours`, que usa la zona del proceso.
- Es distinto del mecanismo dentro de `GET /feeders/global/:feederId`, que compara minuto UTC y no programa apagado ni historial.

# 9. Dependencias externas

| Dependencia o servicio | Uso presente en el proyecto |
| --- | --- |
| Node.js / Express 5 | Servidor HTTP, enrutamiento y JSON. |
| MongoDB | Base de datos requerida; la URI se toma de `MONGO_URI`. |
| Mongoose | Conexión y esquemas `User`, `Feeder` y `FeederLog`. |
| dotenv | Carga de variables de entorno. |
| bcryptjs | Hash y comparación de contraseñas. |
| jsonwebtoken | Firma y verificación de JWT. |
| validator | Validación del formato de email. |
| cors | CORS habilitado globalmente. |
| Luxon | Cálculo de hora en el scheduler. |
| Swagger (`swagger-jsdoc`, `swagger-ui-express`) | Generación y publicación de Swagger UI en `/api-docs`. |
| Axios | Solicitud HTTP a EmailJS. |
| EmailJS | Servicio externo para el email de recuperación; requiere `EMAILJS_SERVICE_ID`, `EMAILJS_TEMPLATE_ID` y `EMAILJS_USER_ID`. |
| crypto nativo de Node.js | Generación de token aleatorio de recuperación. |
| `@sendgrid/mail` | Declarado en `package.json`, sin uso en el código. |
| nodemailer | Declarado en `package.json`, sin uso en el código. |

Variables de entorno observadas: `MONGO_URI`, `JWT_SECRET`, `NODEMCU_API_KEY`, `PORT`, `EMAILJS_SERVICE_ID`, `EMAILJS_TEMPLATE_ID`, `EMAILJS_USER_ID` y `API_URL`.

# 10. Responsabilidades funcionales

- Mantener cuentas de usuarios y preferencias de recepción de email.
- Validar acceso de usuarios mediante sesión JWT.
- Recuperar acceso mediante enlace de restablecimiento de contraseña.
- Identificar y registrar comederos únicos.
- Mantener el ciclo de vida de asignación entre usuario y comedero.
- Personalizar el nombre e icono visible de un comedero.
- Exponer el inventario de dispositivos y las vistas por propietario.
- Mantener el estado lógico de activación del motor.
- Permitir activación y apagado manual por el propietario.
- Programar instantes futuros de activación del motor.
- Revisar automáticamente horarios almacenados.
- Permitir al firmware leer instrucciones de motor por polling.
- Entregar al firmware datos de identificación y cantidad almacenada del comedero.
- Recibir desde firmware una notificación de activación para el historial.
- Conservar y presentar el historial de activaciones en formato de interfaz.
- Publicar una documentación navegable de la API.
- Permitir la eliminación autenticada de un comedero, indicada en el código como uso de desarrollo.

# 11. Funcionalidades candidatas para reutilizar

| Funcionalidad | Descripción | Observaciones |
| --- | --- | --- |
| Registro y login de usuario | Alta con validaciones básicas y emisión de sesión JWT. | La contraseña se almacena con bcrypt y el JWT dura una hora. |
| Recuperación de contraseña | Token temporal persistido y envío de enlace por EmailJS. | Depende de cuatro variables de entorno de correo/enlace. |
| Registro de dispositivo | Alta de un comedero identificado por `feederId`. | Usa una API key compartida. |
| Asignación de comedero | Vincula un dispositivo existente a una cuenta y define nombre e icono. | La asociación se guarda como string en `userId`. |
| Gestión de comederos propios | Consulta individual y lista filtrada por propietario. | La ruta de listado total no filtra por propietario. |
| Personalización | Edición de `feederName` y `feederLogo`. | Solo se permite sobre un comedero asignado al usuario. |
| Orden lógica del motor | Persistencia de un booleano consultable por aplicación y firmware. | No incluye control directo de hardware. |
| Inicio/apagado temporizado | Inicio manual con historial y apagado automático de 40 segundos. | Los temporizadores viven en el proceso del backend. |
| Agenda puntual | Almacenamiento de fechas futuras no duplicadas. | Son instantes `Date`, no reglas recurrentes. |
| Polling del firmware | Lectura autenticada de `motorState`. | Es el mecanismo de entrega de órdenes al dispositivo. |
| Historial de alimentación | Eventos de encendido/apagado y adaptación a formato de UI. | La cantidad mostrada es fija en `20g`. |
| Consulta global de firmware | Entrega nombre, logo, cantidad y fecha de conexión. | También contiene una comprobación de agenda UTC. |
| Documentación de API | Swagger UI publicado por el servidor. | Algunas anotaciones no reflejan con exactitud todos los detalles del controlador. |

# 12. Funcionalidades incompletas o experimentales

- `FeederLog` está definido pero no es leído ni escrito por ninguna ruta o controlador.
- `clearFeederHistory` contiene lógica para borrar historial con comprobación de propiedad, pero no se exporta ni tiene endpoint.
- `stopMotorFromNodemcu` está implementada pero no se exporta; la única ruta que la invocaría (`/feeder/end`) está comentada.
- El scheduler intenta escribir valores incompatibles con la estructura de `feederHistory` y su lógica de control de ejecuciones trata los eventos como fechas.
- La coincidencia de horario existe tanto en el scheduler como en la consulta global del firmware, con criterios de zona horaria y efectos diferentes.
- La ruta de eliminación está comentada en el propio código como destinada solo a desarrollo y no valida propiedad.
- `feederQuantity` y el modelo `FeederLog` describen cantidades, pero no hay flujo expuesto que las actualice.

# 13. Observaciones finales

- La fuente ejecutable registra 22 endpoints. Las anotaciones Swagger y el README son documentación auxiliar; existen discrepancias puntuales, por ejemplo el README indica que `/feeder/start` no está implementado, pero su controlador sí cambia el estado, registra historial y programa apagado.
- Las rutas no tienen prefijo global: `/login`, `/feeders/...` y `/feeder/...` conviven en la raíz; Swagger se publica en `/api-docs`.
- El proceso intenta conectar a MongoDB antes de escuchar solicitudes; ante error de conexión termina con `process.exit(1)`.
- CORS se habilita sin restricción específica de orígenes.
- El campo definido por el esquema es `lastConection`; distintas operaciones escriben `lastConexion`, que no es el campo declarado.
- El envío de recuperación registra en consola los identificadores de EmailJS, destinatario y enlace de restablecimiento.
- Los `setTimeout` de apagado y el scheduler son estado en memoria del proceso: su ejecución depende de que ese proceso continúe activo.
- El historial presentado usa configuración regional `es-AR` y zona `America/Argentina/Buenos_Aires`, mientras una parte de la comprobación de agenda usa UTC.
