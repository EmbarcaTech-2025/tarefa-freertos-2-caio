#include <string.h>
#include <stdio.h>
#include "lwip/apps/mqtt.h"
#include "include/mqtt_comm.h"
#include "lwipopts.h"

static mqtt_message_callback_t user_callback = NULL;
static char incoming_topic[128];
static mqtt_client_t *client;

// Declaração antecipada dos callbacks internos
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
static void mqtt_sub_cb(void *arg, err_t result);

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("[MQTT] Conectado ao broker com sucesso!\n");

        mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, NULL);

        mqtt_comm_subscribe("whatsapp/messages"); // ou outro tópico
    } else {
        printf("[MQTT] Falha ao conectar ao broker, código: %d\n", status);
    }
}

void mqtt_setup(const char *client_id, const char *broker_ip, const char *user, const char *pass) {
    ip_addr_t broker_addr;

    if (!ip4addr_aton(broker_ip, &broker_addr)) {
        printf("[MQTT] IP inválido: %s\n", broker_ip);
        return;
    }

    client = mqtt_client_new();
    if (client == NULL) {
        printf("[MQTT] Falha ao criar cliente MQTT\n");
        return;
    }

    struct mqtt_connect_client_info_t ci = {
        .client_id = client_id,
        .client_user = user,
        .client_pass = pass
    };

    mqtt_client_connect(client, &broker_addr, 1883, mqtt_connection_cb, NULL, &ci);
}

static void mqtt_pub_request_cb(void *arg, err_t result) {
    if (result == ERR_OK) {
        printf("[MQTT] Publicação enviada com sucesso\n");
    } else {
        printf("[MQTT] Erro ao publicar (err=%d)\n", result);
    }
}

void mqtt_comm_publish(const char *topic, const uint8_t *data, size_t len) {
    err_t status = mqtt_publish(
        client,
        topic,
        data,
        len,
        0, // QoS 0
        0, // retain
        mqtt_pub_request_cb,
        NULL
    );

    if (status != ERR_OK) {
        printf("[MQTT] mqtt_publish falhou: %d\n", status);
    }
}

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    strncpy(incoming_topic, topic, sizeof(incoming_topic) - 1);
    incoming_topic[sizeof(incoming_topic) - 1] = '\0';
}

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    if (user_callback) {
        char payload[128];
        int copy_len = len < sizeof(payload) - 1 ? len : sizeof(payload) - 1;
        memcpy(payload, data, copy_len);
        payload[copy_len] = '\0';
        printf("[MQTT] Mensagem recebida no tópico %s: %s\n", incoming_topic, payload);
        user_callback(incoming_topic, payload, copy_len);
    }
}

void mqtt_set_callback(mqtt_message_callback_t callback) {
    user_callback = callback;
}

static void mqtt_sub_cb(void *arg, err_t result) {
    if (result == ERR_OK) {
        printf("[MQTT] Inscrição confirmada com sucesso.\n");
    } else {
        printf("[MQTT] Erro ao se inscrever: %d\n", result);
    }
}

void mqtt_comm_subscribe(const char *topic) {
    if (!mqtt_client_is_connected(client)) {
        printf("[MQTT] Não conectado, inscrição adiada para: %s\n", topic);
        return;
    }

    err_t err = mqtt_sub_unsub(client, topic, 1, mqtt_sub_cb, NULL, 1);
    if (err == ERR_OK) {
        printf("[MQTT] Inscrição enviada para tópico: %s\n", topic);
    } else {
        printf("[MQTT] Falha ao tentar se inscrever: %s (err %d)\n", topic, err);
    }
}