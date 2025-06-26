const { Client, LocalAuth } = require('whatsapp-web.js');
const qrcode = require('qrcode-terminal');

// Cria o cliente com autenticação local

const mqtt = require('mqtt');

// Configuração do MQTT
const MQTT_BROKER = 'mqtt://localhost:1883'; // ou IP/porta do seu broker
const MQTT_TOPIC = 'whatsapp/messages';

const options = {
    username: 'aluno',
    password: 'caiovitor12'
};

// Conectando ao broker MQTT
const mqttClient = mqtt.connect(MQTT_BROKER);

mqttClient.on('connect', () => {
    console.log('✅ Conectado ao broker MQTT');
});

const client = new Client({
    authStrategy: new LocalAuth(),
    puppeteer: {
        headless: false,  // pode colocar false para depuração
        args: [
            '--no-sandbox',
            '--disable-setuid-sandbox',
            '--disable-dev-shm-usage',
            '--disable-accelerated-2d-canvas',
            '--disable-gpu',
        ],
        executablePath: '/usr/bin/google-chrome'  // apontando pro Chromium Snap
    }
});

// Exibe o QR Code no terminal
client.on('qr', (qr) => {
    console.log('Escaneie o QR Code abaixo:');
    qrcode.generate(qr, { small: true });
});

// Evento de pronto
client.on('ready', () => {
    console.log('✅ Conectado ao WhatsApp!');
});

// Evento de nova mensagem
client.on('message', (message) => {
    console.log('📩 Nova mensagem recebida:');
    console.log('De:', message.from);
    console.log('Conteúdo:', message.body);

    const payload = JSON.stringify({
        from: message.from,
        message: message.body,
        timestamp: new Date().toISOString()
    });

    mqttClient.publish(MQTT_TOPIC, payload);
});

client.initialize();

process.on('SIGINT', async () => {
    console.log('\nEncerrando...');
    await client.destroy();  // encerra o whatsapp-web.js
    process.exit();
});
