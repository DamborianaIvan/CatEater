    # Auditoria de preparacion productiva - CatFeeder

    Fecha: 2026-08-14. Alcance: firmware actual ESP8266 NodeMCU v2. Esta auditoria no cambia la arquitectura ni implementa provisioning u OTA.

    ## Dictamen

    La separacion actual entre motor, alimentacion, scheduler, historial, red y API es una base razonable para congelar la arquitectura. El firmware no debe distribuirse todavia: provisioning, secretos, seguridad de endpoints, integridad de persistencia y OTA segura siguen bloqueando un primer producto real.

    No hace falta redisenar ni adoptar MQTT, WebSockets, FreeRTOS, ESP-IDF o ESP32. Tras resolver los bloqueantes de v1.4, se puede congelar la arquitectura y limitar cambios posteriores a bugs o requisitos concretos.

    ## Compilacion y flash

    `platformio run` finalizo correctamente el 2026-08-14.

    | Recurso | Uso actual | Limite |
    |---|---:|---:|
    | RAM | 34.960 B (42,7 %) | 81.920 B |
    | Firmware | 482.751 B (46,2 %) | 1.044.464 B |
    | Flash fisica | 4 MB | 4 MB |
    | LittleFS | ~1.000 KB | layout `4m1m` |

    El linker `eagle.flash.4m1m.ld` deja una ranura de sketch de 1.044.464 B, espacio temporal de ~2 MB y LittleFS de ~1 MB: permite OTA y el binario actual (486.896 B) cabe. No se cambio particion: hacerlo puede borrar/mover LittleFS y requiere migracion y prueba en hardware.

    ## Hallazgos

    | Clasificacion | Hallazgo | Impacto / accion |
    |---|---|---|
    | CRITICO | SSID/password y API key estan hardcodeados; API en HTTP plano. | Secretos expuestos por repositorio, serial y red. Rotarlos, retirarlos de codigo versionado y usar HTTPS con validacion de certificado/pinning antes de distribuir. |
    | CRITICO | `POST /feed` y `PUT /config` locales no tienen autenticacion. | Cualquier dispositivo de la LAN puede alimentar o alterar pasos/horarios. Deshabilitarlos fuera de una ventana fisica o protegerlos con credencial local. |
    | CRITICO | Historial y secuencia se reescriben con `w`. | Corte de energia puede truncar JSON/secuencia. Implementar archivo temporal + rename, backup/recuperacion y validacion. |
    | CRITICO | No hay maximo de `stepsPerFeed` ni `portions`. | Una entrada local/remota extrema puede causar sobrealimentacion o overflow. Definir limites mecanicos calibrados y validarlos en cada capa. |
    | IMPORTANTE | Orden remota en progreso y commandId activo viven solo en RAM. | Reinicio conserva el baseline y no repite de inmediato, pero deja comando sin confirmacion. Persistir estado o acordar expiracion/idempotencia backend. |
    | IMPORTANTE | EEPROM tiene firma solamente, sin version/longitud/CRC. | Corrupcion con valores plausibles se acepta; no hay migracion. Usar registro versionado con CRC y dos copias. |
    | IMPORTANTE | `/config` cambia motor/RAM antes de confirmar EEPROM. | Si falla `EEPROM.commit`, estado vivo y persistido divergen. Guardar primero o restaurar estado previo. |
    | IMPORTANTE | Evento se guarda al iniciar movimiento, no al finalizar. | Tras corte/reinicio no se sabe porcion real. Sin sensor solo puede informarse `started/completed/interrupted`. |
    | IMPORTANTE | JSON corrupto o LittleFS sin montar no tiene recuperacion. | Archivo ausente se trata como vacio; JSON corrupto oculta la cola. No formatear automaticamente: conservar y abrir cola nueva recuperable. |
    | IMPORTANTE | No existe provisioning; WiFi reintenta cada 5 s con datos compilados. | Equipo nuevo o red cambiada no es recuperable por usuario. |
    | IMPORTANTE | No hay tests automatizados ni matriz de power-cut/reinicio. | Invariantes de alimentacion e idempotencia quedan sin proteccion. |
    | MEJORA | Arranque imprime hasta 100 eventos y logs SSID/MAC/IP. | Ruido, arranque mas lento y fuga de diagnostico. Registrar resumen y no secretos. |
    | MEJORA | `HeartbetService` conserva typo en archivo/include. | Mantenimiento; corregir aisladamente. |
    | MEJORA | Hora depende enteramente de NTP. | Sin Internet tras reinicio, scheduler se inhibe de forma segura. Definir mas adelante reloj degradado persistente. |
    | PRODUCTIZACION | Version/modelo existen, pero registro/heartbeat no los envian. | Backend no puede conocer firmware ni hardware activos. |
    | PRODUCTIZACION | `feederId` es `ESP-<chipId>`. | Estable por reinicio y suficiente como ID tecnico inicial; no es claim code comercial. Asociar a MAC/chipId y usar claim aleatorio de un uso en backend. |
    | PRODUCTIZACION | No existe OTA. | Faltan manifest, descarga autenticada, verificacion, reporte de estado y politica de fallo. |
    | FUTURO | Rollback A/B robusto. | ESP8266 Arduino no da rollback robusto equivalente a ESP32; evaluar boot-health o cambio de hardware despues. |

    ## Offline, loop y reinicios

    - Boton y scheduler no requieren WiFi/API. Sync, heartbeat y estado remoto no hacen HTTP sin WiFi. Timeouts: 500 ms estado remoto y 1.000 ms sync/heartbeat/confirmacion.
    - Sync deja eventos pendientes hasta HTTP 2xx y corta tras el primer fallo. Backend debe deduplicar por `(feederId,eventId)` porque un corte entre POST y marca local reenvia el evento.
    - Motor rechaza alimentacion concurrente. Scheduler marca ejecucion solo si fue aceptada y evita duplicado dentro del minuto, incluido 00:00.
    - HTTPClient es sincrono; cada request sigue bloqueando el loop durante su timeout. La suma de llamadas puede demorar boton/motor, aunque esta acotada.
    - Reinicio sin WiFi/API: boton sigue operativo; scheduler espera NTP; eventos quedan pendientes. Reinicio durante motor no inicia una alimentacion solo, pero evento puede representar porcion parcial. Reinicio durante sync puede reintentar. Reinicio durante comando remoto no confirma resultado.
    - Riesgo pendiente: la marca anti-duplicado del scheduler es RAM. Si reinicia dentro del minuto y el reloj esta disponible, puede ejecutar de nuevo. Persistir clave `YYYYMMDDHHmm + scheduleIndex` es la solucion preferible.

    No hay sensores de atasco, peso o nivel. El firmware solo sabe que termino la orden de pasos; no debe afirmar entrega fisica ni inventar deteccion sin nuevo hardware.

    ## Provisioning WiFi recomendado

    Implementar un `ProvisioningService` pequeno y aislado:

    1. Credenciales persistidas fuera del firmware compilado, con schema/checksum.
    2. Sin credenciales, o tras timeout sin asociar: AP + portal cautivo para scan, ingreso y prueba de WiFi. WiFiManager es compatible y maduro; una pagina minima sobre el servidor existente evita otra UI/dependencia.
    3. Guardar solo despues de asociar; timeout del portal para no bloquear alimentacion local.
    4. Gesto fisico al boot de 5-10 s para borrar solo WiFi e iniciar portal. No borrar historial, configuracion ni identidad.
    5. AP con password unico por dispositivo derivado/impreso en etiqueta; nunca abierto permanente.

    Decidir antes UX, nombre/password de AP, gesto, timeout y almacenamiento. No implementar WiFiManager sin esas decisiones.

    ## OTA necesario

    El layout actual ya soporta OTA y no requiere cambio de particiones. Para OTA remoto faltan: version/build/revision de hardware, manifest, consulta de disponibilidad, descarga HTTPS autenticada, SHA-256 y firma, comprobacion de tamano, exclusclusion mientras motor alimenta, reinicio y health report.

    Manifest minimo:

    ```json
    {"version":"1.4.0","build":"ci-id","model":"CatFeeder","hardwareRevision":"revA","url":"https://.../firmware.bin","size":486896,"sha256":"...","signature":"...","mandatory":false}
    ```

    No descargar por HTTP ni aceptar certificados inseguros. No actualizar durante alimentacion ni sin alimentacion electrica estable. Rollback fuerte queda para futuro; reportar health/result tras reboot es el minimo v1.

    ## Contrato firmware-backend

    IDs son cadenas opacas e idempotentes. `feederId` identifica producto; `deviceId` actual lo implementa con chipId; `eventId` identifica evento monotono por feeder; `commandId` identifica una orden inmutable. Claim code solo asocia producto existente a usuario; no reemplaza `feederId`.

    | Operacion | Request / respuesta necesaria |
    |---|---|
    | `POST /feeders/register` | Actual: `{feederId,feederName}`. Agregar `firmwareVersion`, `build`, `hardwareRevision`, `macAddress`. 201, 409, 400, 401/403. |
    | `GET /feeder/motor-state/{feederId}` | 200 `{motorState,portions,commandId}`; `portions` limitado, commandId unico. 401/403/404/409 definidos. |
    | `POST /feeder/complete` | `{feederId,commandId,result}`; 2xx idempotente, incluso si ya completo. Result: completed/interrupted/unknown. |
    | `POST /feeders/history` | `{eventId,feederId,timestamp,portions,source}`; unicidad `(feederId,eventId)`, 2xx para duplicado, 4xx no reintentable, 5xx/red reintentable. |
    | `POST /feeders/heartbeat` | Agregar version/build/hardware, timestamp, RSSI, WiFi/Internet, motorState, eventos pendientes y ultimo error. |

    Futuro desired state: `GET /feeders/{feederId}/desired-state` con `revision`, schedules y comandos. Firmware valida, persiste, aplica y confirma `appliedRevision`. HTTP REST/polling es suficiente.

    ## Plan

    ### v1.4, antes de distribuir/congelar

    1. Rotar secretos y sacarlos de codigo.
    2. Provisioning WiFi con recuperacion fisica segura.
    3. Autenticar/cerrar control web local.
    4. Definir y aplicar limites mecanicos globales.
    5. Persistencia atomica/recovery de LittleFS y CRC/version EEPROM.
    6. Resolver reinicio en minuto programado y comando remoto interrumpido.
    7. Enviar version/build/hardware/conectividad a backend.
    8. Pruebas de WiFi/API caida, credenciales incorrectas, power-cut en historial/sync/motor y reinicios.

    ### Futuras versiones

    - OTA remoto completo con manifest firmado y health report.
    - Claim, usuarios, autenticacion por dispositivo y UI.
    - Sensores de atasco/nivel/peso si hay hardware.
    - Reloj degradado persistente y rollback robusto.

    ## Cambios realizados

    No se modifico firmware ni PlatformIO, no se realizaron commits. Este informe es el unico archivo creado por la auditoria.
