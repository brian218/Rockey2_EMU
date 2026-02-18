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

#define MD5_ROTL(w,x,g,s,t) w=x+_rotl((w+f+t+buf[g]),s);
#define MD5_R0(w,x,y,z,g,s,t) f=(x&y)|(~x&z);MD5_ROTL(w,x,g,s,t)
#define MD5_R1(w,x,y,z,g,s,t) f=(z&x)|(~z&y);MD5_ROTL(w,x,g,s,t)
#define MD5_R2(w,x,y,z,g,s,t) f=x^y^z;MD5_ROTL(w,x,g,s,t)
#define MD5_R3(w,x,y,z,g,s,t) f=y^(x|~z);MD5_ROTL(w,x,g,s,t)

static const uint32_t MD5_InitState[16] =
{
    0xF3FA75B2, 0x4674956E, 0xDF65E04E, 0x6C779AAF,
    0x942AAA39, 0xBB8F3AE9, 0xF10FD6A7, 0x177FEE2C,
    0xF22427A0, 0xCA47F9BA, 0x4D203A65, 0x0E71FFFA,
    0x9086D35D, 0x2EBA8A42, 0x60965967, 0x22B7AC16
};

static void MD5_Transform(uint32_t* state, const void* buffer)
{
    const uint32_t* buf = (const uint32_t*)buffer;
    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];
    uint32_t f;

    MD5_R0(a, b, c, d, 0, 7, 0xD76AA478);
    MD5_R0(d, a, b, c, 1, 12, 0xE8C7B756);
    MD5_R0(c, d, a, b, 2, 17, 0x242070DB);
    MD5_R0(b, c, d, a, 3, 22, 0xC1BDCEEE);
    MD5_R0(a, b, c, d, 4, 7, 0xF57C0FAF);
    MD5_R0(d, a, b, c, 5, 12, 0x4787C62A);
    MD5_R0(c, d, a, b, 6, 17, 0xA8304613);
    MD5_R0(b, c, d, a, 7, 22, 0xFD469501);
    MD5_R0(a, b, c, d, 8, 7, 0x698098D8);
    MD5_R0(d, a, b, c, 9, 12, 0x8B44F7AF);
    MD5_R0(c, d, a, b, 10, 17, 0xFFFF5BB1);
    MD5_R0(b, c, d, a, 11, 22, 0x895CD7BE);
    MD5_R0(a, b, c, d, 12, 7, 0x6B901122);
    MD5_R0(d, a, b, c, 13, 12, 0xFD987193);
    MD5_R0(c, d, a, b, 14, 17, 0xA679438E);
    MD5_R0(b, c, d, a, 15, 22, 0x49B40821);
    MD5_R1(a, b, c, d, 1, 5, 0xF61E2562);
    MD5_R1(d, a, b, c, 6, 9, 0xC040B340);
    MD5_R1(c, d, a, b, 11, 14, 0x265E5A51);
    MD5_R1(b, c, d, a, 0, 20, 0xE9B6C7AA);
    MD5_R1(a, b, c, d, 5, 5, 0xD62F105D);
    MD5_R1(d, a, b, c, 10, 9, 0x02441453);
    MD5_R1(c, d, a, b, 15, 14, 0xD8A1E681);
    MD5_R1(b, c, d, a, 4, 20, 0xE7D3FBC8);
    MD5_R1(a, b, c, d, 9, 5, 0x21E1CDE6);
    MD5_R1(d, a, b, c, 14, 9, 0xC33707D6);
    MD5_R1(c, d, a, b, 3, 14, 0xF4D50D87);
    MD5_R1(b, c, d, a, 8, 20, 0x455A14ED);
    MD5_R1(a, b, c, d, 13, 5, 0xA9E3E905);
    MD5_R1(d, a, b, c, 2, 9, 0xFCEFA3F8);
    MD5_R1(c, d, a, b, 7, 14, 0x676F02D9);
    MD5_R1(b, c, d, a, 12, 20, 0x8D2A4C8A);
    MD5_R2(a, b, c, d, 5, 4, 0xFFFA3942);
    MD5_R2(d, a, b, c, 8, 11, 0x8771F681);
    MD5_R2(c, d, a, b, 11, 16, 0x6D9D6122);
    MD5_R2(b, c, d, a, 14, 23, 0xFDE5380C);
    MD5_R2(a, b, c, d, 1, 4, 0xA4BEEA44);
    MD5_R2(d, a, b, c, 4, 11, 0x4BDECFA9);
    MD5_R2(c, d, a, b, 7, 16, 0xF6BB4B60);
    MD5_R2(b, c, d, a, 10, 23, 0xBEBFBC70);
    MD5_R2(a, b, c, d, 13, 4, 0x289B7EC6);
    MD5_R2(d, a, b, c, 0, 11, 0xEAA127FA);
    MD5_R2(c, d, a, b, 3, 16, 0xD4EF3085);
    MD5_R2(b, c, d, a, 6, 23, 0x04881D05);
    MD5_R2(a, b, c, d, 9, 4, 0xD9D4D039);
    MD5_R2(d, a, b, c, 12, 11, 0xE6DB99E5);
    MD5_R2(c, d, a, b, 15, 16, 0x1FA27CF8);
    MD5_R2(b, c, d, a, 2, 23, 0xC4AC5665);
    MD5_R3(a, b, c, d, 0, 6, 0xF4292244);
    MD5_R3(d, a, b, c, 7, 10, 0x432AFF97);
    MD5_R3(c, d, a, b, 14, 15, 0xAB9423A7);
    MD5_R3(b, c, d, a, 5, 21, 0xFC93A039);
    MD5_R3(a, b, c, d, 12, 6, 0x655B59C3);
    MD5_R3(d, a, b, c, 3, 10, 0x8F0CCC92);
    MD5_R3(c, d, a, b, 10, 15, 0xFFEFF47D);
    MD5_R3(b, c, d, a, 1, 21, 0x85845DD1);
    MD5_R3(a, b, c, d, 8, 6, 0x6FA87E4F);
    MD5_R3(d, a, b, c, 15, 10, 0xFE2CE6E0);
    MD5_R3(c, d, a, b, 6, 15, 0xA3014314);
    MD5_R3(b, c, d, a, 13, 21, 0x4E0811A1);
    MD5_R3(a, b, c, d, 4, 6, 0xF7537E82);
    MD5_R3(d, a, b, c, 11, 10, 0xBD3AF235);
    MD5_R3(c, d, a, b, 2, 15, 0x2AD7D2BB);
    MD5_R3(b, c, d, a, 9, 21, 0xEB86D391);

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
    uint32_t state_ipad[16] = { 0 };
    uint32_t state_opad[4] = { 0 };
    memcpy(state_ipad, MD5_InitState, 16);
    memcpy(state_opad, MD5_InitState + 4, 16);

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

