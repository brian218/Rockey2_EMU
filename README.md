# Rockey2 Dongle Emulator

## Important Disclaimer

**This library is intended strictly for software developers to debug applications that they have legally developed themselves.**

To prevent any form of copyright infringement, this library must only be used for legitimate development and debugging purposes for your own applications. We do not support, condone, or permit the use of this library for piracy or any other copyright-infringing activities. Using this library to circumvent copy protection on software you do not own is strictly prohibited.

This library is distributed in the hope that it will be useful, but **WITHOUT ANY WARRANTY**; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. Use it at your own risk.

## Overview

This project aims to implement high-level emulation of Rockey2 dongle hardware. It is served as a Windows dynamic-link library (a `.dll` file) and is designed to assist developers in developing or debugging THEIR OWN applications that require a Rockey2 dongle for operation.

This library works by reading configuration data from the Windows Registry to simulate the presence of one or more dongles. To ensure **stateful emulation**, any data the application writes to the dongle is also saved back to the Registry. This allows the emulated dongle to maintain its state across sessions, just like a physical device. This library supports the emulation of up to 100 individual dongles simultaneously (`Dongle00` to `Dongle99`).

## Installation Guide

Follow these steps to install and configure this library:

### Step 1: Replace the DLL

1.  Navigate to the installation directory of your application.
2.  Locate the original `Rockey2.dll` (or a similarly named DLL file responsible for dongle communication).
3.  **Backup the original DLL file** to a safe location.
4.  Replace the original DLL with the one provided by this project. Ensure the new file is renamed to match the original file name exactly.

### Step 2: Configure the Windows Registry

1.  A sample registry file, `Sample.reg`, is included. This file contains the necessary configuration for the library to function.
2.  Double-click the `Sample.reg` file to import its contents into the Windows Registry.
3.  Confirm the action when prompted by the Registry Editor.

This will create the required registry keys under the following path:
`HKEY_CURRENT_USER\Software\Rockey2\Dongles`

## Configuration and Usage

This library is designed to be self-configuring. When the application calls the `RY2_Find()` function, this library will automatically create the necessary base registry keys if they are not already present.

However, for full functionality and pre-configuration, it is recommended to import the `Sample.reg` file as described in the installation steps.

To customize the emulated dongles, you can modify the `Sample.reg` file with a text editor before importing it, or you can use the Windows Registry Editor (`regedit.exe`) to modify the values directly after import. See the section below for details.

## Registry Configuration Details

The `Sample.reg` file sets up the configuration in the Windows Registry for your emulated Rockey2 dongles. Here’s a brief guide on how to understand and modify it.

### Main Configuration

The primary location for all settings is `HKEY_CURRENT_USER\Software\Rockey2\Dongles`.

* **`"Count"`**: This value tells the library **how many dongles to emulate**.
    * The valid range is `0` to `100`.
    * The default value is `0`, which means no dongles are present.
    * If this value is found to be outside the valid range, the library will automatically reset it to `0`.

### Individual Dongle Configuration

Each emulated dongle has its own section, from `Dongle00` up to `Dongle99`.

* **`"HID"`, `"UID"`, `"Version"`**: These are the **hardware identifiers** for the dongle. You should set these values to match the specific physical dongle your application is designed to work with.

* **`"Protection"`**: This sets the **write-protection status**.
    * `dword:00000001`: The dongle is **read-only** (write-protected).
    * `dword:00000000`: The dongle is **writable**.

* **`"Block0"` to `"Block4"`**: These five keys represent the **dongle's internal memory**. Each block stores 512 bytes of data in hexadecimal format. This is the data that your application will read from or write to the dongle during operation. You need to populate these blocks with the data your application expects.

## Developer Notes

This section details some of the key design philosophies and implementation techniques used in this library. It is intended for developers who wish to understand, maintain, or contribute to the project by explaining the rationale behind the code's structure and behavior.

### Core Philosophy

* **Faithful API Emulation:** The primary goal of this library is to be a drop-in replacement that is behaviorally identical to the original. This means the public-facing functions (`RY2_*`) intentionally replicate the exact signatures, return value styles, and even the quirks of the original library. For example, the function `RY2_GenUID` does not implement a complex hardware algorithm but rather mimics the observable results that are relevant for software emulation.

