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

#include <stdio.h>
#include <windows.h>
#include "Rockey2.h"

static const HKEY RegRootKey = HKEY_CURRENT_USER;
static const char* RegSubKey = "Software\\Rockey2\\Dongles";
static const char* RegBlockName = "Block%d";
static const char* const RegInfoNames[] = { "HID", "UID", "Version", "Protection" };
static RY2_Dongle* Dongles = NULL;
static int DongleCount = 0;
static HANDLE ProcessHeap = NULL;

static void ReadRegDongleCountValue(void)
{
    const char* RegValueName = "Count";
    HKEY regKey = NULL;
    if ((RegCreateKeyEx(RegRootKey, RegSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WOW64_64KEY | KEY_READ | KEY_WRITE, NULL, &regKey, NULL) == ERROR_SUCCESS ||
        RegOpenKeyEx(RegRootKey, RegSubKey, 0, KEY_WOW64_64KEY | KEY_READ, &regKey) == ERROR_SUCCESS) && regKey)
    {
        DWORD regType = REG_DWORD;
        DWORD regSize = sizeof(DWORD);
        LSTATUS regStatus = RegQueryValueEx(regKey, RegValueName, NULL, &regType, (LPBYTE)&DongleCount, &regSize);
        if (!(regStatus == ERROR_SUCCESS && regType == REG_DWORD && regSize == sizeof(DWORD)) || DongleCount < 0 || DongleCount > 100)
        {
            DongleCount = 0;
            regType = REG_DWORD;
            regSize = sizeof(DWORD);
            regStatus = RegSetValueEx(regKey, RegValueName, 0, regType, (const LPBYTE)&DongleCount, regSize);
        }
        RegCloseKey(regKey);
    }
    else
        DongleCount = 0;
}

static void OpenRegDongleKey(int handle)
{
    char regKeyPath[33 + 1] = { 0 }; // Software\Rockey2\Dongles\Dongle00 + '\0'
    if (!Dongles[handle].regKey)
    {
        _snprintf(regKeyPath, sizeof regKeyPath - 1, "%s\\Dongle%02d", RegSubKey, handle);
        if (RegCreateKeyEx(RegRootKey, regKeyPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WOW64_64KEY | KEY_READ | KEY_WRITE, NULL, &Dongles[handle].regKey, NULL) != ERROR_SUCCESS &&
            RegOpenKeyEx(RegRootKey, regKeyPath, 0, KEY_WOW64_64KEY | KEY_READ, &Dongles[handle].regKey) != ERROR_SUCCESS)
            Dongles[handle].regKey = NULL;
    }
}

static BOOL ReadRegBlockValue(int handle, int block_index, char* buffer512)
{
    if (!Dongles[handle].regKey)
        return FALSE;
    char regName[6 + 1] = { 0 }; // Block0 + '\0'
    _snprintf(regName, sizeof regName - 1, RegBlockName, block_index);
    DWORD regType = REG_BINARY;
    DWORD regSize = 512;
    LSTATUS regStatus = RegQueryValueEx(Dongles[handle].regKey, regName, NULL, &regType, (LPBYTE)buffer512, &regSize);
    return regStatus == ERROR_SUCCESS && regType == REG_BINARY && regSize == 512;
}

static BOOL WriteRegBlockValue(int handle, int block_index, const char* buffer512)
{
    if (!Dongles[handle].regKey)
        return FALSE;
    char regName[6 + 1] = { 0 }; // Block0 + '\0'
    _snprintf(regName, sizeof regName - 1, RegBlockName, block_index);
    DWORD regType = REG_BINARY;
    DWORD regSize = 512;
    LSTATUS regStatus = RegSetValueEx(Dongles[handle].regKey, regName, 0, regType, (const LPBYTE)buffer512, regSize);
    return regStatus == ERROR_SUCCESS /* && regType == REG_BINARY && regSize == 512 */;
}

static BOOL ReadRegInfoValue(int handle)
{
    if (!Dongles[handle].regKey)
        return FALSE;
    BOOL success = TRUE;
    DWORD* const dongleInfo[sizeof RegInfoNames / sizeof(char*)] = { &Dongles[handle].hid, &Dongles[handle].uid, &Dongles[handle].version, &Dongles[handle].isProtected };
    for (int i = 0; i < sizeof dongleInfo / sizeof(DWORD*); i++)
    {
        DWORD regType = REG_DWORD;
        DWORD regSize = sizeof(DWORD);
        LSTATUS regStatus = RegQueryValueEx(Dongles[handle].regKey, RegInfoNames[i], NULL, &regType, (LPBYTE)dongleInfo[i], &regSize);
        if (!(regStatus == ERROR_SUCCESS && regType == REG_DWORD && regSize == sizeof(DWORD)))
        {
            if (success)
                success = FALSE;
            *dongleInfo[i] = 0;
        }
    }
    return success;
}

static BOOL WriteRegInfoValue(int handle)
{
    if (!Dongles[handle].regKey)
        return FALSE;
    BOOL success = TRUE;
    const DWORD* const dongleInfo[sizeof RegInfoNames / sizeof(char*)] = { &Dongles[handle].hid, &Dongles[handle].uid, &Dongles[handle].version, &Dongles[handle].isProtected };
    for (int i = 0; i < sizeof dongleInfo / sizeof(DWORD*); i++)
    {
        DWORD regType = REG_DWORD;
        DWORD regSize = sizeof(DWORD);
        LSTATUS regStatus = RegSetValueEx(Dongles[handle].regKey, RegInfoNames[i], 0, regType, (const LPBYTE)dongleInfo[i], regSize);
        if (!(regStatus == ERROR_SUCCESS /* && regType == REG_DWORD && regSize == sizeof(DWORD) */))
        {
            if (success)
                success = FALSE;
        }
    }
    return success;
}

