/*******************************************************************************
 *  @file: WifiConnect.h
 *   
 *  @brief: Header for WifiConnect.
*******************************************************************************/
#ifndef WIFICONNECT_H
#define WIFICONNECT_H

#include <stdint.h>

/******************************************************************************
    [docexport WifiConnect_init]
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
    const char *gw);
#endif
