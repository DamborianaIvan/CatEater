# CatFeeder Web

Frontend web productivo para CatFeeder, migrado conceptualmente desde la app Expo/React Native a React + TypeScript + Vite.

## Inicio

```bash
npm install
npm run dev
```

Copiá `.env.example` a un archivo `.env.local` si necesitás cambiar la URL. `VITE_API_URL` apunta por defecto al backend productivo.

## Scripts

- `npm run dev`: servidor de desarrollo.
- `npm run build`: comprobación TypeScript y build de producción.
- `npm run lint`: análisis estático.
- `npm run test`: suite de pruebas Vitest (preparada para ampliar).

## Arquitectura

- `src/services`: cliente HTTP centralizado, errores y servicios del contrato.
- `src/app`: sesión persistida y enrutado protegido.
- `src/pages`: autenticación, listado, vinculación, detalle, historial y horarios.
- `src/components`: layout y UI accesible reutilizable.

## Diferencias con la referencia móvil

La referencia usaba endpoints legacy para asignación, desvinculación, reset y horarios. Esta implementación consume el Swagger vigente: `/devices/pair`, `DELETE /devices/:feederId/pair`, `/reset-password` y `PUT /feeders/:feederId/config`. La configuración actual requiere exactamente cinco slots de horarios; la interfaz permite activarlos, editarlos y enviarlos de forma atómica.

La orden manual `POST /feeder/start` se presenta como una orden pendiente: no se informa que la alimentación terminó hasta que el estado del motor actualizado por el dispositivo lo confirme.
