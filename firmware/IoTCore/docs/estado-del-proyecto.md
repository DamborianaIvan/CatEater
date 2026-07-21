# Estado del proyecto CatEater

Este documento explica qu? est? implementado en el firmware y c?mo encajan las
partes. El proyecto usa PlatformIO con un NodeMCU v2 (ESP8266).

## Sprints

| Sprint | Objetivo | Estado |
| --- | --- | --- |
| 1 | Arquitectura, estructura y clases base | Completado |
| 2 | Alimentaci?n manual: usuario ? web ? motor ? comida | En progreso |
| 3 | Persistencia con LittleFS | Pendiente |
| 4 | Scheduler | Pendiente |
| 5 | Backend | Pendiente |
| 6 | Dashboard React | Pendiente |
| 7 | OTA | Pendiente |
| 8 | Notificaciones | Pendiente |

## Estructura de carpetas

```text
include/                         Declaraciones: qu? existe y qu? puede hacer.
??? app/                         Clase principal de la aplicaci?n.
??? application/                 Casos de uso.
??? domain/                      Modelos puros del negocio.
??? infrastructure/              Declaraciones de hardware y Arduino.
??? ports/                       Contratos entre l?gica y hardware.

src/                             Implementaciones: c?mo funciona cada clase.
??? app/
??? application/
??? infrastructure/
```

Un archivo `.h` de `include` declara una clase. Su archivo `.cpp` equivalente
en `src` contiene el c?digo que ejecuta esa clase.

Ejemplo:

```cpp
// include/application/FeedController.h
class FeedController {
 public:
  bool serve(const FeedRequest& request);
};
```

```cpp
// src/application/FeedController.cpp
bool FeedController::serve(const FeedRequest& request) {
  // Logica para validar y pedir alimento al motor.
}
```

## Arquitectura actual

```text
main.cpp
   ? crea los objetos
   ?
IoTCoreApplication
   ? coordina el ciclo de vida
   ?
FeedController
   ? valida una solicitud de porciones
   ?
IFeederActuator
   ? contrato comun
   ?
ULN2003FeederActuator
   ? controla el hardware
   ?
ULN2003 + motor 28BYJ-48
```

`IFeederActuator` evita que `FeedController` conozca detalles el?ctricos. As?,
si en el futuro se cambia el motor, solo cambia su adaptador de infraestructura.

## Motor y driver

| Elemento | Configuraci?n |
| --- | --- |
| Motor | 28BYJ-48, 5 V |
| Driver | ULN2003 |
| IN1 | D1 |
| IN2 | D2 |
| IN3 | D6 |
| IN4 | D7 |
| Librer?a | AccelStepper 1.64.0 |

Para el 28BYJ-48, el constructor usa el orden el?ctrico `IN1, IN3, IN2, IN4`.
Ese orden no cambia el cableado: solo le indica a la librer?a la secuencia
correcta de las bobinas.

## Movimiento no bloqueante

El motor usa `AccelStepper`. Cuando llega una orden, no gira todo de una vez:

1. `dispense(cantidad)` calcula `cantidad ? 512` pasos.
2. `motor_.move(pasos)` programa el objetivo.
3. Cada vuelta de `loop()` llama a `motor_.run()` mediante `update()`.
4. El motor avanza gradualmente sin detener el resto del firmware.
e
La constante actual es `512 pasos por porci?n`. Es un valor inicial d
calibraci?n: hay que dispensar una porci?n, pesar la comida y ajustar el valor
hasta obtener la cantidad deseada.
