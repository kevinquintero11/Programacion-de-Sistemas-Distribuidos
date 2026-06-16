const http = require('http');
const path = require('path');
const fs = require('fs');
const { Server } = require('socket.io');

const PORT = process.env.PORT || 3000;
const PUBLIC_DIR = path.join(__dirname, '..', 'public');

const server = http.createServer((req, res) => {
  const requestedUrl = new URL(req.url, `http://${req.headers.host}`);
  const requestedPath = requestedUrl.pathname === '/' ? '/index.html' : requestedUrl.pathname;
  const filePath = path.normalize(path.join(PUBLIC_DIR, requestedPath));
  const publicRoot = `${PUBLIC_DIR}${path.sep}`;

  if (filePath !== PUBLIC_DIR && !filePath.startsWith(publicRoot)) {
    res.writeHead(403);
    res.end('Forbidden');
    return;
  }

  fs.readFile(filePath, (error, content) => {
    if (error) {
      res.writeHead(404);
      res.end('Not found');
      return;
    }

    res.writeHead(200, { 'Content-Type': getContentType(filePath) });
    res.end(content);
  });
});

const io = new Server(server); // Crea un servidor de WebSocket utilizando Socket.IO
const rooms = new Map();
const MAX_MESSAGE_LENGTH = 500;
let nextClientId = 1;

// Maneja las conexiones de los clientes a través de Socket.IO
io.on('connection', (socket) => {
  // Se invoca cuando un cliente intenta unirse a una sala de chat.
  socket.on('join', ({ userName, roomCode }) => {
    const cleanRoom = sanitizeRoomCode(roomCode);
    const cleanName = sanitizeUserName(userName);

    if (!cleanRoom || !cleanName) {
      socket.emit('error', { text: 'Ingresá un código de sala y un nombre válidos.' });
      return;
    }

    const client = {
      id: nextClientId,
      name: cleanName,
    };

    nextClientId += 1;

    socket.join(cleanRoom);
    socket.data.roomCode = cleanRoom;
    socket.data.client = client;

    if (!rooms.has(cleanRoom)) {
      rooms.set(cleanRoom, new Map());
    }

    rooms.get(cleanRoom).set(socket.id, client);

    // Envía un mensaje de bienvenida al cliente que se unió, y notifica a los demás clientes de la sala.
    socket.emit('welcome', { roomCode: cleanRoom, client });
    broadcastSystem(cleanRoom, `${client.name} entró a la sala.`);
    broadcastPresence(cleanRoom);
  });

  // Se invoca cuando un cliente envía un mensaje de chat.
  socket.on('chat', (data) => {
    const roomCode = socket.data.roomCode;
    const client = socket.data.client;

    if (!roomCode || !client) {
      return;
    }

    const text = String(data.text || '').trim().slice(0, MAX_MESSAGE_LENGTH);

    if (!text) {
      return;
    }

    io.to(roomCode).emit('chat', {
      client,
      text,
      sentAt: new Date().toISOString(),
    });
  });

  // Se invoca cuando un cliente voluntariamente sale de la sala.
  socket.on('leave', () => {
    const roomCode = socket.data.roomCode;
    const client = socket.data.client;

    if (!roomCode || !client) {
      return;
    }

    socket.leave(roomCode);

    const room = rooms.get(roomCode);

    if (room) {
      room.delete(socket.id);

      if (room.size === 0) {
        rooms.delete(roomCode);
      } else {
        broadcastSystem(roomCode, `${client.name} salió de la sala.`);
        broadcastPresence(roomCode);
      }
    }

    socket.data.roomCode = null;
    socket.data.client = null;
    socket.emit('left');
  });

  // Se invoca cuando un cliente se desconecta. Notifica a los demás clientes de la sala.
  socket.on('disconnect', () => {
    const roomCode = socket.data.roomCode;
    const client = socket.data.client;

    if (!roomCode || !client) {
      return;
    }

    const room = rooms.get(roomCode);

    if (room) {
      room.delete(socket.id);

      if (room.size === 0) {
        rooms.delete(roomCode);
      } else {
        broadcastSystem(roomCode, `${client.name} salió de la sala.`);
        broadcastPresence(roomCode);
      }
    }
  });
});

server.listen(PORT, () => {
  console.log(`Servidor listo en http://localhost:${PORT}`);
});

// Envía un mensaje del sistema a todos los clientes de una sala (por ejemplo, para notificar que un cliente entró o salió).
function broadcastSystem(roomCode, text) {
  io.to(roomCode).emit('system', {
    text,
    sentAt: new Date().toISOString(),
  });
}

// Envía un mensaje de presencia a todos los clientes de una sala para notificar cuántos clientes hay conectados y quiénes son.
function broadcastPresence(roomCode) {
  const room = rooms.get(roomCode);

  if (!room) {
    return;
  }

  io.to(roomCode).emit('presence', {
    roomCode,
    count: room.size,
    clients: [...room.values()],
  });
}

// Sanitiza el código de sala ingresado por el usuario para evitar caracteres no válidos y limitar su longitud.
function sanitizeRoomCode(value) {
  return String(value || '')
    .trim()
    .toUpperCase()
    .replace(/[^A-Z0-9-]/g, '')
    .slice(0, 20);
}

// Sanitiza el nombre de usuario ingresado por el cliente para evitar caracteres no válidos y limitar su longitud.
function sanitizeUserName(value) {
  return String(value || '')
    .trim()
    .replace(/\s+/g, ' ')
    .slice(0, 30);
}

function getContentType(filePath) {
  const extension = path.extname(filePath);

  const types = {
    '.html': 'text/html; charset=utf-8',
    '.css': 'text/css; charset=utf-8',
    '.js': 'application/javascript; charset=utf-8',
  };

  return types[extension] || 'application/octet-stream';
}
