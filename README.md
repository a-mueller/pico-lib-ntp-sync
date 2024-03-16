# Pico NTP sync

Syncs the internal RTC with the time from the internet.

## Warning

I have really no idea what I am doing both on the electronics and with C / C++. This code is mostly copied together from stuff I found somewhere. For some reason the lwip stuff prints lots of warning when compiling this, but I haven't found out why.


## How to use it

```
int main() {
    stdio_init_all();
    rtc_init();
    
    if (cyw43_arch_init()) {
        printf("WiFi init failed\n");
        return -1;
    }

    if (pico_ntp_sync("YOUR_SSID", "YOUR_PASSWORD") == 0) {
        datetime_t rtc_time;
        if (rtc_get_datetime(&rtc_time)) {
            // do something with the time which should now be accurate
        }
    }
}
```
