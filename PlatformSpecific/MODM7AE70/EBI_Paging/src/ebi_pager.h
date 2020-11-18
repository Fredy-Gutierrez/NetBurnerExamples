#ifndef  __EBI_PAGER_H
#define  __EBI_PAGER_H

#include <predef.h>
#include <pins.h>

extern volatile uint8_t ebi_0_base[];
extern volatile uint8_t ebi_1_base[];
extern volatile uint8_t ebi_2_base[];
extern volatile uint8_t ebi_3_base[];

/**
 * @brief Enables automatic Page Fault handling and configures a group of GPIOs
 *  to act as Page Select address lines. The GPIOs will automatically update on
 *  page faults such that page faults are invisible to normal code, allowing
 *  normal code to treat the paged area as a contiguous segment.
 *
 * @param csNum The EBI chip select to configure page handling for.
 * @param pageSize The size in bytes of each page attached to the given chip select.
 * @param pageSelectPins An array of PinIOs for the GPIO page select pins.
 *  Note: this array is least significant address line first.
 * @param pageSelectPinCount How many pins are in the pageSelectPins array
 * */

void ConfigAddressPager(uint32_t csNum, uint32_t pageSize,
            PinIO *pageSelectPins, uint32_t pageSelectPinCount);

#endif   /* ----- #ifndef __EBI_PAGER_H  ----- */
