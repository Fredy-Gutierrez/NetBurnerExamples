#include <config_netobj.h>
#include <config_obj.h>
#include <dhcpd.h>
#include <dhcpinternals.h>
#include <fdprintf.h>
#include <http.h>
#include <httppost.h>
#include <iosys.h>
#include <netinterface.h>
#include <stdlib.h>
#include <tcp.h>

#include "MyAlloc.h"

class OneConfigRecord : public config_obj
{
   public:
    config_IPADDR4 addr_start{"0.0.0.0", "AddrStart"};
    config_IPADDR4 addr_end{"0.0.0.0", "AddrEnd"};
    config_IPADDR4 mask{"0.0.0.0", "Mask"};
    config_IPADDR4 gate{"0.0.0.0", "Gate"};
    config_IPADDR4 dns1{"0.0.0.0", "DNS1"};
    config_IPADDR4 dns2{"0.0.0.0", "DNS2"};
    config_chooser mode{"Mode", "Normal", "Normal,Duplicate,Off"};
    config_int duration{3600, "LeaseDurarion"};
    ConfigEndMarker;   // No new data members below this line

    OneConfigRecord(const char *name, const char *desc) : config_obj(name, desc){};
    OneConfigRecord(config_obj &owner, const char *name, const char *desc) : config_obj(owner, name, desc){};
    int count()
    {
        uint32_t s = (uint32_t)(IPADDR4)addr_start;
        uint32_t e = (uint32_t)(IPADDR4)addr_end;
        return (int)(e - s);
    };
    bool GetDhcpInfo(DHCP::DhcpInfo &infoBlock);
};

bool OneConfigRecord::GetDhcpInfo(DHCP::DhcpInfo &infoBlock)
{
    infoBlock.netmask = mask;                 // Netmask for the lease offered
    infoBlock.gateway = gate;                 // gateway to be offered
    infoBlock.dns_1 = dns1;                   // primary DNS Server
    infoBlock.dns_2 = dns1;                   // secondary DNS Server
    infoBlock.logServ = IPADDR4::NullIP();    // Syslog Server
    infoBlock.smtpServ = IPADDR4::NullIP();   // SMTP Server
    infoBlock.ntpServ = IPADDR4::NullIP();    // NTP Server
    infoBlock.domain_name = nullptr;          // Domain name for hosts
    infoBlock.hostname = nullptr;             // Hostname
    infoBlock.tftp_name = nullptr;            // tftp server name/dotted ip, null terminated
    infoBlock.bootfile = nullptr;             // tftp boot file, null terminated
    infoBlock.bonusLength = 0;                // Length of any additional options, pre written
    infoBlock.bonusOpts = nullptr;            // additional options, pre written

    return true;
}

config_MACADR NB_MAC_Prefix(appdata, "00:03:F4:00:00:00", "NB_MAC");
config_MACADR NB_MAC_Mask(appdata, "FF:FF:FF:00:00:00", "NB_MASK");
config_int DefNBMode(1, "DefNBMode");

OneConfigRecord ConfigPC(appdata, "DHCPConfig_PCs", "DHCP configuration for PCs");
OneConfigRecord Config1(appdata, "DHCPConfig_1", "DHCP configuration 1");
OneConfigRecord Config2(appdata, "DHCPConfig_2", "DHCP configuration 2");
OneConfigRecord Config3(appdata, "DHCPConfig_3", "DHCP configuration 3");

void DoConfigs(int sock, PCSTR url)
{
    IPADDR ipmine = InterfaceIP(GetFirstInterface());
    fdprintf(sock, "<A HREF=\"http://%I:20034/UI.html?Config/AppData/DHCPConfig_1\">Config 1</A><BR>\r\n", ipmine);
    fdprintf(sock, "<A HREF=\"http://%I:20034/UI.html?Config/AppData/DHCPConfig_2\">Config 2</A><BR>\r\n", ipmine);
    fdprintf(sock, "<A HREF=\"http://%I:20034/UI.html?Config/AppData/DHCPConfig_3\">Config 3</A><BR>\r\n", ipmine);
    fdprintf(sock, "<A HREF=\"http://%I:20034/UI.html?Config/AppData/DHCPConfig_PCs\">Config PC's</A><BR>\r\n", ipmine);
    fdprintf(sock, "<A HREF=\"http://%I:20034/UI.html?Config/AppData\">Everything</A><BR>\r\n", ipmine);
}

