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

#ifndef _FORM_TOOL_H_
#define _FORM_TOOL_H_

void ShowIP2Sock(int sock, IPADDR ip);

/* -----------------------------------------------------------------------------
 *   Functions to manage HTML form creation.
 * -------------------------------------------------------------------------- */

// Output a selection
// Item 1 = First selection item!
void FormOutputSelect(int sock, const char *name, int selnum, const char **list);
void FormOutputSelectValueOnClick(int sock,
                                  const char *name,
                                  int selnum,
                                  const char **labellist,
                                  const char **valuelist,
                                  const char **onclicklist);

// Output a check box
void FormOutputCheckbox(int sock, const char *name, BOOL checked);

// Output an input box
void FormOutputInput(int sock, const char *name, int siz, const char *val);

// Output an input box for numbers
void FormOutputNumInput(int sock, const char *name, int siz, int val);

// Output an input box for IP addresses
void FormOutputIPInput(int sock, const char *name, IPADDR ip);

/* -----------------------------------------------------------------------------
 *   Functions to manage HTML data extraction.
 *--------------------------------------------------------------------------- */

// Extract an IP address from the post data
IPADDR FormExtractIP(const char *name, char *pData, IPADDR def_val);

// Extract a number from the post data
long FormExtractNum(const char *name, char *pData, long def_val);

// Extract a check box state from the post data
BOOL FormExtractCheck(const char *name, char *pData, BOOL def_val);

// Extract a selection from a select box
// Item 1 = First selection item!
int FormExtractSel(const char *name, char *pdata, const char **pList, int defsel);

#endif /* _FORM_TOOL_H_ */
