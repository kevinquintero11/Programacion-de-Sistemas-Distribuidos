const statusElement = document.querySelector('#status');
const joinForm = document.querySelector('#joinForm');
const userNameInput = document.querySelector('#userNameInput');
const roomCodeInput = document.querySelector('#roomCodeInput');
const clientNameElement = document.querySelector('#clientName');
const roomNameElement = document.querySelector('#roomName');
const presenceElement = document.querySelector('#presence');
const chatInfoElement = document.querySelector('.chat__info');
const usersPanelElement = document.querySelector('#usersPanel');
const usersListElement = document.querySelector('#usersList');
const messagesElement = document.querySelector('#messages');
const messageForm = document.querySelector('#messageForm');
const messageInput = document.querySelector('#messageInput');
const sendButton = document.querySelector('#sendButton');
const leaveButton = document.querySelector('#leaveButton');

let currentClient = null;
let socket = null;

// Se invoca cuando se toca el botón "Entrar" para unirse a una sala de chat.
joinForm.addEventListener('submit', (event) => {
  event.preventDefault();

  if (socket && socket.connected) {
    return;
  }

  connectToRoom({
    userName: userNameInput.value.trim(),
    roomCode: roomCodeInput.value.trim(),
  });
});

// Se invoca cuando se toca el botón "Enviar" para enviar un mensaje de chat.
messageForm.addEventListener('submit', (event) => {
  event.preventDefault();

  const text = messageInput.value.trim();

  if (!text || !socket || !socket.connected) {
    return;
  }

  socket.emit('chat', { text });
  messageInput.value = '';
  messageInput.focus();
});

// Se invoca cuando se toca el botón "Salir" para abandonar la sala.
leaveButton.addEventListener('click', () => {
  if (!socket || !socket.connected) {
    return;
  }

  socket.emit('leave');
  resetUI();
});

// Conecta con el servidor y se une a la sala especificada por el usuario
function connectToRoom({ userName, roomCode }) {
  if (!userName || !roomCode) {
    addSystemMessage('Ingresá tu nombre y el código de sala.');
    return;
  }

  setStatus('Conectando...', 'connecting');
  socket = io();

  socket.on('connect', () => {
    socket.emit('join', { userName, roomCode });
  });

  // Se invoca cuando el servidor acepta la unión a la sala.
  socket.on('welcome', (data) => {
    currentClient = data.client;
    clientNameElement.textContent = `Sos ${currentClient.name}`;
    roomNameElement.textContent = `Sala ${data.roomCode}`;
    joinForm.hidden = true;
    chatInfoElement.hidden = false;
    usersPanelElement.hidden = false;
    setStatus('Conectado', 'open');
    messageInput.disabled = false;
    sendButton.disabled = false;
    messageInput.focus();
  });

  // Se invoca cuando el servidor notifica un cambio en la presencia de la sala (un cliente entró o salió). 
  socket.on('presence', (data) => {
    presenceElement.textContent = `${data.count} cliente${data.count === 1 ? '' : 's'} conectado${data.count === 1 ? '' : 's'}`;
    renderUsers(data.clients);
  });

  // Se invoca cuando el servidor envía un mensaje de chat (puede ser un mensaje propio o de otro cliente).
  socket.on('chat', (data) => {
    addChatMessage(data);
  });

  // Se invoca cuando el servidor envía un mensaje del sistema (por ejemplo, para notificar que un cliente entró o salió de la sala).
  socket.on('system', (data) => {
    addSystemMessage(data.text);
  });

  // Se invoca cuando el servidor notifica un error (por ejemplo, si no se pudo unir a la sala).
  socket.on('error', (data) => {
    addSystemMessage(data.text);
    resetConnectionAfterFailedJoin();
  });

  // Se invoca cuando el servidor notifica que el cliente salió de la sala.
  socket.on('left', () => {
    resetUI();
  });

  // Se invoca cuando se pierde la conexión con el servidor.
  socket.on('disconnect', () => {
    setStatus('Desconectado', 'closed');
    messageInput.disabled = true;
    sendButton.disabled = true;
  });
}

// Agrega un mensaje de chat al panel de mensajes.
function addChatMessage(message) {
  const isOwnMessage = currentClient?.id === message.client.id;
  const messageElement = document.createElement('article');
  messageElement.className = `message${isOwnMessage ? ' message--own' : ''}`;

  const metaElement = document.createElement('span');
  metaElement.className = 'message__meta';
  metaElement.textContent = `${message.client.name} · ${formatTime(message.sentAt)}`;

  const textElement = document.createElement('span');
  textElement.textContent = message.text;

  messageElement.append(metaElement, textElement);
  messagesElement.append(messageElement);
  messagesElement.scrollTop = messagesElement.scrollHeight;
}

// Agrega un mensaje del sistema al panel de mensajes.
function addSystemMessage(text) {
  const messageElement = document.createElement('article');
  messageElement.className = 'message message--system';
  messageElement.textContent = text;
  messagesElement.append(messageElement);
  messagesElement.scrollTop = messagesElement.scrollHeight;
}

// Actualiza el estado de la conexión (por ejemplo, "Conectando...", "Conectado", "Desconectado").
function setStatus(text, modifier) {
  statusElement.textContent = text;
  statusElement.className = `status status--${modifier}`;
}

// Restablece el estado de la interfaz después de un error al intentar unirse a una sala (por ejemplo, si el código de sala no existe).
function resetConnectionAfterFailedJoin() {
  if (currentClient) {
    return;
  }

  setStatus('Error de conexión', 'error');

  if (socket) {
    socket.disconnect();
    socket = null;
  }
}

// Restablece la interfaz al estado inicial para poder ingresar a otra sala.
function resetUI() {
  currentClient = null;

  if (socket) {
    socket.disconnect();
    socket = null;
  }

  joinForm.hidden = false;
  chatInfoElement.hidden = true;
  usersPanelElement.hidden = true;
  messageInput.disabled = true;
  sendButton.disabled = true;
  messageInput.value = '';
  setStatus('Conectando...', 'connecting');
}

// Renderiza la lista de usuarios conectados a la sala.
function renderUsers(clients) {
  usersListElement.replaceChildren(
    ...clients.map((client) => {
      const item = document.createElement('li');
      item.textContent = currentClient?.id === client.id ? `${client.name} (vos)` : client.name;
      return item;
    }),
  );
}

// Formatea una fecha/hora para mostrarla junto a cada mensaje de chat.
function formatTime(value) {
  return new Intl.DateTimeFormat('es-AR', {
    hour: '2-digit',
    minute: '2-digit',
  }).format(new Date(value));
}
