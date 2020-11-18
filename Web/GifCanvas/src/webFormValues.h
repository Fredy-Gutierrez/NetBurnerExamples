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

#ifndef _WEB_FORM_VALUES_H_
#define _WEB_FORM_VALUES_H_
#pragma once

#define DEF_WIDTH (250)
#define DEF_HT (250)
#define DEF_FRED (150)
#define DEF_FGREEN (0)
#define DEF_FBLUE (250)
#define DEF_BRED (250)
#define DEF_BGREEN (150)
#define DEF_BBLUE (0)
#define DEF_TRED (255)
#define DEF_TGREEN (255)
#define DEF_TBLUE (255)

/* This class defines a BAR size width etc... */

class WebFormValues
{
   public:
    uint16_t m_ht = DEF_HT;
    uint16_t m_wid = DEF_WIDTH;
    uint8_t m_fillRed = DEF_FRED;
    uint8_t m_fillGreen = DEF_FGREEN;
    uint8_t m_fillBlue = DEF_FBLUE;
    uint8_t m_borderRed = DEF_BRED;
    uint8_t m_borderGreen = DEF_BGREEN;
    uint8_t m_borderBlue = DEF_BBLUE;
    uint8_t m_textRed = DEF_TRED;
    uint8_t m_textGreen = DEF_TGREEN;
    uint8_t m_textBlue = DEF_TBLUE;

    WebFormValues();
    WebFormValues(PCSTR url);
    void WriteUrl(int sock, PCSTR Prefix);
};

#endif /* _BAR_DEFINITION_H_ */
