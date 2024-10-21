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
#define logg(a,b) printf("\n%s: %s( \"%s\" ) ", __func__, a, b)
#define loggssn(s1,s2,n) printf("\n%s: %s(%s)=%d ", __func__, s1,s2, n)
#else
#define logg(a,b) //
#define loggssn(s1,s2,n) //
#endif

#endif /* LOGG_H_ */
