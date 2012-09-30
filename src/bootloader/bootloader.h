/*
 * bootloader.h
 *
 *  Created on: 27.01.2012
 *      Author: matthias
 */

#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#define BOOTLOADER_REG (GPIOR1)

#define BOOTLOADER_EXECUTED (BOOTLOADER_REG == 1)
#define SET_BOOTLOADER_EXECUTED (BOOTLOADER_REG = 1)

#define BOOTLOADER_EXECUTE() {wdt_enable(WDTO_15MS); while(1) {}; }

#endif /* BOOTLOADER_H_ */
