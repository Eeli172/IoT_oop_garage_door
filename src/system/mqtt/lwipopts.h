#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Ei käyttöjärjestelmää (Pico SDK huolehtii tausta-ajoista)
#define NO_SYS                      1
#define LWIP_SOCKET                 0
#define LWIP_NETCONN                0

// Muisti- ja puskuriasetukset
#define MEM_LIBC_MALLOC             0
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    4000
#define MEMP_NUM_TCP_SEG            32
#define MEMP_NUM_ARP_QUEUE          10
#define PBUF_POOL_SIZE              24

// Verkkoprotokollat
#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    1
#define LWIP_DHCP                   1
#define LWIP_AUTOIP                 1
#define LWIP_DNS                    1
#define LWIP_TCP                    1
#define LWIP_UDP                    1

// TCP puskurit
#define TCP_MSS                     (1500 - 40)
#define TCP_WND                     (8 * TCP_MSS)
#define TCP_SND_BUF                 (8 * TCP_MSS)
#define TCP_SND_QUEUELEN            (4 * TCP_SND_BUF/TCP_MSS)

// MQTT Tuki
#define LWIP_MQTT                   1

// Tilastot pois päältä muistin säästämiseksi
#define LWIP_STATS                  0
#define LWIP_STATS_DISPLAY          0

#endif /* _LWIPOPTS_H */