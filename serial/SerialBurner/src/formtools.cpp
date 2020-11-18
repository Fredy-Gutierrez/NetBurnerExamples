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

/* NB Library Definitions */
#include <http.h>
#include <iosys.h>
#include <stdlib.h>
#include <string.h>
#include <utils.h>

/* HTML extract maximum size */
#define FORM_TOOLS_HTML_EXTRACT_SIZE (128)

char tbuf[FORM_TOOLS_HTML_EXTRACT_SIZE];

////////////////////////////////////////////////////////////////////////////////
// ShowIP2Sock()
//
void ShowIP2Sock(int sock, IPADDR ip)
{
    puint8_t ipb = (puint8_t)&ip;
    sniprintf(tbuf, FORM_TOOLS_HTML_EXTRACT_SIZE, "(%d.%d.%d.%d)", (int)ipb[0], (int)ipb[1], (int)ipb[2], (int)ipb[3]);
    writestring(sock, tbuf);
}

////////////////////////////////////////////////////////////////////////////////
// FormOutputSelect()
//
void FormOutputSelect(int sock, const char *name, int selnum, const char **list)
{
    writestring(sock, "<select name=\"");
    writestring(sock, name);
    writestring(sock, "\" size=\"1\">");
    const char **lp = list;
    int n = 0;

    while (lp[n][0])
    {
        if ((n + 1) == selnum) { writestring(sock, "<option selected>"); }
        else
        {
            writestring(sock, "<option>");
        }

        writesafestring(sock, lp[n]);
        n++;
    }

    writestring(sock, "</select>");
}

////////////////////////////////////////////////////////////////////////////////
// FormOutputSelect()
//
void FormOutputSelectValueOnClick(int sock,
                                  const char *name,
                                  int selnum,
                                  const char **labellist,
                                  const char **valuelist,
                                  const char **onclicklist)
{
    writestring(sock, "<select name=\"");
    writestring(sock, name);
    writestring(sock, "\" size=\"1\">");
    const char **llp = labellist;
    const char **vlp = valuelist;
    const char **olp = onclicklist;
    int n = 0;

    while (llp[n][0])
    {
        if ((n + 1) == selnum)
        {
            writestring(sock, "<option selected value=\"");
            writestring(sock, vlp[n]);
            writestring(sock, "\" onclick=\"");
            writestring(sock, olp[n]);
            writestring(sock, "\"");
            writestring(sock, "\">");
        }
        else
        {
            writestring(sock, "<option value=\"");
            writestring(sock, vlp[n]);
            writestring(sock, "\" onclick=\"");
            writestring(sock, olp[n]);
            writestring(sock, "\"");
            writestring(sock, "\">");
        }

        writesafestring(sock, llp[n]);
        n++;
    }

    writestring(sock, "</select>");
}

////////////////////////////////////////////////////////////////////////////////
// FormOutputCheckbox()
//
void FormOutputCheckbox(int sock, const char *name, BOOL checked)
{
    writestring(sock, "<input type=\"checkbox\" name=\"");
    writestring(sock, name);

    if (checked) { writestring(sock, "\" VALUE=\"checked\" checked>"); }
    else
    {
        writestring(sock, "\" VALUE=\"checked\">");
    }
}

////////////////////////////////////////////////////////////////////////////////
// FormOutputInput()
//
void FormOutputInput(int sock, const char *name, int siz, const char *val)
{
    char buf[10];
    writestring(sock, "<input name=\"");
    writestring(sock, name);
    writestring(sock, "\" type=\"text\" size=\"");
    sniprintf(buf, 10, "%d", siz);
    writestring(sock, buf);
    writestring(sock, "\" value=\"");
    writesafestring(sock, val);
    writestring(sock, "\">");
}

////////////////////////////////////////////////////////////////////////////////
// FormOutputNumInput()
//
void FormOutputNumInput(int sock, const char *name, int siz, int val)
{
    char buf[20];
    sniprintf(buf, 20, "%d", val);
    FormOutputInput(sock, name, siz, buf);
}

////////////////////////////////////////////////////////////////////////////////
// FormOutputIPInput()
//
void FormOutputIPInput(int sock, const char *name, IPADDR ip)
{
    char buf[20];
    puint8_t ipb = (puint8_t)&ip;
    sniprintf(buf, 20, "%d.%d.%d.%d", (int)ipb[0], (int)ipb[1], (int)ipb[2], (int)ipb[3]);
    FormOutputInput(sock, name, 20, buf);
}

//////////////////////////////////////////////////////////////////////////////////
//// FormExtractIP()
////
//IPADDR FormExtractIP(const char *name, char *pData, IPADDR def_val)
//{
//    char rx_buf[40];
//
//    if (ExtractPostData(name, pData, rx_buf, 40)) { return AsciiToIp(rx_buf); }
//
//    return def_val;
//}
//
//////////////////////////////////////////////////////////////////////////////////
//// FormExtractNum()
////
//long FormExtractNum(const char *name, char *pData, long def_val)
//{
//    char rx_buf[40];
//
//    if (ExtractPostData(name, pData, rx_buf, 40)) { return atol(rx_buf); }
//
//    return def_val;
//}
//
//////////////////////////////////////////////////////////////////////////////////
//// FormExtractCheck()
////
//BOOL FormExtractCheck(const char *name, char *pData, BOOL def_val)
//{
//    char rx_buf[FORM_TOOLS_HTML_EXTRACT_SIZE];
//    volatile BOOL checked = def_val;
//
//    if (ExtractPostData(name, pData, rx_buf, FORM_TOOLS_HTML_EXTRACT_SIZE) > 0)
//    {
//        if ((rx_buf[0] == 'C') || (rx_buf[0] == 'c')) { checked = TRUE; }
//        else
//        {
//            checked = FALSE;
//        }
//    }
//
//    return checked;
//}
//
//////////////////////////////////////////////////////////////////////////////////
//// FormExtractSel()
////
//int FormExtractSel(const char *name, char *pData, const char **pList, int defsel)
//{
//    char rx_buf[100];
//
//    if (ExtractPostData(name, pData, rx_buf, 40))
//    {
//        int n = 0;
//
//        while (pList[n][0])
//        {
//            if (strcmp(pList[n], rx_buf) == 0) { return n + 1; }
//
//            n++;
//        }
//    }
//
//    return defsel;
//}
