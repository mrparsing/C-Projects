#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static const uint32_t s[] = {
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

static const uint32_t K[] = {
    // floor(2^32 × abs(sin(i + 1)))
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

#define LEFTROTATE(x, c) (((x) << (c)) | ((x) >> (32 - (c))))

uint8_t *md5_pad(const uint8_t *msg, size_t len, size_t *new_len)
{
    size_t padded_len = len + 1;
    while (padded_len % 64 != 56)
    {
        padded_len++;
    }

    padded_len += 8;

    uint8_t *padded = calloc(padded_len, 1);
    memcpy(padded, msg, len);
    padded[len] = 0x80;

    uint64_t bit_len = len * 8;
    memcpy(padded + padded_len - 8, &bit_len, 8);

    *new_len = padded_len;
    return padded;
}

void md5(const uint8_t *initial_msg, size_t len, uint8_t digest[16])
{
    size_t new_len;
    uint8_t *msg = md5_pad(initial_msg, len, &new_len);

    uint32_t A = 0x67452301;
    uint32_t B = 0xefcdab89;
    uint32_t C = 0x98badcfe;
    uint32_t D = 0x10325476;

    for (size_t offset = 0; offset < new_len; offset += 64)
    {
        uint32_t *w = (uint32_t *)(msg + offset);
        uint32_t a = A, b = B, c = C, d = D;

        for (int i = 0; i < 64; i++)
        {
            uint32_t f, g;

            if (i < 16)
            {
                f = (b & c) | (~b & d);
                g = i;
            }
            else if (i < 32)
            {
                f = (d & b) | (~d & c);
                g = (5 * i + 1) % 16;
            }
            else if (i < 48)
            {
                f = b ^ c ^ d;
                g = (3 * i + 5) % 16;
            }
            else
            {
                f = c ^ (b | ~d);
                g = (7 * i) % 16;
            }

            uint32_t temp = d;
            d = c;
            c = b;
            b = b + LEFTROTATE((a + f + K[i] + w[g]), s[i]);
            a = temp;
        }

        A += a;
        B += b;
        C += c;
        D += d;
    }

    free(msg);

    memcpy(digest + 0, &A, 4);
    memcpy(digest + 4, &B, 4);
    memcpy(digest + 8, &C, 4);
    memcpy(digest + 12, &D, 4);
}

int main()
{
    char buf[1024];

    printf("Enter a message: ");
    fgets(buf, sizeof(buf), stdin);
    buf[strcspn(buf, "\n")] = '\0';

    uint8_t out[16];
    md5((const uint8_t *)buf, strlen(buf), out);

    for (int i = 0; i < 16; i++)
        printf("%02x", out[i]);
    printf("\n");

    return 0;
}