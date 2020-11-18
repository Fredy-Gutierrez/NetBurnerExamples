/* Revision: 3.2.0 */

/******************************************************************************
* Copyright 1998-2020 NetBurner, Inc.  ALL RIGHTS RESERVED
*
*    Permission is hereby granted to purchasers of NetBurner Hardware to use or
*    modify this computer program for any use as long as the resultant program
*    is only executed on NetBurner provided hardware.
*
*    No other rights to use this program or its derivatives in part or in
*    whole are granted.
*
*    It may be possible to license this or other NetBurner software for use on
*    non-NetBurner Hardware. Contact sales@Netburner.com for more information.
*
*    NetBurner makes no representation or warranties with respect to the
*    performance of this computer program, and specifically disclaims any
*    responsibility for any damages, special or consequential, connected with
*    the use of this program.
*
* NetBurner
* 5405 Morehouse Dr.
* San Diego, CA 92121
* www.netburner.com
******************************************************************************/

// NB Definitions
#include <predef.h>

// NB Libs
#include <hal.h>
#include <ctype.h>
#include <init.h>
#include <ipv6/dhcpv6_internal.h>
#include <nbrtos.h>
#include <netinterface.h>
#include <stdio.h>
#include <stdlib.h>
#include <system.h>

#include "ip_util.h"

const char *AppName = "IPv6-DHCPv6";

/*-----------------------------------------------------------------------------
 * DHCPv6 Client callbacks
 *----------------------------------------------------------------------------*/

// The callbacks are in the NB::V6::DHCPv6 namespace
namespace NB
{
namespace V6
{
namespace DHCPv6
{
/**
 * @define Returns the additional options you wish to receive from the server.
 *         The returned list *must* be NULL terminated. Do *NOT* return an array created on the stack.
 *
 * @param None
 *
 * @return Pointer to Option_ID
 *
 */

/*----------------------------------------------------------------------------
 * pAddOROCB -
 *      Returns the additional options you wish to receive from the server
 *      The returned list *must* be NULL terminated.
 *          Do *NOT* return an array created on the stack.
 *----------------------------------------------------------------------------*/
extern Option_ID *(*pAddOROCB)(int ifNum);

/*----------------------------------------------------------------------------
 * pRequestOptionCB -
 *      Callback allows you set additional options for the message
 *      You can determine the message type by calling GetType on the message.
 *          Ex: if (msg.GetType() == NB::V6:DHCPv6::MSG_REQUEST)
 *----------------------------------------------------------------------------*/
extern void (*pRequestOptionCB)(int ifNum, DHCPv6Message &msg);

/*----------------------------------------------------------------------------
 * pReplyCB -
 *      Called upon receiving Reply messages that have been verified and processed
 *      for options that the system desires.
 *----------------------------------------------------------------------------*/
extern void (*pReplyCB)(int ifNum, DHCPv6Message &msg);

}   // namespace DHCPv6
}   // namespace V6
}   // namespace NB

// We can add any option we want to receive here, even those already present
// in the original set. The library wil ensure that no duplicates actually get
// sent on the wire.
NB::V6::DHCPv6::Option_ID extraOptions[] = {NB::V6::DHCPv6::OPT_PREFERENCE, NB::V6::DHCPv6::OPT_DNS_SERVERS,
                                            NB::V6::DHCPv6::OPT_DOMAIN_LIST, NB::V6::DHCPv6::OPT_NULL};

NB::V6::DHCPv6::Option_ID *AddORO(int ifNum)
{
    return extraOptions;
}

void GetReply(int ifNum, NB::V6::DHCPv6::DHCPv6Message &msg)
{
    iprintf("Reply Callback\n");

    // If you wish to see the full DHCPv6 messages, turn on DHCP_DEBUG in 'dhcpv6_internal.h'
    // and recompile the library.
    DHCPv6_DBSHOW(msg);
}

// Here is where we would add options to the sent requests, if we were sending any
void AddSendOption(int ifNum, NB::V6::DHCPv6::DHCPv6Message &msg)
{
    //    uint32_t namelen = strlen(pDHCPOfferName);
    //    union {
    //        NB::V6::DHCPv6::Opt::FQDN fqdn;
    //        char buf[200];
    //    } opt;
    //    opt.fqdn.id = NB::V6::DHCPv6::OPT_CLIENT_FQDN;
    //    opt.fqdn.flags = 0x00;
    //    opt.fqdn.len = namelen + sizeof(NB::V6::DHCPv6::Option) - 2;
    //    opt.fqdn.namelen = namelen;
    //    strcpy(opt.fqdn.name, pDHCPOfferName);
    //    msg.AddOption(opt.fqdn);
}

extern const char PlatformName[];
char DHCPNameBuffer[200];

/*-----------------------------------------------------------------------------
 * User Main
 *----------------------------------------------------------------------------*/
void UserMain(void *pd)
{
    bool autoDeviceName = false;

    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 10);
    StartHttp();

    // A DHCP offer name is not a requirement, but for this example we will use one.
    // The DEVICE_NAME in the configuration record is used as part of the DHCP name.
    // If DEVICE_NAME is not set, create one and reboot the device so it takes effect.
    int32_t ifNumber = GetFirstInterface();
    InterfaceBlock *pIfBlock = GetInterfaceBlock(ifNumber);
    if (((NBString)(pIfBlock->device_name)).empty() && autoDeviceName)
    {
        iprintf("DEVICE_NAME not set, creating one for the DHCP offer. \r\nYour device will automatically reboot\r\n");
        MACADR macAddress = InterfaceMAC(ifNumber);
        siprintf(DHCPNameBuffer, "%s_%s_%02X%02X%02X", "NB", PlatformName, macAddress.GetByte(3), macAddress.GetByte(4),
                 macAddress.GetByte(5));
        pIfBlock->device_name = DHCPNameBuffer;
        SaveConfigToStorage();
        OSTimeDly(TICKS_PER_SECOND * 5);
        ForceReboot();
    }

    iprintf("DHCP Offer Name: %s\n", pIfBlock->device_name.c_str());

    // set our callbacks
    NB::V6::DHCPv6::pAddOROCB = AddORO;
    NB::V6::DHCPv6::pRequestOptionCB = AddSendOption;
    NB::V6::DHCPv6::pReplyCB = GetReply;

    iprintf("Application: %s\r\nNNDK Revision: %s\r\n", AppName, GetReleaseTag());
    iprintf("Type the enter key to display address information\r\n");
    iprintf("Note that routable addresses may take a few second to receive\r\n");

    while (1)
    {
        showIpAddressesSerial();
        getchar();
        iprintf("\r\n\r\n");
    }
}