static void CloseDongle(int handle)
{
    if (Dongles[handle].regKey)
    {
        RegCloseKey(Dongles[handle].regKey);
        Dongles[handle].regKey = NULL;
    }
    if (Dongles[handle].mutex)
    {
        CloseHandle(Dongles[handle].mutex);
        Dongles[handle].mutex = NULL;
    }
}

static void Cleanup(void)
{
    if (Dongles)
    {
        for (int i = 0; i < DongleCount; i++)
            CloseDongle(i);
        HeapFree(ProcessHeap, 0, Dongles);
        Dongles = NULL;
    }
    DongleCount = 0;
}

int WINAPI RY2_Find()
{
    Cleanup();
    ReadRegDongleCountValue();
    if (DongleCount > 0)
    {
        Dongles = (RY2_Dongle*)HeapAlloc(ProcessHeap, HEAP_ZERO_MEMORY, DongleCount * sizeof(RY2_Dongle));
        if (!Dongles)
            DongleCount = 0;
    }
    for (int i = 0; i < DongleCount; i++)
    {
        OpenRegDongleKey(i);
        if (!ReadRegInfoValue(i))
            WriteRegInfoValue(i);
        CloseDongle(i);
    }
    return DongleCount;
}

int WINAPI RY2_Open(int mode, DWORD uid, DWORD* hid)
{
    int uidMatchCount = 0;
    for (int i = 0; i < DongleCount; i++)
    {
        if ((mode == 0) || (mode == -1 && *hid == Dongles[i].hid) || (mode > 0 && uid == Dongles[i].uid && mode == ++uidMatchCount))
        {
            OpenRegDongleKey(i);
            if (!Dongles[i].mutex)
            {
                char mutexName[15 + 1] = { 0 }; // ROCKEY2_MUTEX00 + '\0'
                _snprintf(mutexName, sizeof mutexName - 1, "ROCKEY2_MUTEX%02d", i);
                Dongles[i].mutex = CreateMutex(NULL, FALSE, mutexName);
            }
            if (Dongles[i].regKey && Dongles[i].mutex)
            {
                *hid = Dongles[i].hid;
                return i;
            }
            CloseDongle(i);
            return RY2ERR_OPEN_DEVICE;
        }
    }
    return RY2ERR_NO_SUCH_DEVICE;
}

void WINAPI RY2_Close(int handle)
{
    if (handle < 0 || handle >= DongleCount)
        return;
    CloseDongle(handle);
}

int WINAPI RY2_GenUID(int handle, DWORD* uid, char* seed, int isProtect)
{
    if (handle < 0 || handle >= DongleCount)
        return RY2ERR_NO_SUCH_DEVICE;
    if (strlen(seed) > 64)
        return RY2ERR_TOO_LONG_SEED;
    if (!Dongles[handle].regKey || !Dongles[handle].mutex)
        return RY2ERR_NOT_OPENED_DEVICE;
    WaitForSingleObject(Dongles[handle].mutex, INFINITE);
    Dongles[handle].isProtected = isProtect;
    *uid = Dongles[handle].uid;
    ReleaseMutex(Dongles[handle].mutex);
    return RY2ERR_SUCCESS;
}

int WINAPI RY2_Read(int handle, int block_index, char* buffer512)
{
    if (handle < 0 || handle >= DongleCount)
        return RY2ERR_NO_SUCH_DEVICE;
    if (block_index < 0 || block_index > 4)
        return RY2ERR_WRONG_INDEX;
    if (!Dongles[handle].regKey || !Dongles[handle].mutex)
        return RY2ERR_NOT_OPENED_DEVICE;
    WaitForSingleObject(Dongles[handle].mutex, INFINITE);
    if (!ReadRegBlockValue(handle, block_index, buffer512))
        memset(buffer512, 0, 512);
    ReleaseMutex(Dongles[handle].mutex);
    return RY2ERR_SUCCESS;
}

int WINAPI RY2_Write(int handle, int block_index, char* buffer512)
{
    if (handle < 0 || handle >= DongleCount)
        return RY2ERR_NO_SUCH_DEVICE;
    if (block_index < 0 || block_index > 4)
        return RY2ERR_WRONG_INDEX;
    if (!Dongles[handle].regKey || !Dongles[handle].mutex)
        return RY2ERR_NOT_OPENED_DEVICE;
    if (Dongles[handle].isProtected)
        return RY2ERR_WRITE_PROTECT;
    WaitForSingleObject(Dongles[handle].mutex, INFINITE);
    WriteRegBlockValue(handle, block_index, buffer512);
    ReleaseMutex(Dongles[handle].mutex);
    return RY2ERR_SUCCESS;
}

int WINAPI RY2_GetVersion(int handle)
{
    if (handle < 0 || handle >= DongleCount)
        return RY2ERR_NO_SUCH_DEVICE;
    if (!Dongles[handle].regKey || !Dongles[handle].mutex)
        return RY2ERR_NOT_OPENED_DEVICE;
    return Dongles[handle].version;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        ProcessHeap = GetProcessHeap();
        if (!ProcessHeap)
            return FALSE;
        DisableThreadLibraryCalls(hModule);
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
    {
        Cleanup();
        break;
    }
    }
    return TRUE;
}
