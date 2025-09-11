/*
 * This file is part of the Rockey2 EMU project authored by Brian218.
 * 
 * Copyright (C) 2025 Brian218 (https://github.com/brian218)
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

#pragma once

#define RY2ERR_SUCCESS              0
#define RY2ERR_NO_SUCH_DEVICE       0xA0100001
#define RY2ERR_NOT_OPENED_DEVICE    0xA0100002
#define RY2ERR_WRONG_UID            0xA0100003
#define RY2ERR_WRONG_INDEX          0xA0100004
#define RY2ERR_TOO_LONG_SEED        0xA0100005
#define RY2ERR_WRITE_PROTECT        0xA0100006
#define RY2ERR_OPEN_DEVICE          0xA0100007

typedef struct
{
    DWORD hid;
    DWORD uid;
    DWORD version;
    DWORD isProtected;
    HKEY regKey;
    HANDLE mutex;
} RY2_Dongle;

int WINAPI RY2_Find();
int WINAPI RY2_Open(int mode, DWORD uid, DWORD* hid);
void WINAPI RY2_Close(int handle);
int WINAPI RY2_GenUID(int handle, DWORD* uid, char* seed, int isProtect);
int WINAPI RY2_Read(int handle, int block_index, char* buffer512);
int WINAPI RY2_Write(int handle, int block_index, char* buffer512);
int WINAPI RY2_GetVersion(int handle);
