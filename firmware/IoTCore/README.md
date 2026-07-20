# CatEater IoT Core

Firmware para un dispensador de alimento basado en NodeMCU v2 (ESP8266). El
objetivo es que la base pueda reutilizarse en otros productos IoT cambiando los
adaptadores de hardware y las reglas de cada producto.

## Estado actual

### Sprint 1 ? Arquitectura

Completado y compilado con PlatformIO.

- Separaci?n entre dominio, aplicaci?n, puertos e infraestructura.
- Caso de uso `FeedController` que valida y coordina una solicitud de comida.
- Interfaz `IFeederActuator` para aislar el motor del resto del firmware.
- Logger por Serial a 115200 baud.

### Sprint 2 ? Alimentaci?n manual

Hardware definido, pendiente de incorporar el adaptador real y la p?gina web.

- Motor: 28BYJ-48 de 5 V.
- Driver: ULN2003.
- Cableado: IN1=D1, IN2=D2, IN3=D6, IN4=D7.
- Orden del motor paso a paso: IN1, IN3, IN2, IN4.
- Calibraci?n inicial propuesta: 512 pasos por porci?n. Debe ajustarse pesando
  el alimento dispensado.

## Estructura del proyecto

`include/` y `src/` reflejan las dos partes de una clase C++:

| Carpeta | Contiene | Ejemplo |
| --- | --- | --- |
| `include/` | Declaraciones: qu? ofrece una clase o contrato. Los archivos `.h` no ejecutan la l?gica. | `include/application/FeedController.h` |
| `src/` | Implementaciones: c?mo se comporta esa clase. Los `.cpp` se compilan y enlazan en el firmware. | `src/application/FeedController.cpp` |

Por eso aparecen carpetas con nombres repetidos. No son duplicados: una expone
la API y la otra implementa esa misma capa.

```text
include/                       src/
??? domain/                    ??? application/
??? ports/                     ??? app/
??? application/               ??? infrastructure/
??? app/
??? infrastructure/
```

Las responsabilidades son:

- `domain`: modelos puros del negocio, como una solicitud de alimentaci?n.
- `application`: casos de uso; decide qu? pasos ejecutar.
- `ports`: contratos para interactuar con motor, almacenamiento o servicios.
