/*
 * lfsr.h
 *
 *  Created on: Feb 25, 2016
 *      Author: Sam
 */

#ifndef LFSR_H_
#define LFSR_H_

unsigned int shift_lfsr(unsigned int v);

// Mimicking the Linux interface.
void srandom(unsigned int new_seed);
unsigned int random();

#endif /* LFSR_H_ */
