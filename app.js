let chartTempUmid = null;
let chartLuminosidade = null;
const alertasAtivos = new Set();

const mqttClient = mqtt.connect('wss://broker.hivemq.com:8884/mqtt', {
  clientId: 'web_estufa_display_' + Math.random().toString(16).substr(2, 8)
});

mqttClient.on('connect', () => {
  const status = document.getElementById('mqtt-status');
  status.textContent = 'Broker conectado';
  status.classList.add('online');
  mqttClient.subscribe('estufa/senai/dados');
});

mqttClient.on('offline', () => {
  const status = document.getElementById('mqtt-status');
  status.textContent = 'Dispositivo desconectado';
  status.classList.remove('online');
});

mqttClient.on('error', () => { document.getElementById('mqtt-status').textContent = 'Falha na conexão'; });

mqttClient.on('message', (topic, message) => {
  if (topic !== 'estufa/senai/dados') return;
  try { processarDadosRecebidos(JSON.parse(message.toString())); }
  catch (error) { console.error('Payload MQTT inválido', error); }
});

function opcoesGrafico() {
  return { responsive: true, maintainAspectRatio: false, interaction: { mode: 'index', intersect: false }, plugins: { legend: { labels: { color: '#718077', usePointStyle: true, boxWidth: 8 } } }, scales: { x: { ticks: { color: '#718077' }, grid: { display: false } }, y: { ticks: { color: '#718077' }, grid: { color: '#e5ebe2' } } } };
}

function inicializarGraficos() {
  chartTempUmid = new Chart(document.getElementById('chartTempUmid'), { type: 'line', data: { labels: [], datasets: [{ label: 'Temperatura (°C)', data: [], borderColor: '#c97921', backgroundColor: 'rgba(201,121,33,.12)', tension: .35, fill: true }, { label: 'Umidade (%)', data: [], borderColor: '#287d84', backgroundColor: 'rgba(40,125,132,.08)', tension: .35, fill: true }] }, options: opcoesGrafico() });
  chartLuminosidade = new Chart(document.getElementById('chartLuminosidade'), { type: 'line', data: { labels: [], datasets: [{ label: 'Luminosidade (%)', data: [], borderColor: '#2f7956', backgroundColor: 'rgba(47,121,86,.12)', tension: .35, fill: true }] }, options: opcoesGrafico() });
}

function processarDadosRecebidos(data) {
  const temp = Number.parseFloat(data.temp); const umid = Number(data.umid); const agua = Math.max(0, Math.min(100, Number(data.agua))); const luz = Number(data.luz);
  if (![temp, umid, agua, luz].every(Number.isFinite)) return;
  document.getElementById('temp-val').textContent = `${temp.toFixed(1)} °C`;
  document.getElementById('umid-val').textContent = `${umid.toFixed(0)} %`;
  document.getElementById('nivel-agua-val').textContent = `${agua.toFixed(0)}%`;
  document.getElementById('luz-val').textContent = `${luz.toFixed(0)}%`;
  document.getElementById('water-bar').style.width = `${agua}%`;
  document.getElementById('summary-title').textContent = temp > 30 || umid > 85 ? 'Atenção ao microclima' : 'Ambiente dentro dos parâmetros';
  document.getElementById('last-update').textContent = `Última leitura às ${new Date().toLocaleTimeString('pt-BR')}`;
  atualizarStatusBombaUI(Boolean(data.bomba));
  atualizarAlertas(temp, umid);
  const hora = new Date().toLocaleTimeString('pt-BR', { hour: '2-digit', minute: '2-digit' });
  adicionarPontoGrafico(chartTempUmid, hora, [temp, umid]); adicionarPontoGrafico(chartLuminosidade, hora, [luz]);
}

function atualizarStatusBombaUI(estado) {
  document.getElementById('irrigacao-texto').textContent = estado ? 'LIGADO' : 'DESLIGADO';
  document.getElementById('irrigacao-texto').style.color = estado ? 'var(--cyan)' : 'var(--ink)';
  document.getElementById('irrigacao-badge').textContent = estado ? 'Bomba em operação' : 'Bomba inativa';
}

function atualizarAlertas(temp, umid) {
  const alertas = [];
  if (temp > 30) alertas.push({ id: 'calor', classe: 'hot', simbolo: '!', titulo: 'Está muito quente', texto: `Temperatura em ${temp.toFixed(1)} °C. Verifique a ventilação.` });
  if (umid > 85) alertas.push({ id: 'mofo', classe: 'mold', simbolo: '!', titulo: 'Alerta de mofo', texto: `Umidade do ar em ${umid.toFixed(0)}%. Aumente a circulação de ar.` });
  alertas.forEach(alerta => { if (!alertasAtivos.has(alerta.id)) { notificar(alerta.titulo, alerta.texto); alertasAtivos.add(alerta.id); } });
  ['calor', 'mofo'].forEach(id => { if (!alertas.some(alerta => alerta.id === id)) alertasAtivos.delete(id); });
  const lista = document.getElementById('alert-list');
  document.getElementById('alert-count').textContent = `${alertas.length} ativo${alertas.length === 1 ? '' : 's'}`;
  lista.innerHTML = alertas.length ? alertas.map(alerta => `<div class="alert ${alerta.classe}"><span class="alert-symbol">${alerta.simbolo}</span><div><strong>${alerta.titulo}</strong>${alerta.texto}</div></div>`).join('') : '<div class="alert ok"><span class="alert-symbol">✓</span><div><strong>Ambiente dentro dos parâmetros</strong>Sem alertas no momento.</div></div>';
}

function notificar(titulo, texto) { if ('Notification' in window && Notification.permission === 'granted') new Notification(`Estufa: ${titulo}`, { body: texto }); }

function adicionarPontoGrafico(chart, label, dados) {
  if (!chart) return;
  if (chart.data.labels.length >= 12) { chart.data.labels.shift(); chart.data.datasets.forEach(dataset => dataset.data.shift()); }
  chart.data.labels.push(label); chart.data.datasets.forEach((dataset, index) => dataset.data.push(dados[index])); chart.update('none');
}

window.addEventListener('load', () => {
  inicializarGraficos();
  if ('Notification' in window && Notification.permission === 'default') Notification.requestPermission();
  if ('serviceWorker' in navigator) {
    navigator.serviceWorker.register('./sw.js').then(registration => registration.update());
  }
});

