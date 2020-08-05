/**************************
 * sha-3 (keccak) hash    *
 * written by: raciborski *
 **************************/

#define __USE_XOPEN2K8
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "keccak.h"

typedef struct keccak_t {
  uint32_t pos;
  union {
    uint8_t state_b[200];
    uint64_t state_q[25];
  };
} keccak_t;

static const uint64_t keccak_rndc[24] = {
  0x0000000000000001, 0x0000000000008082, 0x800000000000808a,
  0x8000000080008000, 0x000000000000808b, 0x0000000080000001,
  0x8000000080008081, 0x8000000000008009, 0x000000000000008a,
  0x0000000000000088, 0x0000000080008009, 0x000000008000000a,
  0x000000008000808b, 0x800000000000008b, 0x8000000000008089,
  0x8000000000008003, 0x8000000000008002, 0x8000000000000080,
  0x000000000000800a, 0x800000008000000a, 0x8000000080008081,
  0x8000000000008080, 0x0000000080000001, 0x8000000080008008};

static const uint32_t keccak_rotc[24] = {1,  3,  6,  10, 15, 21, 28, 36,
                                         45, 55, 2,  14, 27, 41, 56, 8,
                                         25, 43, 62, 18, 39, 61, 20, 44};

static const uint32_t keccak_piln[24] = {10, 7,  11, 17, 18, 3,  5,  16,
                                         8,  21, 24, 4,  15, 23, 19, 13,
                                         12, 2,  20, 14, 22, 9,  6,  1};

static uint64_t rotl64(uint64_t x, uint64_t y) {
  return (x << y) | (x >> (64 - y));
}

static void keccak_sum(keccak_t *self) {
  uint32_t n, i, k;
  uint64_t bc[5], bin, *st = self->state_q, tmp;

  for(n = 0; n < 24; n++) {
    for(i = 0; i < 5; i++)
      bc[i] = st[i] ^ st[i + 5] ^ st[i + 10] ^ st[i + 15] ^ st[i + 20];

    for(i = 0; i < 5; i++) {
      bin = bc[(i + 4) % 5] ^ rotl64(bc[(i + 1) % 5], 1);
      for(k = 0; k < 25; k += 5)
        st[i + k] ^= bin;
    }

    bin = st[1];
    for(i = 0; i < 24; i++) {
      k = keccak_piln[i];
      tmp = st[k];
      st[k] = rotl64(bin, keccak_rotc[i]);
      bin = tmp;
    }

    for(k = 0; k < 25; k += 5) {
      for(i = 0; i < 5; i++)
        bc[i] = st[i + k];
      for(i = 0; i < 5; i++)
        st[i + k] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
    }

    st[0] ^= keccak_rndc[n];
  }
}

static void keccak_abs(keccak_t *self, const uint8_t *block, size_t size) {
  size_t i;
  uint32_t k = self->pos;

  for(i = 0; i < size; i++) {
    self->state_b[k++] ^= block[i];
    if(k == 136) {
      keccak_sum(self);
      k = 0;
    }
  }
  self->pos = k;
}

static void keccak_xof(keccak_t *self) {
  self->state_b[self->pos] ^= 0x1f;
  self->state_b[135] ^= 0x80;
  keccak_sum(self);
  self->pos = 0;
}

static void keccak_out(keccak_t *self, uint8_t *block, size_t size) {
  size_t i;
  uint32_t k = self->pos;

  for(i = 0; i < size; i++) {
    if(k == 136) {
      keccak_sum(self);
      k = 0;
    }
    block[i] = self->state_b[k++];
  }
  self->pos = k;
}

void sha3_file(uint8_t *hash, int dir_fd, const char *name) {
  size_t length;
  uint8_t buffer[4096];
  keccak_t sponge = {0};
  int fd = openat(dir_fd, name, O_RDONLY);

  if(fd < 0) {
    memset(hash, 0, 16);
    return;
  }

  while((length = read(fd, buffer, 4096)))
    keccak_abs(&sponge, buffer, length);
  keccak_xof(&sponge);
  keccak_out(&sponge, hash, 16);

  close(fd);
}
