# Fase 1 finalizada: CatEater local

La primera fase entrega un comedero funcional en red local. El NodeMCU controla
el motor, conserva la calibracion y ofrece una interfaz web sin depender de un
backend ni de Internet.

## Alcance entregado

| Componente | Estado | Descripcion |
| --- | --- | --- |
| Firmware | Completado | Proyecto PlatformIO para NodeMCU v2 (ESP8266). |
| Motor | Completado | 28BYJ-48 de 5 V mediante driver ULN2003 y AccelStepper. |
| LittleFS | Completado | La calibracion se guarda en `/config.bin`. |
| API HTTP local | Completado | Rutas HTTP para servir y configurar el dispositivo. |
| Interfaz web local | Completado | Portal Wi-Fi para servir y calibrar porciones. |

## Hardware configurado

```text
NodeMCU     ULN2003
D1          IN1
D2          IN2
D6          IN3
D7          IN4
GND         GND comun con la fuente de 5 V
```

El motor debe tener una fuente de 5 V externa. El GND de esa fuente, del
ULN2003 y del NodeMCU debe estar conectado en comun.

## Red y portal local

Al iniciar, el dispositivo crea:

```text
Wi-Fi: CatEater
Contrasena: catfeeder
URL: http://192.168.4.1
```

La red es local y funciona aunque no haya Internet.

## API local

| Metodo | Ruta | Uso |
| --- | --- | --- |
| `GET` | `/` | Pantalla principal para servir alimento. |
| `POST` | `/servir` | Recibe `porciones` entre 1 y 10 y ordena el movimiento. |
| `GET` | `/calibrar` | Muestra la pantalla de calibracion. |
| `POST` | `/calibrar` | Recibe `pasos` entre 16 y 4096 y los guarda en LittleFS. |

## Calibracion

El valor inicial es 512 pasos por porcion. Para calibrarlo:

1. Entrar en `http://192.168.4.1/calibrar`.
2. Indicar un valor de pasos.
3. Guardar la configuracion.
4. Servir una porcion y pesar el alimento.
5. Repetir hasta obtener la cantidad deseada.

El valor se aplica de inmediato y persiste despues de reiniciar el dispositivo.

## Validacion final

La Fase 1 se considera aceptada si se cumplen estos puntos:

- El firmware compila y genera `firmware.bin`.
- El NodeMCU crea la red `CatEater`.
- La pagina `192.168.4.1` carga desde un celular o PC conectado.
- La orden de servir mueve el motor.
- El valor de calibracion puede cambiarse desde el navegador.
- Tras reiniciar, el valor de calibracion se conserva.

## Fuera de alcance de Fase 1

- Conexion a la red Wi-Fi domestica.
