# Sprint 3: persistencia de configuracion

El firmware guarda la calibracion del dispensador en LittleFS, el sistema de
archivos interno del ESP8266. Esto permite conservarla cuando el dispositivo se
reinicia o pierde energia.

## Que se guarda

Por ahora se persiste un unico valor:

```text
stepsPerPortion = 512
```

Significa que una porcion mueve el motor 512 pasos. Es un valor inicial de
calibracion, no una cantidad de gramos fija.

## Flujo al iniciar

```text
setup()
  -> LittleFsConfigStorage.begin()
  -> intenta leer /config.bin
  -> si no existe, guarda la configuracion por defecto
  -> aplica stepsPerPortion al motor
```

El archivo esta en LittleFS como `/config.bin`. Incluye una firma interna para
evitar usar datos que no correspondan a la aplicacion.

## Archivos involucrados

| Archivo | Responsabilidad |
| --- | --- |
| `include/domain/DeviceConfig.h` | Modelo de configuracion. |
| `include/ports/IConfigStorage.h` | Contrato para guardar y leer configuracion. |
| `include/infrastructure/LittleFsConfigStorage.h` | Adaptador de LittleFS. |
| `src/infrastructure/LittleFsConfigStorage.cpp` | Implementacion de lectura y escritura. |
| `src/main.cpp` | Carga la configuracion al arrancar y se la entrega al motor. |

