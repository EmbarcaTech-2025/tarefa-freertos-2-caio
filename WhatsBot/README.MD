# 📲 Integração WhatsApp + MQTT com Node.js

Este projeto integra o **WhatsApp Web** com um **broker MQTT**, utilizando a biblioteca `whatsapp-web.js` para capturar mensagens recebidas e publicar os dados no tópico MQTT.

---

## ⚙️ Requisitos

Antes de começar, você precisa ter instalado:

- Node.js (v14 ou superior)
- npm (gerenciador de pacotes)
- Google Chrome (ou Chromium)
- Um broker MQTT local (ex: [Mosquitto](https://mosquitto.org/))

---

```bash
mkdir whatsapp-mqtt && cd whatsapp-mqtt
