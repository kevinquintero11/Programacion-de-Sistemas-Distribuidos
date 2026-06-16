const elementoEstado = document.querySelector('#status');
const formularioIngreso = document.querySelector('#joinForm');
const entradaNombreUsuario = document.querySelector('#userNameInput');
const entradaCodigoSala = document.querySelector('#roomCodeInput');
const elementoNombreCliente = document.querySelector('#clientName');
const elementoNombreSala = document.querySelector('#roomName');
const elementoPresencia = document.querySelector('#presence');
const elementoInfoChat = document.querySelector('.chat__info');
const elementoPanelUsuarios = document.querySelector('#usersPanel');
const elementoListaUsuarios = document.querySelector('#usersList');
const elementoMensajes = document.querySelector('#messages');
const formularioMensaje = document.querySelector('#messageForm');
const entradaMensaje = document.querySelector('#messageInput');
const botonEnviar = document.querySelector('#sendButton');
const botonSalir = document.querySelector('#leaveButton');

let clienteActual = null;
let conexionSocket = null;

// Se invoca cuando se toca el botón "Entrar" para unirse a una sala de chat.
formularioIngreso.addEventListener('submit', (evento) => {
  evento.preventDefault();

  if (conexionSocket && conexionSocket.connected) {
    return;
  }

  conectarASala({
    nombreUsuario: entradaNombreUsuario.value.trim(),
    codigoSala: entradaCodigoSala.value.trim(),
  });
});

// Se invoca cuando se toca el botón "Enviar" para enviar un mensaje de chat.
formularioMensaje.addEventListener('submit', (evento) => {
  evento.preventDefault();

  const texto = entradaMensaje.value.trim();

  if (!texto || !conexionSocket || !conexionSocket.connected) {
    return;
  }

  conexionSocket.emit('chat', { texto });
  entradaMensaje.value = '';
  entradaMensaje.focus();
});

// Se invoca cuando se toca el botón "Salir" para abandonar la sala.
botonSalir.addEventListener('click', () => {
  if (!conexionSocket || !conexionSocket.connected) {
    return;
  }

  conexionSocket.emit('leave');
  restablecerInterfaz();
});

// Conecta con el servidor y se une a la sala especificada por el usuario
function conectarASala({ nombreUsuario, codigoSala }) {
  if (!nombreUsuario || !codigoSala) {
    agregarMensajeSistema('Ingresá tu nombre y el código de sala.');
    return;
  }

  actualizarEstado('Conectando...', 'connecting');
  conexionSocket = io();

  conexionSocket.on('connect', () => {
    conexionSocket.emit('join', { nombreUsuario, codigoSala });
  });

  // Se invoca cuando el servidor acepta la unión a la sala.
  conexionSocket.on('welcome', (datos) => {
    clienteActual = datos.cliente;
    elementoNombreCliente.textContent = `Sos ${clienteActual.nombre}`;
    elementoNombreSala.textContent = `Sala ${datos.codigoSala}`;
    formularioIngreso.hidden = true;
    elementoInfoChat.hidden = false;
    elementoPanelUsuarios.hidden = false;
    actualizarEstado('Conectado', 'open');
    entradaMensaje.disabled = false;
    botonEnviar.disabled = false;
    entradaMensaje.focus();
  });

  // Se invoca cuando el servidor notifica un cambio en la presencia de la sala (un cliente entró o salió).
  conexionSocket.on('presence', (datos) => {
    elementoPresencia.textContent = `${datos.cantidad} cliente${datos.cantidad === 1 ? '' : 's'} conectado${datos.cantidad === 1 ? '' : 's'}`;
    renderizarUsuarios(datos.clientes);
  });

  // Se invoca cuando el servidor envía un mensaje de chat (puede ser un mensaje propio o de otro cliente).
  conexionSocket.on('chat', (datos) => {
    agregarMensajeChat(datos);
  });

  // Se invoca cuando el servidor envía un mensaje del sistema (por ejemplo, para notificar que un cliente entró o salió de la sala).
  conexionSocket.on('system', (datos) => {
    agregarMensajeSistema(datos.texto);
  });

  // Se invoca cuando el servidor notifica un error (por ejemplo, si no se pudo unir a la sala).
  conexionSocket.on('error', (datos) => {
    agregarMensajeSistema(datos.texto);
    restablecerConexionTrasIngresoFallido();
  });

  // Se invoca cuando el servidor notifica que el cliente salió de la sala.
  conexionSocket.on('left', () => {
    restablecerInterfaz();
  });

  // Se invoca cuando se pierde la conexión con el servidor.
  conexionSocket.on('disconnect', () => {
    actualizarEstado('Desconectado', 'closed');
    entradaMensaje.disabled = true;
    botonEnviar.disabled = true;
  });
}