bool bIsNetBurner(MACADR &ma)
{
    for (int i = 0; i < 6; i++)
    {
        uint8_t bv = ma.GetByte(i);
        uint8_t bm = ((MACADR)NB_MAC_Mask).GetByte(i);
        uint8_t bt = ((MACADR)NB_MAC_Prefix).GetByte(i);
        if ((bv & bm) != (bt & bm)) return false;
    }
    return true;
}

struct AllocRecord
{
    IPADDR4 ip;
    MACADR ma;
    uint32_t Expiry;   // Zero for not currently active
    DHCP::LeaseState_t offer_state;
    int state;   // Used to set state....
    bool GetDhcpInfo(DHCP::DhcpInfo &infoBlock);
    void print();
};

void AllocRecord::print()
{
    printf("R[");
    ma.print();
    printf("]:%hI\r\n", ip);
}

const int NumOfAllocRecord = ((8192 / sizeof(AllocRecord)) - 20);

static AllocRecord ARecords[NumOfAllocRecord];
static int alloc_used;

void OutputModeSelect(int sock, int val, const char *name)
{
    fdprintf(sock, "<select name=\"%s\">\r\n", name);

    if (val == 1) { fdprintf(sock, "<option value=\"1\" selected>Mode1</option>\r\n"); }
    else
    {
        fdprintf(sock, "<option value=\"1\">Mode1</option>\r\n");
    }

    if (val == 2) { fdprintf(sock, "<option value=\"2\" selected>Mode 2</option>\r\n"); }
    else
    {
        fdprintf(sock, "<option value=\"2\">Mode 2</option>\r\n");
    }

    if (val == 3) { fdprintf(sock, "<option value=\"3\" selected>Mode 3</option>\r\n"); }
    else
    {
        fdprintf(sock, "<option value=\"3\">Mode 3</option>\r\n");
    }

    fdprintf(sock, "</select>\r\n");
}

void OutputModeSelect(int sock, int val, int index)
{
    char buffer[20];
    siprintf(buffer, "Mode_%d", index);
    OutputModeSelect(sock, val, buffer);
}

void ListNetBurners(int sock, PCSTR url)
{
    fdprintf(sock, "Default:");
    OutputModeSelect(sock, (int)DefNBMode, "defmode");
    fdprintf(sock, "<BR>");
    fdprintf(sock, "<input type = \"submit\" name = \"submit\" value = \"SetAllToDefault\"><BR><BR>");

    fdprintf(sock, "<b>DHCP NetBurner Device Clients:</b><BR>");
    if (alloc_used > 0)
    {
        for (int i = 0; i < alloc_used; i++)
        {
            if ((!ARecords[i].ma.IsNull()) && (bIsNetBurner(ARecords[i].ma)))
            {
                fdprintf(sock, "Entry [%d] = %hI [", i, ARecords[i].ip);
                ARecords[i].ma.fdprint(sock);
                fdprintf(sock, "]:State:");
                OutputModeSelect(sock, ARecords[i].state, i);
                fdprintf(sock, "<BR>\r\n");
            }
        }
    }
    else
    {
        fdprintf(sock, "None");
    }

    fdprintf(sock, "<BR>");

    fdprintf(sock, "<input type = \"submit\" name = \"submit\" value = \"Submit\">");
}

void ListOther(int sock, PCSTR url)
{
    fdprintf(sock, "DHCP Clients:<BR>\r\n");
    for (int i = 0; i < alloc_used; i++)
    {
        if ((!ARecords[i].ma.IsNull()) && (!bIsNetBurner(ARecords[i].ma)))
        {
            fdprintf(sock, "Entry [%d] = %hI [", i, ARecords[i].ip);
            ARecords[i].ma.fdprint(sock);
            fdprintf(sock, "]<BR>\r\n");
        }
    }

    fdprintf(sock, "<BR>End<BR>\r\n");
}

class SpecialAllocator : public DHCP::LeaseAllocator
{
   public:
    SpecialAllocator();
    virtual bool OfferLease(DHCP::DhcpLeaseRequest *pLease, int intfNum);
    virtual bool RequestLease(DHCP::DhcpLeaseRequest *pLease, int intfNum);
    virtual bool ReleaseLease(DHCP::DhcpLeaseRequest *pLease, int intfNum);
    virtual bool LeaseValid(DHCP::DhcpLeaseRequest *pLease, int intfNum);
    virtual bool GetDhcpInfo(DHCP::DhcpInfo &infoBlock, MACADR &client_mac, int intfNum);

