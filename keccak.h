/**************************
 * sha-3 (keccak) hash    *
 * written by: raciborski *
 **************************/

#ifndef KECCAK_H
#define KECCAK_H

#include <stdint.h>

void sha3_file(uint8_t *hash, int dir_fd, const char *name);

#endif
