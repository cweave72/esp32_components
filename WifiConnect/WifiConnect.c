/*******************************************************************************
 *  @file: WifiConnect.c
 *  
 *  @brief: A helper component for easily connecting to Wifi AP.
*******************************************************************************/
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include <netdb.h>
#include "WifiConnect.h"
#include "LogPrint.h"

static const char *TAG = "WifiConnect";

#ifdef CONFIG_ESP_MAXIMUM_RETRY
#define MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY
#else
#define MAXIMUM_RETRY  4
#endif

#define WIFI_CONNECTED_FLAG     (uint32_t)(0x1 << 0)
#define WIFI_FAILED_FLAG        (uint32_t)(0x1 << 1)

#define RET_ON_ERROR_MSG(x, msg)                    \
    do {                                            \
        if ((x) != ESP_OK) {                        \
            ESP_LOGE(TAG, "%s: 0x%x", msg, (x));    \
            return (x);                             \
        }                                           \
    } while(0)

/** @brief Configuration object for WifiConnect.
*/
typedef struct WifiConfig
{
    char ssid[32];
    char password[64];
    uint8_t use_dhcp;
    char ip[32];
    char netmask[32];
    char gw[32];
} WifiConfig;

static WifiConfig current_cfg;
static EventGroupHandle_t eventGrp;
static int retry_count = 0;


/******************************************************************************
    set_dns_server
*//**
    @brief Sets the DNS server.
******************************************************************************/
static esp_err_t
set_dns_server(esp_netif_t *netif, uint32_t addr, esp_netif_dns_type_t type)
{
    if (addr && (addr != IPADDR_NONE))
    {
        esp_netif_dns_info_t dns;
        dns.ip.u_addr.ip4.addr = addr;
        dns.ip.type = IPADDR_TYPE_V4;
        ESP_ERROR_CHECK(esp_netif_set_dns_info(netif, type, &dns));
    }
    return ESP_OK;
}

/******************************************************************************
    set_static_ip
*//**
    @brief Sets a static ip.
******************************************************************************/
static int
set_static_ip(esp_netif_t *netif)
{
    esp_netif_ip_info_t ip;
    esp_err_t ret;

    if (esp_netif_dhcpc_stop(netif) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to stop dhcp client");
        return ESP_FAIL;
    }

    memset(&ip, 0 , sizeof(esp_netif_ip_info_t));

    ip.ip.addr      = ipaddr_addr(current_cfg.ip);
    ip.netmask.addr = ipaddr_addr(current_cfg.netmask);
    ip.gw.addr      = ipaddr_addr(current_cfg.gw);

    if (esp_netif_set_ip_info(netif, &ip) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set ip info");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Success setting static ip: %s, netmask: %s, gw: %s",
        current_cfg.ip, current_cfg.netmask, current_cfg.gw);

    ret = set_dns_server(netif, ipaddr_addr(current_cfg.gw), ESP_NETIF_DNS_MAIN);
    RET_ON_ERROR_MSG(ret, "Error setting dns server");

    //ESP_ERROR_CHECK(
    //    example_set_dns_server(
    //        netif,
    //        ipaddr_addr(EXAMPLE_BACKUP_DNS_SERVER), ESP_NETIF_DNS_BACKUP)
    //    );
    return ESP_OK;
}

/******************************************************************************
    event_handler
*//**
    @brief Handles connection events.
******************************************************************************/
static void
event_handler(
    void* arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void* event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_START:
            ESP_LOGI(TAG, "STA_START");
            esp_wifi_connect();
            break;

        case WIFI_EVENT_STA_CONNECTED:
            if (!current_cfg.use_dhcp)
            {
                ESP_LOGI(TAG, "Station connected, using static ip: %s", current_cfg.ip);
                set_static_ip(arg);
                break;
            }
            ESP_LOGI(TAG, "Station connected, using dhcp");
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "STA_DISCONNECTED");
            if (retry_count < MAXIMUM_RETRY)
            {
                retry_count++;
                ESP_LOGI(TAG, "Retry connect to the AP... %u/%u", retry_count, MAXIMUM_RETRY);
                esp_wifi_connect();
            }
            else
            {
                xEventGroupSetBits(eventGrp, WIFI_FAILED_FLAG);
            }
            break;

        default:
            break;
        }
    }

    if (event_base == IP_EVENT)
    {
        switch (event_id)
        {
        case IP_EVENT_STA_GOT_IP:
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
            retry_count = 0;
            xEventGroupSetBits(eventGrp, WIFI_CONNECTED_FLAG);
            break;

        default:
            break;
        }
    }
}

/******************************************************************************
    [docimport WifiConnect_init]
*//**
    @brief Performs wifi station connect to AP.
    @param[in] cfg  Pointer to WifiConfig object.
******************************************************************************/
esp_err_t
WifiConnect_init(
    const char *ssid,
    const char *pass,
    uint8_t use_dhcp,
    const char *ip,
    const char *netmask,
    const char *gw)
{
    esp_err_t ret;

    current_cfg.use_dhcp = use_dhcp;
    memcpy(current_cfg.ssid, ssid, strlen(ssid));
    memcpy(current_cfg.password, pass, strlen(pass));
    memcpy(current_cfg.ip, ip, strlen(ip));
    memcpy(current_cfg.netmask, netmask, strlen(netmask));
    memcpy(current_cfg.gw, gw, strlen(gw));

    eventGrp = xEventGroupCreate();

    ret = esp_netif_init();
    RET_ON_ERROR_MSG(ret, "Error esp_netif_init()");

    ret = esp_event_loop_create_default();
    RET_ON_ERROR_MSG(ret, "Error esp_event_loop_create_default()");

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t initcfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&initcfg);
    RET_ON_ERROR_MSG(ret, "Error esp_wifi_init()");

    esp_event_handler_instance_t instance_any_id;
    ret = esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &event_handler,
        sta_netif,
        &instance_any_id);
    RET_ON_ERROR_MSG(ret, "Error esp_event_handler_instance_register()");

    esp_event_handler_instance_t instance_got_ip;
    ret = esp_event_handler_instance_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &event_handler,
        NULL,
        &instance_got_ip);
    RET_ON_ERROR_MSG(ret, "Error esp_event_handler_instance_register()");

    wifi_config_t wifi_config = { 0 };
    memcpy(wifi_config.sta.ssid, ssid, strlen(ssid));
    memcpy(wifi_config.sta.password, pass, strlen(pass));

    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    RET_ON_ERROR_MSG(ret, "Error esp_wifi_set_mode()");

    ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    RET_ON_ERROR_MSG(ret, "Error esp_wifi_set_config()");

    ret = esp_wifi_start();
    RET_ON_ERROR_MSG(ret, "Error esp_wifi_start()");

    /*  Waiting until either the connection is established (WIFI_CONNECTED_FLAG)
        or connection failed for the maximum number of re-tries (WIFI_FAILED_FLAG).
        The flags are set by event_handler() (see above)
    */
    EventBits_t flags = xEventGroupWaitBits(
        eventGrp,
        WIFI_CONNECTED_FLAG | WIFI_FAILED_FLAG,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY);

    /*  xEventGroupWaitBits() returns the flags before the call returned, hence
        we can test which event actually happened.
    */
    if (flags & WIFI_CONNECTED_FLAG)
    {
        ESP_LOGI(TAG, "connected to ap SSID: %s", ssid);
    }
    else if (flags & WIFI_FAILED_FLAG)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID: %s", ssid);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    return ESP_OK;
}
