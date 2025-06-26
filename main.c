#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "include/ssd1306.h"
#include "include/wifi_conn.h"
#include "include/mqtt_comm.h"
#include "FreeRTOS.h"
#include "task.h"

#define MAX_MSGS 20
#define LINHAS_VISIVEIS 5

const uint I2C_SDA = 14;
const uint I2C_SCL = 15;
const uint BUTTON_MARK = 5;
const uint JOYSTICK_VRX = 26;
#define ADC_CHANNEL_VRX 0

typedef struct {
    char texto[64];
    bool entregue;
} Mensagem;

Mensagem mensagens[MAX_MSGS];
int total_mensagens = 0;
int indice_selecionado = 0;
int scroll_offset = 0;

void trim(char *str) {
    int len = strlen(str);
    while(len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\r' || str[len - 1] == '\n')) {
        str[--len] = '\0';
    }
    char *start = str;
    while(*start == ' ') start++;
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

void remover_quebras_linha(char *str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] == '\n' || str[i] == '\r') {
            str[i] = ' ';
        }
    }
}

void mqtt_callback(const char *topic, const char *payload, int len) {
    if (strncmp(topic, "whatsapp/messages", 17) != 0) return;

    char remetente[32];
    char corpo[64];

    if (sscanf(payload, "{\"from\":\"%31[^\"]\",\"message\":\"%63[^\"]\",\"timestamp\":\"%*[^\"]\"}", remetente, corpo) == 2) {
        if (total_mensagens < MAX_MSGS) {
            trim(corpo);
            remover_quebras_linha(corpo);
            snprintf(mensagens[total_mensagens].texto, sizeof(mensagens[total_mensagens].texto), "%s", corpo);
            mensagens[total_mensagens].entregue = false;
            total_mensagens++;
            printf("[MQTT] Nova mensagem de %s: %s\n", remetente, corpo);
        }
    } else {
        printf("[MQTT] Erro ao fazer parsing do JSON: %s\n", payload);
    }
}

void mostrar_mensagens(uint8_t *ssd, struct render_area *area) {
    memset(ssd, 0, ssd1306_buffer_length);

    if (indice_selecionado < scroll_offset) {
        scroll_offset = indice_selecionado;
    } else if (indice_selecionado >= scroll_offset + LINHAS_VISIVEIS) {
        scroll_offset = indice_selecionado - LINHAS_VISIVEIS + 1;
    }

    for (int i = 0; i < LINHAS_VISIVEIS; i++) {
        int msg_index = scroll_offset + i;
        if (msg_index >= total_mensagens) break;

        char linha[64];
        snprintf(
            linha, sizeof(linha),
            "%s%s %.20s",
            (msg_index == indice_selecionado ? "\xe2\x86\x92" : " "),
            mensagens[msg_index].entregue ? "[X]" : "[ ]",
            mensagens[msg_index].texto
        );

        ssd1306_draw_string(ssd, 0, i * 12, linha);
    }

    render_on_display(ssd, area);
}

void oled_task(void *params) {
    struct render_area area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };
    calculate_render_area_buffer_length(&area);

    uint8_t ssd[ssd1306_buffer_length];

    while (1) {
        mostrar_mensagens(ssd, &area);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void input_task(void *params) {
    bool last_mark = true;
    absolute_time_t ultimo_scroll = get_absolute_time();

    while (1) {
        bool state_mark = gpio_get(BUTTON_MARK);
        if (!state_mark && last_mark && total_mensagens > 0) {
            mensagens[indice_selecionado].entregue = !mensagens[indice_selecionado].entregue;
        }
        last_mark = state_mark;

        adc_select_input(ADC_CHANNEL_VRX);
        sleep_us(2);
        uint16_t vrx_value = adc_read();
        int64_t diff = absolute_time_diff_us(ultimo_scroll, get_absolute_time());

        if (diff > 300000) {
            if (vrx_value > 3000 && indice_selecionado > 0) {
                indice_selecionado--;
                ultimo_scroll = get_absolute_time();
            } else if (vrx_value < 1000 && indice_selecionado < total_mensagens - 1) {
                indice_selecionado++;
                ultimo_scroll = get_absolute_time();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

int main() {
    stdio_init_all();

    connect_to_wifi("CAIOECAMYLA-", "cv102002");
    mqtt_setup("bitdog2", "192.168.1.4", "aluno", "caiovitor12");
    mqtt_set_callback(mqtt_callback);
    mqtt_comm_subscribe("whatsapp/messages");

    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init();

    gpio_init(BUTTON_MARK);
    gpio_set_dir(BUTTON_MARK, GPIO_IN);
    gpio_pull_up(BUTTON_MARK);

    adc_init();
    adc_gpio_init(JOYSTICK_VRX);

    xTaskCreate(oled_task, "OLED Task", 2048, NULL, 1, NULL);
    xTaskCreate(input_task, "Input Task", 2048, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1); // Nunca chega aqui
}
