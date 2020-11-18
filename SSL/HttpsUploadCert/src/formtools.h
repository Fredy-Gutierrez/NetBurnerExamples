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

/**
 * @brief ShowIP2Sock
 *
 * @param sock
 * @param ip
 */
void ShowIP2Sock(int sock, IPADDR ip);

/**
 *   Functions to manage HTML form creation.
 */

/**
 * @brief Output a selection
 *
 * @param sock
 * @param name
 * @param selnum
 * @param list
 *
 * Item 1 = First selection item!
 */
void FormOutputSelect(int sock, const char *name, int selnum, const char **list);

/**
 * @brief Output a selection value
 *
 * @param sock
 * @param name
 * @param selnum
 * @param labellist
 * @param valuelist
 * @param onclicklist
 */
void FormOutputSelectValueOnClick(int sock,
                                  const char *name,
                                  int selnum,
                                  const char **labellist,
                                  const char **valuelist,
                                  const char **onclicklist);

/**
 * @brief Output a check box
 *
 * @param sock
 * @param name
 * @param checked
 */
void FormOutputCheckbox(int sock, const char *name, BOOL checked);

/**
 * @brief Output an input box
 *
 * @param sock
 * @param name
 * @param siz
 * @param val
 */
void FormOutputInput(int sock, const char *name, int siz, const char *val);

/**
 * @brief Output an input box for numbers
 *
 * @param sock
 * @param name
 * @param siz
 * @param val
 */
void FormOutputNumInput(int sock, const char *name, int siz, int val);

/**
 * @brief Output an input box for IP addresses
 *
 * @param sock
 * @param name
 * @param ip
 */
void FormOutputIPInput(int sock, const char *name, IPADDR ip);

#endif /* _FORM_TOOL_H_ */
