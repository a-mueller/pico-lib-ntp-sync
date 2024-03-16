#ifndef PICO_NTP_SYNC_H
# define PICO_NTP_SYNC_H

#include "hardware/rtc.h"
#include "pico/cyw43_arch.h"
#include <string.h>
#include <time.h>
#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include <string>

#define NTP_SERVER "pool.ntp.org"
#define NTP_MSG_LEN 48
#define NTP_PORT 123
#define NTP_DELTA 2208988800 // seconds between 1 Jan 1900 and 1 Jan 1970
#define NTP_TEST_TIME (30 * 1000)
#define NTP_RESEND_TIME (10 * 1000)
#define NTP_STATUS_DEFAULT 0
#define NTP_STATUS_WAIT_DNS 1
#define NTP_STATUS_WAIT_NTP 2
#define NTP_STATUS_RESULT_OK 3
#define NTP_STATUS_RESULT_ERROR 4


typedef struct NTP_T_ {
    ip_addr_t ntp_server_address;
    bool dns_request_sent;
    struct udp_pcb *ntp_pcb;
    int status;
} NTP_T;

/* Syncs the Pico's internal RTC with the network time */
int pico_ntp_sync(const std::string& ssid, const std::string& wpa);

#endif
