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

#include <iosys.h>

#include "webFormValues.h"

uint16_t GetParam(PCSTR url, uint8_t offset, uint8_t digits, uint16_t defaultv);

WebFormValues::WebFormValues() {}

/**
 *  WebFormValues Constructor
 *
 *  Values are assigned from the URL passed in
 */
WebFormValues::WebFormValues(PCSTR url)
{
    m_ht = GetParam(url, 0, 3, DEF_HT);
    m_wid = GetParam(url, 3, 3, DEF_WIDTH);
    m_fillRed = GetParam(url, 6, 2, DEF_FRED);
    m_fillGreen = GetParam(url, 8, 2, DEF_FGREEN);
    m_fillBlue = GetParam(url, 10, 2, DEF_FBLUE);
    m_borderRed = GetParam(url, 12, 2, DEF_BRED);
    m_borderGreen = GetParam(url, 14, 2, DEF_BGREEN);
    m_borderBlue = GetParam(url, 16, 2, DEF_BBLUE);
    m_textRed = GetParam(url, 18, 2, DEF_TRED);
    m_textGreen = GetParam(url, 20, 2, DEF_TGREEN);
    m_textBlue = GetParam(url, 22, 2, DEF_TBLUE);
}

/**
 *  WriteUrl
 *
 *  Given a WebFormValues definition write out the URL par that describes it
 */
void WebFormValues::WriteUrl(int sock, PCSTR Prefix)
{
    char buffer[80];
    sniprintf(buffer, 80, "%s%03X%03X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", Prefix, m_ht, m_wid, m_fillRed, m_fillGreen, m_fillBlue,
              m_borderRed, m_borderGreen, m_borderBlue, m_textRed, m_textGreen, m_textBlue);

    writestring(sock, buffer);
}
