# CatEater

> **Nombre futuro del producto: The PetFeeder**

Sistema IoT para alimentación automática de mascotas, diseñado desde el inicio con una arquitectura modular, funcionamiento local independiente de Internet y capacidad de integración con un backend remoto.

El proyecto comenzó bajo el nombre **CatEater** y evolucionará hacia **The PetFeeder** como nombre del producto final, ampliando su alcance desde un alimentador orientado a gatos hacia una plataforma de alimentación automática para mascotas.

---

## 📌 Estado actual

**Rama estable:** `main`

**Firmware actual:** `1.1.3`

**Hardware actual:** NodeMCU ESP8266 — `REV-A`

Los incrementos completados hasta el momento cubren:

- Firmware base funcional.
- Alimentación manual y programada.
- Configuración local persistente.
- WiFi y provisioning.
- Comunicación con backend remoto.
- Registro automático del dispositivo.
- Identidad mediante `feederId`.
- Asociación `feederId → userId`.
- Heartbeat.
- Estado remoto.
- Comandos remotos.
- Persistencia local de eventos.
- Store-and-forward cuando no existe conectividad.
- Backoff ante indisponibilidad del backend.
- Estado local de conectividad.
- OTA.

El proyecto se encuentra en desarrollo activo hacia una primera versión candidata a producción doméstica.

---

# 🎯 Objetivo del proyecto

Construir un alimentador automático para mascotas que pueda operar de forma confiable tanto **localmente** como conectado a un ecosistema remoto.

El principio fundamental del proyecto es:

> **Internet mejora el dispositivo, pero nunca debe ser necesaria para alimentar a la mascota.**

Por lo tanto, las funciones críticas deben continuar funcionando aunque:

- no exista conexión WiFi;
- no exista conexión a Internet;
- el backend esté caído;
- exista una interrupción temporal de red.

La conectividad remota se considera una capa complementaria para sincronización, control remoto, configuración, monitoreo y futuras funcionalidades.

---

# 🏗️ Arquitectura general

El ecosistema está pensado en tres grandes componentes:

```text
┌──────────────────────┐
│      The PetFeeder   │
│       Firmware       │
│                      │
│  NodeMCU / ESP8266   │
└──────────┬───────────┘
           │
           │ HTTP / WiFi
           │
           ▼
┌──────────────────────┐
│       Backend        │
│                      │
│   Node.js / Express  │
│        MongoDB       │
└──────────┬───────────┘
           │
           │ API
           ▼
┌──────────────────────┐
│       Frontend       │
│                      │
│ Web / Mobile Client  │
└──────────────────────┘
```

El firmware también dispone de un **WebServer local**, por lo que puede seguir siendo administrado directamente desde la red local sin depender del backend.

---

# 🧠 Principios de diseño

## Local First

El dispositivo debe ser funcional sin Internet.

Funciones críticas como:

- alimentación manual;
- alimentación programada;
- scheduler;
- configuración local;
- persistencia de configuración;
- registro local de eventos;

no deben depender del backend.

## Remote as a layer

Las funciones remotas son complementarias:

- sincronización;
- heartbeat;
- comandos remotos;
- configuración remota;
- monitoreo;
- futuras actualizaciones OTA.

## Store-and-forward

Cuando no existe conectividad, los eventos se almacenan localmente y se sincronizan posteriormente.

```text
Evento local
    │
    ├── Backend disponible → sincronizar
    │
    └── Backend no disponible
              │
              ▼
        guardar localmente
              │
              ▼
        esperar conexión
              │
              ▼
          sincronizar
```

## La red nunca bloquea la alimentación

Las tareas de red no deben ejecutarse mientras el motor está realizando una alimentación.

Esto evita que un timeout HTTP o una indisponibilidad del backend pueda afectar el funcionamiento físico del dispositivo.

---

# 📁 Estructura del repositorio

