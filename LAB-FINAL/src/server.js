const http = require('http');
const ruta = require('path');
const sistemaArchivos = require('fs');
const { Server: ServidorSocket } = require('socket.io');

const PUERTO = process.env.PORT || 3000;
const DIRECTORIO_PUBLICO = ruta.join(__dirname, '..', 'public');

const servidorHttp = http.createServer((solicitud, respuesta) => {
  const urlSolicitada = new URL(solicitud.url, `http://${solicitud.headers.host}`);
  const rutaSolicitada = urlSolicitada.pathname === '/' ? '/index.html' : urlSolicitada.pathname;
  const rutaArchivo = ruta.normalize(ruta.join(DIRECTORIO_PUBLICO, rutaSolicitada));
  const raizPublica = `${DIRECTORIO_PUBLICO}${ruta.sep}`;

  if (rutaArchivo !== DIRECTORIO_PUBLICO && !rutaArchivo.startsWith(raizPublica)) {
    respuesta.writeHead(403);
    respuesta.end('Forbidden');
    return;
  }

  sistemaArchivos.readFile(rutaArchivo, (error, contenido) => {
    if (error) {
      respuesta.writeHead(404);
      respuesta.end('Not found');
      return;
    }

    respuesta.writeHead(200, { 'Content-Type': obtenerTipoContenido(rutaArchivo) });
    respuesta.end(contenido);
  });
});

const servidorSocket = new ServidorSocket(servidorHttp); // Crea un servidor de WebSocket utilizando Socket.IO
const salas = new Map();
const MAXIMA_LONGITUD_MENSAJE = 500;
let proximoIdCliente = 1;

// Maneja las conexiones de los clientes a través de Socket.IO
servidorSocket.on('connection', (conexionCliente) => {
  // Se invoca cuando un cliente intenta unirse a una sala de chat.
  conexionCliente.on('join', ({ nombreUsuario, codigoSala }) => {
    
    const cliente = {
      id: proximoIdCliente,
      nombre: nombreUsuario,
    };

    proximoIdCliente += 1;

    conexionCliente.join(codigoSala);
    conexionCliente.data.codigoSala = codigoSala;
    conexionCliente.data.cliente = cliente;

    if (!salas.has(codigoSala)) {
      salas.set(codigoSala, new Map());
    }

    salas.get(codigoSala).set(conexionCliente.id, cliente);

    // Envía un mensaje de bienvenida al cliente que se unió, y notifica a los demás clientes de la sala.
    conexionCliente.emit('welcome', { codigoSala: codigoSala, cliente });
    broadcastSistema(codigoSala, `${cliente.nombre} entró a la sala.`);
    broadcastCantUsuarios(codigoSala);
  });

  // Se invoca cuando un cliente envía un mensaje de chat.
  conexionCliente.on('chat', (datos) => {
    const codigoSala = conexionCliente.data.codigoSala;
    const cliente = conexionCliente.data.cliente;

    if (!codigoSala || !cliente) {
      return;
    }

    const texto = String(datos.texto || '').trim().slice(0, MAXIMA_LONGITUD_MENSAJE);

    if (!texto) {
      return;
    }

    servidorSocket.to(codigoSala).emit('chat', {
      cliente,
      texto,
      enviadoEn: new Date().toISOString(),
    });
  });

  // Se invoca cuando un cliente voluntariamente sale de la sala.
  conexionCliente.on('leave', () => {
    const codigoSala = conexionCliente.data.codigoSala;
    const cliente = conexionCliente.data.cliente;

    if (!codigoSala || !cliente) {
      return;
    }

    conexionCliente.leave(codigoSala);

    const sala = salas.get(codigoSala);

    if (sala) {
      sala.delete(conexionCliente.id);

      if (sala.size === 0) {
        salas.delete(codigoSala);
      } else {
        broadcastSistema(codigoSala, `${cliente.nombre} salió de la sala.`);
        broadcastCantUsuarios(codigoSala);
      }
    }

    conexionCliente.data.codigoSala = null;
    conexionCliente.data.cliente = null;
    conexionCliente.emit('left');
  });

  // Se invoca cuando un cliente se desconecta. Notifica a los demás clientes de la sala.
  conexionCliente.on('disconnect', () => {
    const codigoSala = conexionCliente.data.codigoSala;
    const cliente = conexionCliente.data.cliente;

    if (!codigoSala || !cliente) {
      return;
    }

    const sala = salas.get(codigoSala);

    if (sala) {
      sala.delete(conexionCliente.id);

      if (sala.size === 0) {
        salas.delete(codigoSala);
      } else {
        broadcastSistema(codigoSala, `${cliente.nombre} salió de la sala.`);
        broadcastCantUsuarios(codigoSala);
      }
    }
  });
});

servidorHttp.listen(PUERTO, () => {
  console.log(`Servidor listo en http://localhost:${PUERTO}`);
});

// Envía un mensaje del sistema a todos los clientes de una sala (por ejemplo, para notificar que un cliente entró o salió).
function broadcastSistema(codigoSala, texto) {
  servidorSocket.to(codigoSala).emit('system', {
    texto,
    enviadoEn: new Date().toISOString(),
  });
}

// Envía un mensaje de presencia a todos los clientes de una sala para notificar cuántos clientes hay conectados y quiénes son.
function broadcastCantUsuarios(codigoSala) {
  const sala = salas.get(codigoSala);

  if (!sala) {
    return;
  }

  servidorSocket.to(codigoSala).emit('presence', {
    codigoSala,
    cantidad: sala.size,
    clientes: [...sala.values()],
  });
}


function obtenerTipoContenido(rutaArchivo) {
  const extension = ruta.extname(rutaArchivo);

  const tipos = {
    '.html': 'text/html; charset=utf-8',
    '.css': 'text/css; charset=utf-8',
    '.js': 'application/javascript; charset=utf-8',
  };

  return tipos[extension] || 'application/octet-stream';
}