// Agrega un mensaje de chat al panel de mensajes.
function agregarMensajeChat(mensaje) {
  const esMensajePropio = clienteActual?.id === mensaje.cliente.id;
  const elementoMensaje = document.createElement('article');
  elementoMensaje.className = `message${esMensajePropio ? ' message--own' : ''}`;

  const elementoMetadatos = document.createElement('span');
  elementoMetadatos.className = 'message__meta';
  elementoMetadatos.textContent = `${mensaje.cliente.nombre} · ${formatearHora(mensaje.enviadoEn)}`;

  const elementoTexto = document.createElement('span');
  elementoTexto.textContent = mensaje.texto;

  elementoMensaje.append(elementoMetadatos, elementoTexto);
  elementoMensajes.append(elementoMensaje);
  elementoMensajes.scrollTop = elementoMensajes.scrollHeight;
}

// Agrega un mensaje del sistema al panel de mensajes.
function agregarMensajeSistema(texto) {
  const elementoMensaje = document.createElement('article');
  elementoMensaje.className = 'message message--system';
  elementoMensaje.textContent = texto;
  elementoMensajes.append(elementoMensaje);
  elementoMensajes.scrollTop = elementoMensajes.scrollHeight;
}

// Actualiza el estado de la conexión (por ejemplo, "Conectando...", "Conectado", "Desconectado").
function actualizarEstado(texto, modificador) {
  elementoEstado.textContent = texto;
  elementoEstado.className = `status status--${modificador}`;
}

// Restablece el estado de la interfaz después de un error al intentar unirse a una sala (por ejemplo, si el código de sala no existe).
function restablecerConexionTrasIngresoFallido() {
  if (clienteActual) {
    return;
  }

  actualizarEstado('Error de conexión', 'error');

  if (conexionSocket) {
    conexionSocket.disconnect();
    conexionSocket = null;
  }
}

// Restablece la interfaz al estado inicial para poder ingresar a otra sala.
function restablecerInterfaz() {
  clienteActual = null;

  if (conexionSocket) {
    conexionSocket.disconnect();
    conexionSocket = null;
  }

  formularioIngreso.hidden = false;
  elementoInfoChat.hidden = true;
  elementoPanelUsuarios.hidden = true;
  entradaMensaje.disabled = true;
  botonEnviar.disabled = true;
  entradaMensaje.value = '';
  actualizarEstado('Desconectado', 'closed');
}

// Renderiza la lista de usuarios conectados a la sala.
function renderizarUsuarios(clientes) {
  elementoListaUsuarios.replaceChildren(
    ...clientes.map((cliente) => {
      const elementoItem = document.createElement('li');
      elementoItem.textContent = clienteActual?.id === cliente.id ? `${cliente.nombre} (vos)` : cliente.nombre;
      return elementoItem;
    }),
  );
}

// Formatea una fecha/hora para mostrarla junto a cada mensaje de chat.
function formatearHora(valor) {
  return new Intl.DateTimeFormat('es-AR', {
    hour: '2-digit',
    minute: '2-digit',
  }).format(new Date(valor));
}
