#include <string.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

// LwIP und Display Header
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#define UDP_PORT 9000
#define MSG_BUFFER_SIZE 32
#define WIFI_SSID „MRT1“ 
#define WIFI_PASSWORD „mrt1“ 

// Struktur, um den Zustand Anwendung zu speichern
typedef struct UDP_RECEIVER_STATE {
    char received_message[MSG_BUFFER_SIZE];
    bool new_data_received;
} UDP_RECEIVER_STATE;

// UDP Receive Callback-Funktion, wie auf Seite 13/14  
// Diese Funktion wird vom LwIP-Stack aufgerufen, wenn ein Paket eintrifft.
void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    UDP_RECEIVER_STATE *state = (UDP_RECEIVER_STATE*)arg;

    if (p != NULL) {
        // Kopiere Daten aus dem pbuf-Payload in unseren Puffer
        int len = p->len < MSG_BUFFER_SIZE - 1 ? p->len : MSG_BUFFER_SIZE - 1;
        memcpy(state->received_message, p->payload, len);
        state->received_message[len] = '\0'; 

        state->new_data_received = true;

        printf("Received packet from %s: '%s'\n", ip4addr_ntoa(addr), state->received_message);
         
        pbuf_free(p);
    }
}

int main() {
    stdio_init_all();

    // Zustand für Empfänger erstellen
    UDP_RECEIVER_STATE *state = calloc(1, sizeof(UDP_RECEIVER_STATE));
    if (!state) {
        printf("Failed to allocate state\n");
        return -1;
    }
    

    // Display initialisieren
    DEV_Module_Init();
    DEV_Delay_ms(1000);
    LCD_1IN14_V2_Init(HORIZONTAL);
    DEV_SET_PWM(50);
    UWORD Image[LCD_1IN14_V2_HEIGHT * LCD_1IN14_V2_WIDTH * 2];
    Paint_NewImage((UBYTE *)Image, LCD_1IN14_V2.WIDTH, LCD_1IN14_V2.HEIGHT, 0, WHITE);
    Paint_SetRotate(ROTATE_0);
    Paint_Clear(WHITE);

    // WiFi-Verbindung herstellen
    if (cyw43_arch_init()) {
        printf("WiFi init failed");
        return -1;
    }
    cyw43_arch_enable_sta_mode();
    printf("Connecting to WiFi '%s'...\n", WIFI_SSID);
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        return 1;
    }
    printf("Connected. My IP is %s\n", ip4addr_ntoa(netif_ip4_addr(netif_default)));

    // UDP-Server initialisieren 
    struct udp_pcb *pcb = udp_new();
    if (udp_bind(pcb, IP_ADDR_ANY, UDP_PORT) != ERR_OK) {
        printf("Failed to bind pcb\n");
        return -1;
    }
    // Callback-Funktion registrieren mit udp_recv() 
    udp_recv(pcb, udp_receive_callback, state);

    // Hauptschleife: Display aktualisieren, wenn neue Daten da sind
    while (1) {
        if (state->new_data_received) {
            Paint_Clear(WHITE);
            // Statischen Text und IP-Adresse anzeigen
            Paint_DrawString_EN(10, 20, "UDP Empfaenger", &Font20, BLACK, WHITE);
            Paint_DrawString_EN(10, 50, ip4addr_ntoa(netif_ip4_addr(netif_default)), &Font16, BLUE, WHITE);
            // Empfangene Nachricht anzeigen
            Paint_DrawString_EN(10, 90, state->received_message, &Font24, BLACK, WHITE);
            
            // Den Buffer auf das Display schreiben 
            LCD_1IN14_V2_Display(Image);
            state->new_data_received = false;
        }
        
        sleep_ms(100);
    }

    free(state);
    return 0;
}