* **Zero-Dependency & CRT-Free Design:** This library is meticulously crafted to rely only on native Win32 APIs provided by essential system DLLs like `kernel32.dll` and `advapi32.dll`. It deliberately avoids any dependency on the Microsoft Visual C++ Runtime (CRT). This design choice is critical for two reasons:
    1.  **Maximum Portability:** It runs on nearly any Windows system out-of-the-box without requiring the installation of a specific C++ redistributable package.
    2.  **Injection Safety:** It can be safely injected into foreign processes without the risk of causing CRT version conflicts or crashing a process that doesn't use a CRT.

### Robustness & Safety

* **Inter-Process Concurrency Safety:** The library uses **named mutexes** (e.g., `ROCKEY2_MUTEX00`) to manage access to emulated dongle data. This is a crucial design choice that prevents race conditions not only between threads within a single process but also **between multiple, separate processes** that might be accessing the same emulated dongle via the registry.

* **Guaranteed-Safe String Formatting:** This library ensures all string formatting is safe and properly terminated by adhering to the following disciplined, multi-part mechanism.
    1.  **Proactive Initialization:** As a primary layer of safety, all character arrays are **zero-initialized upon declaration** (e.g., `char buffer[N + 1] = { 0 };`). This fills the entire buffer with null characters from the start, ensuring the string is safely terminated by default, even before any data is written.
    2.  **Meticulous Sizing and Truncation:** A C string requires a **null terminator (`\0`)** to mark its end, so to store **N** characters, a buffer of size **N+1** is necessary. To actively protect this crucial `+1` space, this library always passes `sizeof(buffer) - 1` as the size parameter to `_snprintf`. This technique **reserves the last byte of the buffer** exclusively for the null terminator, which actively prevents buffer overflows.
    3.  **Deliberate Function Choice:** The use of `_snprintf` over the more modern `sprintf_s` is intentional. `sprintf_s` introduces a dependency on a specific version of the C Runtime (CRT). By using the more fundamental `_snprintf` and enforcing safety manually, this library remains **CRT-free**, guaranteeing maximum compatibility and safe injection into any target process.

* **Self-Healing Configuration:** The library is designed to be resilient against corrupted or invalid registry configurations through a three-level self-healing and fault-tolerance mechanism:
    1.  **Global Dongle Count:** The main `Count` value is validated upon initialization. If it is missing, corrupted, or outside the valid range (0-100), the library automatically resets the value to a safe default of `0`.
    2.  **Individual Dongle Properties:** For each individual dongle key (e.g., `Dongle00`), its hardware identifier values (`HID`, `UID`, `Version`, `Protection`) are also validated. If any of these values are found to be missing or of an incorrect data type, they are reset to `0`, and the corrected values are written back to the registry.
    3.  **Default State for Missing Block Data:** When a data block (`Block0` through `Block4`) is missing from the registry for a specified dongle, the function will not return an error. Instead, it will fill the application's buffer with 512 zeros, simulating an empty state, while not writing back to the registry since writing a full 512-byte block (or up to 2.5 KB for all 5 blocks) is a slow I/O operation.

### Resource Management

* **Disciplined Resource Cleanup:** All resource allocation (memory via `HeapAlloc`, system handles for registry keys and mutexes) is meticulously tracked. The `DllMain` function ensures that on `DLL_PROCESS_DETACH`, a `Cleanup` function is called to release every acquired resource, preventing any leaks in the host process.

* **Conservative Handle Management:** The `RY2_Find` function opens, reads, and immediately closes the registry key for each dongle it finds. While this may seem less performant than keeping the handles open, it is a deliberate trade-off. This "just-in-time" approach ensures the library maintains a minimal resource footprint, avoiding a scenario where dozens of unused registry handles are held open indefinitely. Handles are only acquired and held by `RY2_Open` when a dongle is actively in use.

* **Precision Stack Allocation:** For fixed-format strings, stack buffers are allocated with precisely calculated sizes (e.g., `char mutexName[15 + 1];`) rather than arbitrary large sizes (e.g., `256`). This reflects a "no byte wasted" philosophy common in disciplined systems programming, ensuring a minimal memory footprint.

### Code Elegance & Maintainability

* **Data-Driven Implementation (DRY Principle):** In functions like `ReadRegInfoValue` and `WriteRegInfoValue`, an array of pointers to the struct members is used in parallel with an array of registry value names. This allows a single, generic loop to read or write all properties, completely eliminating repetitive code. This makes the logic cleaner, less error-prone, and vastly easier to maintain—adding a new property simply requires updating the two arrays.
