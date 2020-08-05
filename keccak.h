/**************************
 * sha-3 (keccak) hash    *
 * written by: raciborski *
 **************************/

#include <stdint.h>

void sha3_file(uint8_t *hash, int dir_fd, const char *name);