    virtual uint32_t GetLeaseTime() { return 3600; };   // One hour
    virtual bool AddInterface(int intfNum) { return true; }
    virtual void RemoveInterface(int intfNum) { return; }
    virtual bool IsRegisteredInterface(int intfNum) { return true; }
    virtual bool GetLeaseData(DHCP::DhcpLeaseData *data) { return false; }
};

AllocRecord *FindAlloc(MACADR &ma)
{
    int i = 0;
    while (((!ARecords[i].ma.IsNull())) && (ARecords[i].ma != ma))
    {
        i++;
    }
    if (ARecords[i].ma.IsNull()) { return nullptr; }
    return ARecords + i;
}

AllocRecord *GetSetNew(MACADR &ma, IPADDR4 i4)
{
    AllocRecord *ar;
    ar = ARecords + alloc_used++;
    ar->ma = ma;
    ar->offer_state = DHCP::LEASE_OFFERED;

    OneConfigRecord *pr = nullptr;
    bool bIsNB = bIsNetBurner(ma);

    if (bIsNB)
    {
        ar->state = DefNBMode;
        switch (ar->state)
        {
            case 1: pr = &Config1; break;
            case 2: pr = &Config2; break;
            case 3: pr = &Config3; break;
        }

        if (pr->mode == "Off")
        {
            alloc_used--;
            ar->ma = ENET_ZERO;
            return nullptr;
        }
        if (pr->mode == "Duplicate")
        {
            ar->ip = pr->addr_start;
            return ar;
        }
        // Ok Mode is normal
    }
    else
    {
        pr = &ConfigPC;
    }

    if (i4.NotNull())
    {
        if ((i4 >= (IPADDR4)(pr->addr_start)) && (i4 <= (IPADDR4)(pr->addr_end)))
        {
            int i = 0;
            for (i = 0; i < alloc_used; i++)
            {
                if (ARecords[i].ip == i4) break;
            }

            if (ARecords[i].ip != i4)
            {
                ar->ip = i4;
                return ar;
            }
        }
    }
    IPADDR4 it = pr->addr_start;

    for (int i = 0; i < alloc_used; i++)
    {
        if (bIsNetBurner(ARecords[i].ma) == bIsNB)
        {
            if (it < ARecords[i].ip) it = ARecords[i].ip;
        }
    }

    if (it.IsNull())
    {
        it = pr->addr_start;
    }
    else
    {
        uint32_t ui = (uint32_t)it;
        ui++;
        it = ui;
        if (it > pr->addr_end)
        {   // We are off the end
            uint32_t oldest = 0;
            for (int i = 0; i < alloc_used; i++)
            {
                if (bIsNetBurner(ARecords[i].ma) == bIsNB)
                {
                    if (ARecords[oldest].Expiry < ARecords[i].Expiry) oldest = i;
                }
            }
            // Ok the record might be reusable
            if ((ARecords[oldest].Expiry == 0) || (ARecords[oldest].Expiry < Secs))
            {
                alloc_used--;
                ar = ARecords + oldest;
                ar->ma = ma;
                return ar;
            }
            return nullptr;
        }
        ar->ip = it;
    }
    return ar;
}

bool AllocRecord::GetDhcpInfo(DHCP::DhcpInfo &infoBlock)
{
    if (bIsNetBurner(ma))
    {
        switch (state)
        {
            case 1: return Config1.GetDhcpInfo(infoBlock);
            case 2: return Config2.GetDhcpInfo(infoBlock);
            case 3: return Config3.GetDhcpInfo(infoBlock);
            default: return Config1.GetDhcpInfo(infoBlock);
        }
    }
    else
    {
        return ConfigPC.GetDhcpInfo(infoBlock);
    }
    return false;
}

static SpecialAllocator MyAlloc;
DHCP::Server MyServer;

SpecialAllocator::SpecialAllocator() {}

bool SpecialAllocator::OfferLease(DHCP::DhcpLeaseRequest *pLease, int intfNum)
{
    AllocRecord *ar = FindAlloc(pLease->mac);
    iprintf("Offer Lease IP = %hI ar=%p\r\n", pLease->ip, ar);

    if (!ar) { ar = GetSetNew(pLease->mac, pLease->ip); }

    if (ar)
    {
        pLease->ip = ar->ip;
        pLease->duration = GetLeaseTime();
        ar->offer_state = DHCP::LEASE_OFFERED;
        ar->Expiry = Secs;
        iprintf("Offer Lease:");
        ar->print();
        return true;
    }

    iprintf("AR is null Offer fail\r\n");
    return false;
}

