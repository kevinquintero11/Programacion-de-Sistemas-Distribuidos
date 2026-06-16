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

let currentClient = null;
let socket = null;

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

  socket.on('presence', (data) => {
    presenceElement.textContent = `${data.count} cliente${data.count === 1 ? '' : 's'} conectado${data.count === 1 ? '' : 's'}`;
    renderUsers(data.clients);
  });

  socket.on('chat', (data) => {
    addChatMessage(data);
  });

  socket.on('system', (data) => {
    addSystemMessage(data.text);
  });

  socket.on('error', (data) => {
    addSystemMessage(data.text);
    resetConnectionAfterFailedJoin();
  });

  socket.on('disconnect', () => {
    setStatus('Desconectado', 'closed');
    messageInput.disabled = true;
    sendButton.disabled = true;
  });
}

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

function addSystemMessage(text) {
  const messageElement = document.createElement('article');
  messageElement.className = 'message message--system';
  messageElement.textContent = text;
  messagesElement.append(messageElement);
  messagesElement.scrollTop = messagesElement.scrollHeight;
}

function setStatus(text, modifier) {
  statusElement.textContent = text;
  statusElement.className = `status status--${modifier}`;
}

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

function renderUsers(clients) {
  usersListElement.replaceChildren(
    ...clients.map((client) => {
      const item = document.createElement('li');
      item.textContent = currentClient?.id === client.id ? `${client.name} (vos)` : client.name;
      return item;
    }),
  );
}

function formatTime(value) {
  return new Intl.DateTimeFormat('es-AR', {
    hour: '2-digit',
    minute: '2-digit',
  }).format(new Date(value));
}
