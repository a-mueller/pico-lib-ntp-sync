#include "pico/stdlib.h"
#include <stdio.h>
#include "pico_ntp_sync.hpp"

// Called with results of operation
static void ntp_result(NTP_T* state, int status, time_t *result) {
    if (status == 0 && result) {
        struct tm *utc = gmtime(result);
        datetime_t rtc_time;
        rtc_time.year = utc->tm_year + 1900;
        rtc_time.month= utc->tm_mon + 1;
        rtc_time.day = utc->tm_mday;
        rtc_time.hour = utc->tm_hour;
        rtc_time.min = utc->tm_min;
        rtc_time.sec = utc->tm_sec;
        rtc_time.dotw = utc->tm_wday;
        if (!rtc_set_datetime(&rtc_time)) {
            state->status = NTP_STATUS_RESULT_ERROR;
        } else {
            state->status = NTP_STATUS_RESULT_OK;  
        }
    } else {
        state->status = NTP_STATUS_RESULT_ERROR;
    }
    
}

static int64_t ntp_failed_handler(alarm_id_t id, void *user_data);

// Make an NTP request
static void ntp_request(NTP_T *state) {
    state->status = NTP_STATUS_WAIT_NTP;
    cyw43_arch_lwip_begin();
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, NTP_MSG_LEN, PBUF_RAM);
    uint8_t *req = (uint8_t *) p->payload;
    memset(req, 0, NTP_MSG_LEN);
    req[0] = 0x1b;
    udp_sendto(state->ntp_pcb, p, &state->ntp_server_address, NTP_PORT);
    pbuf_free(p);
    cyw43_arch_lwip_end();
}

static int64_t ntp_failed_handler(alarm_id_t id, void *user_data)
{
    NTP_T* state = (NTP_T*)user_data;
    state->status = NTP_STATUS_RESULT_ERROR;
    return 0;
}

// Call back with a DNS result
static void ntp_dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg) {
    NTP_T *state = (NTP_T*)arg;
    if (ipaddr) {
        state->ntp_server_address = *ipaddr;
        ntp_request(state);
    } else {
       state->status = NTP_STATUS_RESULT_ERROR;
    }
}

// NTP data received
static void ntp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    NTP_T *state = (NTP_T*)arg;
    uint8_t mode = pbuf_get_at(p, 0) & 0x7;
    uint8_t stratum = pbuf_get_at(p, 1);

    // Check the result
    if (ip_addr_cmp(addr, &state->ntp_server_address) && port == NTP_PORT && p->tot_len == NTP_MSG_LEN && mode == 0x4 && stratum != 0) {
        uint8_t seconds_buf[4] = {0};
        pbuf_copy_partial(p, seconds_buf, sizeof(seconds_buf), 40);
        uint32_t seconds_since_1900 = seconds_buf[0] << 24 | seconds_buf[1] << 16 | seconds_buf[2] << 8 | seconds_buf[3];
        uint32_t seconds_since_1970 = seconds_since_1900 - NTP_DELTA;
        time_t epoch = seconds_since_1970;
        ntp_result(state, 0, &epoch);
    } else {
        state->status = NTP_STATUS_RESULT_ERROR;
    }
    pbuf_free(p);
}


// Perform initialisation
static NTP_T* ntp_init(void) {
    NTP_T *state = (NTP_T*)calloc(1, sizeof(NTP_T));
    if (!state) {
        return NULL;
    }
    state->status = NTP_STATUS_DEFAULT;
    state->ntp_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
    if (!state->ntp_pcb) {
        free(state);
        return NULL;
    }
    udp_recv(state->ntp_pcb, ntp_recv, state);
    return state;
}


int pico_ntp_sync(const std::string& ssid, const std::string& wpa) {
    rtc_init(); // make sure the RTC clock is on

    cyw43_arch_enable_sta_mode();

    if (cyw43_arch_wifi_connect_timeout_ms(ssid.c_str(), wpa.c_str(), CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        return -1;
    }
    
    NTP_T *state = ntp_init();
    if (!state) return -2;

    state->status = NTP_STATUS_WAIT_DNS;
    cyw43_arch_lwip_begin();
    int err = dns_gethostbyname(NTP_SERVER, &state->ntp_server_address, ntp_dns_found, state);
    cyw43_arch_lwip_end();

    state->dns_request_sent = true;
    if (err == ERR_OK) {
        state->status = NTP_STATUS_WAIT_NTP;
        ntp_request(state); // Cached result, otherwise we will go via the callback
    }
    
    while(state->status != NTP_STATUS_RESULT_OK && state->status != NTP_STATUS_RESULT_ERROR) {
        sleep_ms(100);
    }
    
    int result = -1;
    if (state->status == NTP_STATUS_RESULT_OK) {
      result = 0;
    }
    
    free(state);
    cyw43_arch_disable_sta_mode();
    // disable internet
    //cyw43_arch_deinit();
    
    
    return result;
}
