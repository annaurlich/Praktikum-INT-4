#include <string.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"

// Inkludieren der LwIP Header
#include "lwip/pbuf.h"
#include "lwip/udp.h"

// UDP-Server (Empfänger) IP-Adresse und Port
#define UDP_SERVER_IP "192.168.0.101" // WICHTIG: IP an Empfänger-Pico anpassen!
#define UDP_PORT 9000
#define SEND_INTERVAL_MS 2000 // Sendeintervall in Millisekunden

// struktur zur Speicherung des Client Zustands (Temperaturwerte)
typedef struct UDP_SENDER_STATE
{
    struct udp_pcb *udp_pcb;
    ip_addr_t target_addr;
} UDP_SENDER_STATE;

// Funktion zum senden der Daten
void send_udp_packet(UDP_SENDER_STATE *state, const char *buf){
    // Puffer für Transport
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, strlen(buf), PBUF_RAM);
    if (p == NULL) {
        printf("Failed to allocate pbuf\n");
        return;
    }
    // Daten in den Puffer-Payload kopieren
    memcpy(p->payload, buf, strlen(buf));
    
    // UDP- Packet senden 
    err_t err = udp_sendto(state->udp_pcb, p, &state->target_addr, UDP_PORT);
    if (err != ERR_OK) {
        printf("Failed to send UDP packet! error=%d", err);
    } else {
        printf("Sent packet: %s\n", buf);
    }

    // Puffer wieder freigeben
    pbuf_free(p);
}

int main(){
    stdio_init_all();

    //Zustand initalisieren
    UDP_SENDER_STATE *state = calloc(1,sizeof(UDP_SENDER_STATE));
    if (!state){
        printf("Failed to allocat state \n");
        return -1;
    }

    // Wifi Verbindung herstellen
    if (cyw43_arch_init()){
        printf("WiFi init failed");
        return 1
    }

    cyw43_arch_enable_sta_mode(); 
    printf("Connecting to WiFi...\n", WIFI_SSID); 
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) { 
        printf("failed to connect.\n"); 
        return 1; 
    } else { 
        printf("Connected to WiFi. My IP: %s\n", ip4addr_ntoa(netif_ip4_addr(&(cyw43_state.netif[CYW43_ITF_STA]))));
    }

    // UDP-Client initialisieren
    state->udp_pcb = udp_new();
    if (state->udp_pcb == NULL) {
        printf("Failed to create pcb\n");
        return 1;
    }

    // Ziel-IP Adresse setzten
    ip4addr_aton(UDP_SERVER_IP, &state->target_addr);

    // Temperatursensor initialisieren
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);

    // Hauptschleife: Temperatur messen und senden
    while (1) {
        // Temperatur auslesen und umrechnen 
        










    }
    
    free(state);
    return 0;

}




















