
typedef struct {
    uint32_t magic;
    char wifi_ssid[32];
    char wifi_password[64];
    uint8_t unused_1[3];
    bool use_dhcp;
    uint8_t unused_2[3];
    bool right_to_left;
    uint8_t ip[4];
    uint8_t gateway[4];
    uint8_t netmask[4];
    uint32_t strip_length;
    uint32_t crc;
} led_strip_config_t;