```text
CatEater/
│
├── firmware/
│   └── IoTCater/
│       ├── include/
│       │   ├── config/
│       │   ├── device/
│       │   ├── domain/
│       │   ├── hardware/
│       │   ├── network/
│       │   ├── services/
│       │   ├── storage/
│       │   └── web/
│       │
│       ├── src/
│       │   ├── device/
│       │   ├── domain/
│       │   ├── hardware/
│       │   ├── network/
│       │   ├── services/
│       │   ├── storage/
│       │   ├── web/
│       │   └── main.cpp
│       │
│       └── platformio.ini
│
├── backend/
│   └── ...
│
└── README.md
```

La estructura puede evolucionar a medida que el backend y el frontend se incorporen formalmente al repositorio, pero el objetivo es mantener responsabilidades separadas y evitar dependencias innecesarias entre capas.

---

# ⚙️ Firmware

El firmware está desarrollado en **C++ sobre Arduino**, utilizando PlatformIO.

## Hardware actual

- NodeMCU ESP8266.
- Driver ULN2003.
- Motor paso a paso.
- Botón físico para alimentación manual.
- Almacenamiento local mediante memoria persistente/LittleFS según el componente.

La revisión actual del hardware está definida como:

```text
REV-A
```

La revisión se centraliza en `Config.h` junto con la versión del firmware. citeturn16file0

---

# 🧩 Componentes principales del firmware

## Motor

Responsable del control físico del motor paso a paso.

El motor se mantiene aislado de las capas de red y backend.

## FeedingService

Centraliza la operación de alimentación y registra el evento correspondiente.

Las fuentes de alimentación contempladas incluyen:

- física;
- programada;
- remota.

## Scheduler

Ejecuta las alimentaciones configuradas localmente.

El scheduler continúa funcionando independientemente del estado de Internet.

## TimeService

Proporciona la referencia temporal utilizada por el scheduler y los eventos.

## WiFiService

Gestiona la conexión WiFi y el estado de conectividad del ESP8266.

## ProvisioningService

Permite configurar/provisionar la conectividad WiFi sin compilar las credenciales dentro del firmware.

Las credenciales WiFi son proporcionadas por el usuario y no forman parte del firmware compilado. citeturn16file0

## DeviceInfo

Proporciona información de identidad y diagnóstico del dispositivo, incluyendo el identificador utilizado como `feederId`, firmware, modelo, fabricante, chip, MAC, IP, RSSI y memoria disponible.

La identidad del dispositivo se utiliza como identidad del feeder dentro del backend.

## ApiClient

Centraliza la comunicación entre el firmware y el backend.

Actualmente gestiona operaciones como:

- registro del feeder;
- consulta de información;
- heartbeat;
- estado remoto del motor;
- configuración remota;
- confirmación de comandos;
- sincronización de eventos.

## HttpClient

Abstrae las peticiones HTTP utilizadas por `ApiClient`.

## BackendConnectionService

Controla la disponibilidad conocida del backend y aplica backoff progresivo ante fallos de comunicación.

Actualmente utiliza:

```text
Backoff inicial: 10 segundos
Backoff máximo:  120 segundos
```

La lógica está implementada como un servicio independiente. citeturn14file0

## HeartbeatService

Envía periódicamente una señal al backend para informar que el feeder continúa conectado.

## RemoteStateService

Consulta y procesa el estado/comandos remotos, incluyendo la ejecución y confirmación de comandos.

## ConfigurationSyncService

Gestiona la sincronización de la configuración remota y sus revisiones.

## FeedingHistoryService

Mantiene el historial local de alimentaciones y permite conservar eventos pendientes de sincronización.

## SyncService

Implementa el mecanismo store-and-forward:

```text
Evento local
    ↓
Persistencia
    ↓
Pending
    ↓
Backend disponible
    ↓
Sincronización
    ↓
Marcado como synced
```

## ConfigurationStorage

Persiste la configuración necesaria para que el dispositivo pueda continuar operando después de reinicios.

## RemoteCommandStorage

Persiste comandos remotos pendientes para evitar perderlos ante reinicios o interrupciones.

## WebServer

Expone una interfaz HTTP local para administrar y consultar el dispositivo desde la red local.

Actualmente incluye endpoints relacionados con:

- alimentación manual;
- estado;
- configuración;
- actualización OTA.

## OtaService

Gestiona la actualización del firmware mediante OTA.

OTA es un requisito previo para la futura versión productiva del dispositivo.

---