int Transform(uint32_t uid, uint8_t* data, int len)
{
    if (len == 0 || len > 32)
        return 0xB7;

    uint64_t xor_key[4] = { 0 };
    uint64_t data_temp[4] = { 0 };
    memcpy(data_temp, data, len);

    for (int i = 0; i < 2; i++)
    {
        uint32_t state_ipad[16] = { 0 };
        uint32_t state_opad[4] = { 0 };
        memcpy(state_ipad, MD5_InitState, 16);
        memcpy(state_opad, MD5_InitState + 4, 16);
        for (int j = 0; j < 4; j++)
        {
            state_ipad[j] ^= uid;
            state_opad[j] ^= uid;
        }
        uint8_t buffer[64] = { 0 };
        memcpy(buffer + (((uint8_t*)data_temp)[i * 16] % 23), data_temp, len);
        MD5_Final_HMAC(state_ipad, buffer, 55);
        MD5_Final_HMAC(state_opad, (uint8_t*)state_ipad, 16);
        memcpy(xor_key + i * 2, state_opad, 16);
    }

    for (int i = 0; i < 4; i++)
        data_temp[i] ^= xor_key[i];

    memcpy(data, data_temp, len);
    return 0;
}

void Transform_Factory(const uint8_t* challenge, uint8_t* response)
{
    uint32_t state_ipad[16] = { 0 };
    uint32_t state_opad[4] = { 0 };
    memcpy(state_ipad, MD5_InitState + 8, 16);
    memcpy(state_opad, MD5_InitState + 12, 16);

    uint8_t buffer[64] = { 0 };
    memcpy(buffer, challenge, 55);
    MD5_Final_HMAC(state_ipad, buffer, 55);
    MD5_Final_HMAC(state_opad, (uint8_t*)state_ipad, 16);
    memcpy(response, state_opad, 16);
}
