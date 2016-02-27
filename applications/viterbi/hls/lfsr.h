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
void srandom_fake(unsigned int new_seed);
int random_fake();

#endif /* LFSR_H_ */
