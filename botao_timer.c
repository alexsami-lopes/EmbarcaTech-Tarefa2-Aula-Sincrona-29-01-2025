//Embarcatec 
//Tarefa 2 - Aula Sincrona - 29/01/2025
//Discente: Alexsami

#include <stdio.h> // Biblioteca padrão do C.
#include "pico/stdlib.h" // Biblioteca padrão do Raspberry Pi Pico para controle de GPIO, temporização e comunicação serial.
#include "pico/time.h"   // Biblioteca para gerenciamento de temporizadores e alarmes.

const uint LED_PIN_BLUE = 11; // Define o pino GPIO 11 para controlar o LED AZUL.
const uint LED_PIN_RED = 12; // Define o pino GPIO 12 para controlar o LED VERMELHO.
const uint LED_PIN_GREEN = 13; // Define o pino GPIO 13 para controlar o LED VERDE.
  
const uint BUTTON_PIN = 5;  // Define o pino GPIO 5 para ler o estado do botão.

int count = 0;  // Somado ao pino base (LED_PIN_BLUE) indica qual led deve ser desligado.

static volatile uint32_t last_time = 0; // Armazena o tempo do último evento (em microssegundos)

bool leds_active = false;    // Indica se o LED está atualmente aceso (para evitar múltiplas ativações).
absolute_time_t turn_off_time;  // Variável para armazenar o tempo em que o LED deve ser desligado (não utilizada neste código).

// Função de callback para desligar os LEDs um por um a cada 3 segundos.
int64_t turn_off_callback(alarm_id_t id, void *user_data) {
    // Desliga o LED atual usando LED_PIN_BLUE como base.
    // Com count=0 desliga o LED azul (GPIO11), count=1 desliga o LED vermelho (GPIO12) e count=2 desliga o LED verde (GPIO13).
    gpio_put(LED_PIN_BLUE + count, false);

    count++;  // Avança para o próximo LED

    // Se ainda há LEDs para desligar, reagenda o alarme.
    if(count < 3) {
        add_alarm_in_ms(3000, turn_off_callback, NULL, false);
    } else {
        // Todos os LEDs foram desligados, reseta a contagem e sinaliza que a rotina acabou.
        count = 0;
        leds_active = false;
    }
    return 0;  
}

int main() {
    // Inicializa a comunicação serial para permitir o uso de printf.
    // Isso é útil para depuração, embora não seja usado neste exemplo.
    stdio_init_all();

    // Configura o pino LED_PIN_BLUE (11) como saída digital.
    gpio_init(LED_PIN_BLUE);
    gpio_set_dir(LED_PIN_BLUE, GPIO_OUT);

    // Configura o pino LED_PIN_RED (12) como saída digital.
    gpio_init(LED_PIN_RED);
    gpio_set_dir(LED_PIN_RED, GPIO_OUT);

    // Configura o pino LED_PIN_GREEN (13) como saída digital.
    gpio_init(LED_PIN_GREEN);
    gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);

    // Configura o pino BUTTON_PIN (5) como entrada digital.
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);

    // Habilita o resistor pull-up interno para o pino do botão.
    // Isso garante que o pino seja lido como alto (3,3 V) quando o botão não está pressionado.
    gpio_pull_up(BUTTON_PIN);

    // Loop principal do programa que verifica continuamente o estado do botão.
    while (true) {
        // Verifica se o botão foi pressionado (nível baixo no pino) e se o LED não está ativo.
        if (gpio_get(BUTTON_PIN) == 0 && !leds_active) {
            // Adiciona um pequeno atraso para debounce, evitando leituras errôneas.
            // Obtém o tempo atual em microssegundos
            uint32_t current_time = to_us_since_boot(get_absolute_time());

            // Verifica se passou tempo suficiente desde o último evento
            if (current_time - last_time > 200000) // 200 ms de debouncing
            {

              // Verifica novamente o estado do botão após o debounce.
              if (gpio_get(BUTTON_PIN) == 0) {
                  // Liga o LED configurando o pino LED_PIN_BLUE para nível alto.
                  gpio_put(LED_PIN_BLUE, true);

                  gpio_put(LED_PIN_RED, true);

                  gpio_put(LED_PIN_GREEN, true);

                  // Define 'leds_active' como true para indicar que um ou mais LEDs estão acesos.
                  leds_active = true;

                  // Agenda um alarme para desligar o LED após 3 segundos (3000 ms).
                  // A função 'turn_off_callback' será chamada após esse tempo.
                  add_alarm_in_ms(3000, turn_off_callback, NULL, false);


              }
            }
        }

        // Introduz uma pequena pausa de 10 ms para reduzir o uso da CPU.
        // Isso evita que o loop seja executado muito rapidamente e consuma recursos desnecessários.
        sleep_ms(10);
    }

    // Retorno de 0, que nunca será alcançado devido ao loop infinito.
    // Isso é apenas uma boa prática em programas com um ponto de entrada main().
    return 0;
}
