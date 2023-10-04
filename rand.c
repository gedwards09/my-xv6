/*
 * Simple pseudo-random number implementation
 * 
 * @author Greg Edwards
 * @version 1.0
 * @revisions
 *   GJE p2b - Created
 */

#include "rand.h"

static int Seed = 999999;
const int Modulus = 0x7FFFFFFF;
const int Multiplier = 0x138D395;
const int Increment = 12345;

int rand0()
{
	Seed = (Multiplier * Seed + Increment) & Modulus;
	return Seed;
}
