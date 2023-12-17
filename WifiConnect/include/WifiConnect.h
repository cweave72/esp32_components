/*******************************************************************************
 *  @file: WifiConnect.h
 *   
 *  @brief: Header for WifiConnect.
*******************************************************************************/
#ifndef WIFICONNECT_H
#define WIFICONNECT_H

#include <stdint.h>

/** @brief Configuration object for WifiConnect.
*/
typedef struct WifiConfig
{
    uint8_t ssid[32];
    uint8_t password[64];
    bool use_dhcp;
    char *ip;
    char *netmask;
    char *gw;
} WifiConfig;

/******************************************************************************
    [docexport WifiConnect_init]
*//**
    @brief Performs wifi station connect to AP.
    @param[in] cfg  Pointer to WifiConfig object.
******************************************************************************/
int
WifiConnect_init(WifiConfig *cfg);
#endif
