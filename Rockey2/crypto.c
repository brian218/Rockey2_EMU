/*
 * This file is part of the Rockey2 EMU project authored by Brian218.
 *
 * Copyright (C) 2026 Brian218 (https://github.com/brian218)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <string.h>
#include "crypto.h"

static const uint32_t G[64] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12,
    5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2,
    0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9
};

static const uint32_t S[64] = {
    7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
    5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
    4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
    6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21
};

static const uint32_t T[64] = {
    0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE,
    0xF57C0FAF, 0x4787C62A, 0xA8304613, 0xFD469501,
    0x698098D8, 0x8B44F7AF, 0xFFFF5BB1, 0x895CD7BE,
    0x6B901122, 0xFD987193, 0xA679438E, 0x49B40821,
    0xF61E2562, 0xC040B340, 0x265E5A51, 0xE9B6C7AA,
    0xD62F105D, 0x02441453, 0xD8A1E681, 0xE7D3FBC8,
    0x21E1CDE6, 0xC33707D6, 0xF4D50D87, 0x455A14ED,
    0xA9E3E905, 0xFCEFA3F8, 0x676F02D9, 0x8D2A4C8A,
    0xFFFA3942, 0x8771F681, 0x6D9D6122, 0xFDE5380C,
    0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60, 0xBEBFBC70,
    0x289B7EC6, 0xEAA127FA, 0xD4EF3085, 0x04881D05,
    0xD9D4D039, 0xE6DB99E5, 0x1FA27CF8, 0xC4AC5665,
    0xF4292244, 0x432AFF97, 0xAB9423A7, 0xFC93A039,
    0x655B59C3, 0x8F0CCC92, 0xFFEFF47D, 0x85845DD1,
    0x6FA87E4F, 0xFE2CE6E0, 0xA3014314, 0x4E0811A1,
    0xF7537E82, 0xBD3AF235, 0x2AD7D2BB, 0xEB86D391
};

static void MD5_Transform(uint32_t* state, const void* buffer)
{
    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];
    uint32_t f, tmp;

    for (int i = 0; i < 64; ++i)
    {
        if (i < 16)
            f = (b & c) | (~b & d);
        else if (i < 32)
            f = (d & b) | (~d & c);
        else if (i < 48)
            f = b ^ c ^ d;
        else
            f = c ^ (b | ~d);

        tmp = d;
        d = c;
        c = b;
        b = b + _rotl((a + f + T[i] + ((const uint32_t*)buffer)[G[i]]), S[i]);
        a = tmp;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

static void MD5_Final_HMAC(uint32_t* state, uint8_t* buffer, uint64_t len)
{
    const uint64_t bitcount = (len + 64) * 8;
    buffer[len] = 0x80;
    memcpy(buffer + 56, &bitcount, sizeof bitcount);
    MD5_Transform(state, buffer);
}

uint32_t GenUID(const char* seed)
{
    uint32_t state_ipad[16] = { 0xF3FA75B2, 0x4674956E, 0xDF65E04E, 0x6C779AAF };
    uint32_t state_opad[4] = { 0x942AAA39, 0xBB8F3AE9, 0xF10FD6A7, 0x177FEE2C };

    uint8_t buffer[64] = { 0 };
    strncpy(buffer, seed, sizeof buffer);

    for (uint8_t i = 0, tail = 55; i < 54; i++)
    {
        buffer[i] ^= buffer[tail++];
        if (tail == 64) tail = 55;
    }

    MD5_Final_HMAC(state_ipad, buffer, 55);
    MD5_Final_HMAC(state_opad, (uint8_t*)state_ipad, 16);

    return state_opad[0];
}