# 🌐 Estado local del dispositivo

El endpoint local `/status` permite consultar información operacional del dispositivo.

Conceptualmente devuelve información como:

```json
{
  "feeding": false,
  "wifiConnected": true,
  "backendAvailable": true,
  "ipAddress": "192.168.1.100"
}
```

Los estados se mantienen separados deliberadamente:

```text
feeding
wifiConnected
backendAvailable
```

Tener WiFi no significa necesariamente que el backend esté disponible.

---

# 🔐 Identidad y ownership

El modelo actual utiliza dos conceptos principales:

```text
feederId
userId
```

## feederId

Identifica al CatFeeder/dispositivo.

Actualmente se genera a partir del identificador único del ESP8266 y se mantiene estable entre reinicios.

Ejemplo:

```text
ESP-A1B2C3
```

## userId

Identifica al usuario propietario del feeder dentro del backend.

La relación es:

```text
userId ───── owns ───── feederId
```

Un dispositivo puede registrarse inicialmente sin estar asignado a un usuario.

```text
feederId: ESP-A1B2C3
userId: null
feederAsign: false
```

Después de ser reclamado por un usuario:

```text
feederId: ESP-A1B2C3
userId: USER-123
feederAsign: true
```

Esto permite separar claramente la identidad física del dispositivo de la cuenta que lo administra.

---

# 🔄 Registro y asignación

El flujo actual es:

```text
NodeMCU
   │
   │ genera feederId
   ▼
POST /feeders/register
   │
   ▼
Backend
   │
   ▼
Feeder registrado
   │
   ├── feederId
   ├── userId = null
   └── feederAsign = false
        │
        │ usuario autenticado
        ▼
POST /feeder/assign
        │
        ▼
Feeder asignado
   │
   ├── feederId
   ├── userId
   └── feederAsign = true
```

Este flujo fue validado utilizando el dispositivo físico y verificando posteriormente el estado en MongoDB.

---

# 📡 Conectividad resiliente

La conectividad remota está diseñada para ser tolerante a fallos.

El firmware diferencia entre:

```text
WiFi conectado
```

y:

```text
Backend disponible
```

Ante un fallo de comunicación, `BackendConnectionService` aplica backoff progresivo para evitar realizar peticiones continuamente contra un backend caído. citeturn14file0

La alimentación local no depende de esta capa.

---

# 💾 Store-and-forward

Los eventos de alimentación se registran localmente.

Si el backend no está disponible:

```text
Feeding
   ↓
Evento generado
   ↓
Persistencia local
   ↓
Pending
```

Cuando vuelve la conectividad:

```text
Pending
   ↓
SyncService
   ↓
POST al backend
   ↓
Confirmación
   ↓
Evento sincronizado
```

Este comportamiento permite que el historial no dependa de una conexión permanente a Internet.

---

# 🛡️ Regla crítica de ejecución

Una regla arquitectónica fundamental del firmware es:

> **Las operaciones de red no deben bloquear una alimentación.**

El loop principal ejecuta las tareas de red solamente cuando el motor no está alimentando.

```text
Motor alimentando
       │
       ├── Scheduler / motor → prioridad
       │
       └── HTTP / heartbeat / sync → esperar
```

Esto evita que un timeout o una respuesta lenta del backend pueda afectar el movimiento físico del motor.

---

# 🛠️ Tecnologías

## Firmware

- C++
- Arduino Framework
- PlatformIO
- ESP8266 / NodeMCU
- AccelStepper
- ArduinoJson
- WiFi
- HTTP
- LittleFS / almacenamiento persistente

Las dependencias principales de PlatformIO están declaradas en `platformio.ini`. citeturn12file0

## Backend

La arquitectura contempla:

- Node.js
- Express.js
- MongoDB
- Mongoose
- API REST

## Frontend

El ecosistema está pensado para incorporar una aplicación web/mobile para:

- autenticación;
- visualización de feeders;
- asignación de dispositivos;
- alimentación remota;
- configuración;
- horarios;
- historial;
- estadísticas;
- monitoreo.

---

# 🚀 Configuración del firmware

## Requisitos

Instalar:

