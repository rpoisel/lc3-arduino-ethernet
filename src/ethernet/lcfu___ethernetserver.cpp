#ifndef LC_PROT_LCFU___ETHERNETSERVER__C
#define LC_PROT_LCFU___ETHERNETSERVER__C

#include <lcfu___ethernetserver.h>
#include <lcfu___ethernetbegin.h>

extern "C" {
#include <lcfu___getnumberofleds.h>
}

#include <LC3Globals.h>

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>

#include <stdint.h>

static LC_TD_USINT getNumberOfLeds(struct _lcoplck_epdb_1_impl* pEPDB);

static EthernetServer* server;

/*                            Functions                        */
void  lcfu___ETHERNETBEGIN(LC_TD_Function_ETHERNETBEGIN* LC_this, LC_TD_DataType_ADDR_MAC LC_VD_MAC, LC_TD_DataType_ADDR_IP LC_VD_IP, LC_TD_DataType_ADDR_PORT LC_VD_TCP_PORT, LC_TD_BOOL LC_VD_ENABLE, struct _lcoplck_epdb_1_impl* pEPDB)
{
    if (!LC_VD_ENABLE)
    {
        return;
    }
    IPAddress ip(LC_VD_IP[0], LC_VD_IP[1], LC_VD_IP[2], LC_VD_IP[3]);
    Ethernet.begin(LC_VD_MAC, ip);
    server = new EthernetServer(LC_VD_TCP_PORT);
    if (!server)
    {
        LC_this->LC_VD_SUCCESS = LC_EL_false;
        LC_this->LC_VD_ENO = LC_EL_false;
        return;
    }
    server->begin();
}

void  lcfu___ETHERNETSERVER(LC_TD_Function_ETHERNETSERVER* LC_this, LC_TD_BOOL LC_VD_ENABLE, struct _lcoplck_epdb_1_impl* pEPDB)
{
    if (!LC_VD_ENABLE)
    {
        return;
    }
    EthernetClient client = server->available();
    if (!client)
    {
        LC_this->LC_VD_ENO = LC_EL_false;
        return;
    }
    boolean currentLineIsBlank = true;
    while(client.connected())
    {
        if (!client.available())
        {
            continue;
        }
        auto c = client.read();
        if (c == '\n' && currentLineIsBlank)
        {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");  // the connection will be closed after completion of the response
            client.println("Refresh: 1");  // refresh the page automatically every 1 sec
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.print("OutLedPattern: ");
            uint64_t outLedPattern = LC_GV_READ_ELEM(pEPDB,LC_VD_OUTLEDPATTERN,LC_TD_LWORD);
            for (size_t idx = getNumberOfLeds(pEPDB); idx > 0; idx--)
            {
                client.print(outLedPattern & (1 << idx - 1) ? "1" : "0");
            }
            client.println();
            client.println("</html>");
            break;
        }
        if (c == '\n')
        {
            currentLineIsBlank = true;
        }
        else if(c != '\r')
        {
            currentLineIsBlank = false;
        }
    }

    delay(1);
    client.stop();
}

static LC_TD_USINT getNumberOfLeds(struct _lcoplck_epdb_1_impl* pEPDB)
{
    LC_TD_Function_GETNUMBEROFLEDS lFunction_Data;
    LC_INIT_Function_GETNUMBEROFLEDS(&lFunction_Data);
    lFunction_Data.LC_VD_ENO = LC_EL_true;
    lcfu___GETNUMBEROFLEDS(&lFunction_Data, pEPDB);
    return lFunction_Data.LC_VD_GETNUMBEROFLEDS;
}

#endif
