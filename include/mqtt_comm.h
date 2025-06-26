#ifndef MQTT_COMM_H
#define MQTT_COMM_H
void mqtt_setup(const char *client_id, const char *broker_ip, const char *user, const char *pass);
void mqtt_comm_publish(const char *topic, const uint8_t *data, size_t len);
typedef void (*mqtt_message_callback_t)(const char *topic, const char *payload, int len);
void mqtt_set_callback(mqtt_message_callback_t callback);
void mqtt_comm_subscribe(const char *topic);
#endif