bool SpecialAllocator::RequestLease(DHCP::DhcpLeaseRequest *pLease, int intfNum)
{
    iprintf("Requesting IP = %hI\r\n", pLease->ip);
    AllocRecord *ar = FindAlloc(pLease->mac);
    if ((!ar) || (pLease->ip != ar->ip))
    {
        if (ar) ar->offer_state = DHCP::LEASE_OPEN;
        pLease->ip = 0x00000000;
        iprintf("Request fail\r\n");
        return false;
    }

    ar->offer_state = DHCP::LEASE_TAKEN;
    pLease->duration = GetLeaseTime();
    ar->Expiry = Secs + pLease->duration;
    iprintf("Request Lease:");
    ar->print();
    return true;
}

bool SpecialAllocator::ReleaseLease(DHCP::DhcpLeaseRequest *pLease, int intfNum)
{
    AllocRecord *ar = FindAlloc(pLease->mac);
    if ((!ar) || (pLease->ip != ar->ip)) return false;
    if (ar) ar->offer_state = DHCP::LEASE_OPEN;
    return true;
}

bool SpecialAllocator::LeaseValid(DHCP::DhcpLeaseRequest *pLease, int intfNum)
{
    AllocRecord *ar = FindAlloc(pLease->mac);
    if ((!ar) || (pLease->ip != ar->ip)) return false;
    return true;
}

bool SpecialAllocator::GetDhcpInfo(DHCP::DhcpInfo &infoBlock, MACADR &client_mac, int intfNum)
{
    AllocRecord *ar = FindAlloc(client_mac);
    if (!ar) return false;
    return ar->GetDhcpInfo(infoBlock);
}

extern DHCPProcessFunction *pDHCPServerProcessFunction FAST_IP_VAR;

bool AddMyDHCPServer(int intf)
{
    DHCP::DhcpInfo serverInfo;
    memset(&serverInfo, 0x00, sizeof(DHCP::DhcpInfo));
    serverInfo.netmask = 0xFFFFFF00;

    IPADDR4 i4;

    MyAlloc.AddInterface(intf);
    MyServer.AddLeaseAllocator(&MyAlloc);
    pDHCPServerProcessFunction = DHCP::Server::ProcessMessage;

    return true;
}

/**
 * @brief A POST callback for the NbForm post request. This callback is called for each field
 * of a post form.
 *
 * @param sock HTTP socket.
 * @param event The kind of post event that is currently being handled with this callback.
 * @param pName The name of the post element that is currently being handled.
 * @param pValue The value of the post element that is currently being handled.
 */
void HandleNbFormPost(int sock, PostEvents event, const char *pName, const char *pValue)
{
    iprintf("Post Data, name: %s     value: %s\r\n", pName, pValue);
    static bool setAll = false;

    if (event == eStartingPost) { setAll = false; }
    if (event == eVariable)
    {
        if (strcmp(pName, "defmode") == 0)
        {
            iprintf("     Default Mode: %d\r\n", atoi(pValue));
            DefNBMode = atoi(pValue);
        }
        else if (strcmp(pName, "submit") == 0)
        {
            if (strcmp(pValue, "SetAllToDefault") == 0)
            {
                iprintf("     Setting all\r\n");
                setAll = true;
                for (int i = 0; i < alloc_used; i++)
                {
                    if (bIsNetBurner(ARecords[i].ma)) { ARecords[i].state = DefNBMode; }
                }
            }
        }
        else if (!setAll)
        {
            iprintf("     Setting individual\r\n");
            for (int i = 0; i < alloc_used; i++)
            {
                if (bIsNetBurner(ARecords[i].ma))
                {
                    char name[20];
                    siprintf(name, "Mode_%d", i);
                    if (strcmp(pName, name) == 0) { ARecords[i].state = DefNBMode; }
                }
            }
        }
    }
    if (event == eEndOfPost)
    {
        iprintf("End of post data extraction\r\n");
        RedirectResponse(sock, "NetBurners.html");
    }
}

HtmlPostVariableListCallback gHandlePost("nbform*", HandleNbFormPost);