- Visual Studio Code.
- PlatformIO.
- Driver correspondiente para el NodeMCU/USB si el sistema lo requiere.

## Compilación

Desde la carpeta del firmware:

```bash
pio run
```

## Upload

```bash
pio run --target upload
```

## Monitor serial

```bash
pio device monitor
```

Velocidad configurada:

```text
115200 baud
```

La configuración actual utiliza el entorno `nodemcuv2`, plataforma ESP8266 y framework Arduino. citeturn12file0

---

# 📋 Configuración de firmware

La información global del firmware se encuentra centralizada en `Config.h`.

Actualmente incluye:

```cpp
#define FIRMWARE_VERSION "1.1.3"
#define HARDWARE_REVISION "REV-A"
```

La identidad comercial y técnica se encuentra en `Firmware.h`. citeturn16file0 citeturn15file0

Las credenciales WiFi no se almacenan dentro del código fuente.

---

# 🧪 Filosofía de pruebas

Cada incremento debe validarse en dos niveles:

### 1. Validación de software

- Compilación limpia.
- Sin warnings.
- Validación de comportamiento esperado.
- Revisión de integración.

### 2. Validación sobre hardware real

Cuando el cambio afecta conectividad, motor, persistencia o comunicación remota, debe validarse sobre el NodeMCU físico.

Ejemplos de escenarios importantes:

- reinicio del dispositivo;
- pérdida de WiFi;
- pérdida de backend;
- recuperación de conectividad;
- alimentación durante indisponibilidad de red;
- persistencia después de reinicio;
- sincronización posterior.

La estabilidad del firmware tiene prioridad sobre la velocidad de incorporación de funcionalidades.

---

# 🗺️ Roadmap

El proyecto se desarrolla mediante incrementos pequeños y verificables.

## Incremento 1 — Firmware base

**Estado: ✅ Completado**

Base funcional del alimentador:

- motor;
- alimentación;
- scheduler;
- WebServer local;
- persistencia;
- API local;
- configuración WiFi.

## Incremento 2 — Integración de conectividad

**Estado: ✅ Completado**

Integración inicial con el ecosistema remoto:

- HTTP client;
- API client;
- registro;
- heartbeat;
- estado remoto;
- sincronización;
- comandos remotos.

## Incremento 3 — Identidad y asignación

**Estado: ✅ Completado**

- `feederId` estable.
- Registro automático del NodeMCU.
- Modelo `Feeder`.
- Relación `feederId → userId`.
- Asignación de feeder.
- Estado `feederAsign`.
- Validación real contra MongoDB.

## Incremento 4 — Conectividad resiliente

**Estado: ✅ Completado**

- detección de disponibilidad del backend;
- backoff progresivo;
- store-and-forward;
- recuperación automática;
- estado `backendAvailable`;
- protección del motor frente a tareas de red;
- validación offline/online sobre hardware real.

## Próximos incrementos

Los próximos incrementos estarán orientados a consolidar:

- control remoto;
- sincronización de configuración;
- horarios remotos;
- historial remoto;
- autenticación y seguridad del dispositivo;
- OTA productivo;
- monitoreo;
- frontend completo;
- preparación para múltiples dispositivos;
- hardening para producción.

El roadmap puede cambiar a medida que las pruebas del dispositivo revelen nuevos requisitos.

---

# 📦 Objetivo de la v1.3

La versión **v1.3** tiene como objetivo convertirse en una primera versión candidata a producción doméstica.

Los requisitos fundamentales son:

- funcionar sin Internet;
- alimentación programada local;
- botón físico para alimentación manual;
- persistencia local de eventos;
- sincronización posterior con backend;
- comunicación remota confiable;
- OTA;
- firmware estable y conservador.

La WebApp y el backend pueden evolucionar independientemente siempre que se mantenga el contrato con el firmware.

---

# 🔮 Visión de producto — The PetFeeder

CatEater es el nombre actual del proyecto, pero el producto final está pensado como **The PetFeeder**.

La visión es evolucionar desde un prototipo funcional hacia un dispositivo IoT doméstico comercializable.

La evolución esperada incluye:

