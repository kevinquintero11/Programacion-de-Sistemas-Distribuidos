# Chat WebSocket con salas

Aplicación cliente-servidor con comunicación bidireccional usando WebSocket. El servidor empuja automáticamente los cambios al navegador, por eso el chat se actualiza sin refrescar la página.

Antes de entrar al chat, cada usuario ingresa su nombre y un código de sala. Solo reciben mensajes y presencia los usuarios conectados a la misma sala.

## Requisitos

- Node.js 18 o superior

## Ejecutar

```bash
npm install
npm start
```

Después abrí varias pestañas o navegadores en:

```text
http://localhost:3000
```

Usá el mismo código de sala para que varios usuarios entren al mismo chat. También podés abrir otra sala con otro código.

## Archivos principales

- `src/server.js`: servidor HTTP y WebSocket con salas y presencia.
- `public/index.html`: estructura del cliente web.
- `public/app.js`: ingreso a sala, conexión WebSocket y actualización del DOM sin refrescar.
- `public/styles.css`: estilos visuales del chat.
