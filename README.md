# CatEater

> **Nombre futuro del producto: The PetFeeder** 🐾

Sistema IoT de alimentación automática para mascotas, diseñado para funcionar de forma **local e independiente de Internet**, con la posibilidad de conectarse a un backend remoto para control, sincronización y monitoreo.

## 🎯 Objetivo

Construir un alimentador confiable que pueda:

- Alimentar automáticamente según horarios.
- Permitir alimentación manual mediante botón físico.
- Mantener su funcionamiento aunque no haya Internet.
- Registrar localmente las alimentaciones realizadas.
- Sincronizar los eventos cuando vuelva la conectividad.
- Permitir control y monitoreo remoto.
- Recibir actualizaciones de firmware mediante OTA.

> **Principio principal:** Internet mejora el dispositivo, pero nunca debe ser necesaria para alimentar a la mascota.

## 🏗️ Arquitectura

```text
┌────────────────────┐
│   The PetFeeder    │
│  ESP8266 Firmware  │
└─────────┬──────────┘
          │ WiFi / HTTP
          ▼
┌────────────────────┐
│      Backend       │
│ Node.js / Express  │
│     MongoDB        │
└─────────┬──────────┘
          │ API
          ▼
┌────────────────────┐
│ Web / Mobile App   │
└────────────────────┘
```

El dispositivo también cuenta con un **servidor web local**, por lo que puede seguir funcionando y siendo administrado desde la red local sin depender del backend.

## ⚙️ Firmware

Actualmente desarrollado sobre:

- C++ / Arduino Framework
- PlatformIO
- NodeMCU ESP8266
- Motor paso a paso + ULN2003
- AccelStepper
- ArduinoJson
- Almacenamiento persistente / LittleFS

El firmware contempla:

- Alimentación manual y programada.
- Scheduler local.
- Persistencia de configuración y eventos.
- Configuración WiFi.
- API/WebServer local.
- Identidad mediante `feederId`.
- Comunicación con backend.
- Heartbeat y estado remoto.
- Comandos remotos.
- Store-and-forward de eventos.
- Sincronización automática.
- Conectividad resiliente.
- OTA.

## 🌐 Funcionamiento offline

El dispositivo sigue una filosofía **Local First**.

```text
Internet disponible
        │
        ▼
 Control local + funciones remotas

Internet no disponible
        │
        ▼
 Control local + eventos almacenados
        │
        ▼
 Conectividad recuperada
        │
        ▼
 Sincronización automática
```

Las tareas de red no deben bloquear una alimentación. El funcionamiento físico del feeder tiene prioridad sobre cualquier operación remota.

## 🔐 Identidad

Cada dispositivo posee un `feederId` único y estable.

El backend relaciona ese feeder con un usuario mediante `userId`:

```text
userId ───── propietario ───── feederId
```

Esto permite administrar múltiples feeders dentro del mismo ecosistema.

## 🚀 Estado actual

**Firmware:** `1.1.3`  
**Hardware:** `REV-A`  
**Rama estable:** `main`

El proyecto se encuentra en desarrollo activo hacia una primera versión candidata a producción doméstica.

## 🗺️ Roadmap

Incrementos principales completados:

- ✅ Firmware base y alimentación local.
- ✅ Scheduler y persistencia.
- ✅ Conectividad con backend.
- ✅ Identidad y asignación de feeders.
- ✅ Heartbeat y estado remoto.
- ✅ Store-and-forward.
- ✅ Conectividad resiliente.
- ✅ OTA.

Próximas etapas: consolidación del control remoto, configuración y horarios remotos, seguridad, frontend completo y preparación para producción.

## 🔮 The PetFeeder

**CatEater** es el nombre actual del proyecto. El producto final se proyecta como **The PetFeeder**, con la visión de evolucionar hacia un dispositivo IoT doméstico producido a mayor escala, con hardware propio, firmware actualizable y soporte para múltiples feeders.

---

**CatEater / The PetFeeder** — Sistema IoT de alimentación automática para mascotas. 🐾