```text
CatEater
   ↓
Firmware estable
   ↓
The PetFeeder
   ↓
Hardware propio
   ↓
PCB propia
   ↓
Firmware común para múltiples dispositivos
   ↓
OTA
   ↓
Backend escalable
   ↓
Frontend / App
   ↓
Producción
```

El hardware y firmware deben diseñarse progresivamente para que múltiples unidades puedan utilizar la misma base de software, diferenciándose mediante su identidad individual (`feederId`).

---

# 🌐 Escalabilidad futura

La arquitectura está siendo preparada para soportar múltiples feeders.

Conceptualmente:

```text
Usuario A
├── Feeder 001
├── Feeder 002
└── Feeder 003

Usuario B
├── Feeder 004
└── Feeder 005
```

Cada dispositivo mantiene su propio `feederId`, mientras que el backend administra la relación con los usuarios.

Esto permitirá que el mismo firmware pueda utilizarse en múltiples dispositivos físicos sin modificar la lógica específica del usuario.

---

# 🔐 Seguridad

La seguridad se irá incorporando progresivamente.

Actualmente existe comunicación autenticada entre firmware y backend mediante las credenciales previstas por la arquitectura actual.

Las siguientes etapas deberán considerar especialmente:

- protección de credenciales;
- autenticación robusta del dispositivo;
- autorización por `feederId`;
- protección contra comandos no autorizados;
- rotación de credenciales;
- HTTPS/TLS cuando la infraestructura lo permita;
- seguridad del proceso OTA.

La seguridad debe incorporarse sin comprometer el principio Local First.

---

# 🔄 Estrategia de Git

El repositorio utiliza `main` como rama estable.

Para cada incremento se crea una rama temporal:

```text
main
  │
  └── feature/increment-X-nombre
```

El flujo recomendado es:

```text
Crear rama
   ↓
Desarrollar
   ↓
Compilar
   ↓
Probar hardware
   ↓
Commit
   ↓
Pull Request
   ↓
Merge a main
   ↓
Eliminar rama feature
```

Las ramas de incrementos completados se eliminan después del merge. El historial de commits permanece en `main`.

Actualmente se mantiene solamente `main` como rama permanente.

---

# 📝 Convenciones de commits

Los commits deben ser claros y descriptivos.

Formato recomendado:

```text
<tipo>: <descripcion>
```

Ejemplos:

```text
feat: implementar conectividad resiliente
fix: corregir bloqueo durante alimentacion
refactor: separar servicio de sincronizacion
chore: actualizar configuracion de firmware
docs: actualizar documentacion del proyecto
```

Los comentarios, commits, README y documentación del proyecto se mantienen en **español**.

El código y las APIs mantienen las convenciones técnicas correspondientes al lenguaje y al contrato definido.

---

# ⚠️ Consideraciones importantes de desarrollo

Antes de introducir cambios en el firmware:

1. No bloquear el loop principal innecesariamente.
2. No hacer que Internet sea requisito para alimentar.
3. Evitar llamadas HTTP largas durante operaciones críticas.
4. Mantener persistencia local para datos importantes.
5. Validar cambios sobre hardware cuando correspondan.
6. Mantener las responsabilidades separadas.
7. Evitar refactors grandes si un cambio pequeño resuelve el problema.
8. Mantener `main` siempre estable.
9. Trabajar por incrementos aislados.
10. No agregar complejidad futura antes de que exista una necesidad real.

---

# 📈 Estado de madurez

El proyecto pasó de un firmware local experimental a una arquitectura IoT con:

```text
Motor
  ↓
Feeding
  ↓
Scheduler
  ↓
Persistencia
  ↓
WiFi
  ↓
Backend
  ↓
Identidad
  ↓
Ownership
  ↓
Heartbeat
  ↓
Remote State
  ↓
Store-and-forward
  ↓
Resiliencia
  ↓
OTA
```

El objetivo inmediato no es agregar funcionalidades indiscriminadamente, sino **consolidar una base estable sobre la cual construir The PetFeeder**.

---

# 👨‍💻 Proyecto

**CatEater / The PetFeeder**

Sistema IoT de alimentación automática para mascotas.

Estado: **desarrollo activo** 🚧

Nombre futuro del producto: **The PetFeeder** 🐾
