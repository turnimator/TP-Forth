/*
 * log.h
 *
 *  Created on: Oct 2, 2024
 *      Author: Jan Atle Ramsli
 *
 */

#ifndef LOGG_H_
#define LOGG_H_
#ifdef DEBUG
#define logg(a,b) printf("%s: %s %s\n", __func__, a, b)
#else
#define logg(a,b) //
#endif

#endif /* LOGG_H_ */
