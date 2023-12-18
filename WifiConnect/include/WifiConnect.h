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
    char ssid[32];
    char password[64];
    uint8_t use_dhcp;
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
