//
// Created by Marat on 13.08.25.
//

#include "DeviceWatcher.h"
#include <map>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <IOKit/usb/IOUSBLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/graphics/IOGraphicsLib.h>

std::string currentTime() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_time), "\n%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string DeviceWatcher::usbDeviceName(io_object_t device) {
    std::string result;
    const auto productName = static_cast<CFStringRef>(IORegistryEntryCreateCFProperty(
        device,
        CFSTR(kUSBProductString),
        kCFAllocatorDefault,
        0
    ));
    if (productName) {
        char name[256];
        if (CFStringGetCString(productName, name, sizeof(name), kCFStringEncodingUTF8)) {
            result = name;
        }
        CFRelease(productName);
    }
    return result;
}

void DeviceWatcher::usbDeviceAdded(void *, io_iterator_t iterator) {
    io_object_t usbDevice;
    while ((usbDevice = IOIteratorNext(iterator))) {
        std::string name = usbDeviceName(usbDevice);
        if (name.empty()) name = "Unknown USB Device";
        std::cout << colorText(BWhite, currentTime() + " [+] USB device: " + name + " connected\n");
        std::cout << colorText(BWhite, std::string(80, '-'));
        IOObjectRelease(usbDevice);
    }
}

void DeviceWatcher::usbDeviceRemoved(void *, io_iterator_t iterator) {
    io_object_t usbDevice;
    while ((usbDevice = IOIteratorNext(iterator))) {
        std::string name = usbDeviceName(usbDevice);
        if (name.empty()) name = "Unknown USB Device";
        std::cout << colorText(BWhite, currentTime() + " [-] USB device: " + name + " disconnected\n");
        std::cout << colorText(BWhite, std::string(80, '-'));
        IOObjectRelease(usbDevice);
    }
}

std::string DeviceWatcher::audioDeviceName(AudioObjectID device) {
    std::string result;
    CFStringRef nameRef = nullptr;
    UInt32 size = sizeof(nameRef);

    constexpr AudioObjectPropertyAddress addr = {
        kAudioObjectPropertyName,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };

    if (AudioObjectGetPropertyData(device, &addr, 0, nullptr, &size, &nameRef) == noErr) {
        char name[256];
        if (CFStringGetCString(nameRef, name, sizeof(name), kCFStringEncodingUTF8)) {
            result = name;
        }
        CFRelease(nameRef);
    }

    return !result.empty() ? result : "Unknown Audio Device.";
}

std::map<AudioObjectID, std::string> currentAudioDevices;
OSStatus DeviceWatcher::audioDevicesChanged(AudioObjectID, UInt32, const AudioObjectPropertyAddress*, void*) {
    UInt32 size;
    constexpr AudioObjectPropertyAddress addr = {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };
    if (AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &addr, 0, nullptr, &size) != noErr) return noErr;

    const UInt32 count = size / sizeof(AudioObjectID);
    std::vector<AudioObjectID> devices(count);
    AudioObjectGetPropertyData(kAudioObjectSystemObject, &addr, 0, nullptr, &size, devices.data());

    std::set<AudioObjectID> newSet(devices.begin(), devices.end());

    std::set<std::string> printedNames;
    for (const auto& dev : newSet) {
        if (!currentAudioDevices.contains(dev)) {
            std::string name = audioDeviceName(dev);
            if (name != "Unknown Audio Device") {
                if (!printedNames.contains(name)) {
                    currentAudioDevices[dev] = name;
                    std::cout << colorText(BWhite, currentTime() + " [+] Audio device: " + name + " connected\n");
                    std::cout << colorText(BWhite, std::string(80, '-'));
                    printedNames.insert(name);
                }
            }
        }
    }

    for (auto it = currentAudioDevices.begin(); it != currentAudioDevices.end(); ) {
        if (!newSet.contains(it->first)) {
            std::cout << colorText(BWhite, currentTime() + " [-] Audio device: " + it->second + " disconnected\n");
            std::cout << colorText(BWhite, std::string(80, '-'));
            it = currentAudioDevices.erase(it);
        } else {
            ++it;
        }
    }

    return noErr;
}

std::map<CGDirectDisplayID, std::string> currentDisplays;
std::string DeviceWatcher::displayDeviceName(CGDirectDisplayID displayID) {
    CGDisplayModeRef mode = CGDisplayCopyDisplayMode(displayID);
    if (!mode) return "Unknown Display";

    const size_t width  = CGDisplayModeGetWidth(mode);
    const size_t height = CGDisplayModeGetHeight(mode);
    const double refreshRate = CGDisplayModeGetRefreshRate(mode);

    CFRelease(mode);

    std::ostringstream oss;
    oss << colorText(BWhite, std::to_string(width) + "x" + std::to_string(height));
    if (refreshRate > 0)
        oss << colorText(BWhite, " " + std::to_string(static_cast<int>(refreshRate)) + "Hz");

    return oss.str();
}

void DeviceWatcher::displayDeviceChanged(CGDirectDisplayID displayID, CGDisplayChangeSummaryFlags flags, void*) {
    if (flags & kCGDisplayAddFlag) {
        std::string name = displayDeviceName(displayID);
        currentDisplays[displayID] = name;
        std::cout << colorText(BWhite, currentTime() + " [+] Display connected: " + name + "\n");
        std::cout << colorText(BWhite, std::string(80, '-'));
    }

    if (flags & kCGDisplayRemoveFlag) {
        auto it = currentDisplays.find(displayID);
        if (it != currentDisplays.end()) {
            std::cout << colorText(BWhite, currentTime() + " [-] Display disconnected: " + it->second + "\n");
            std::cout << colorText(BWhite, std::string(80, '-'));
            currentDisplays.erase(it);
        }
    }
}

void DeviceWatcher::execute(const std::vector<std::string> &args) {
    (void) args;

    while (true) {
        std::cout << colorText(BWhite, "\nChoose one [1, 2, 3]: ");
        int inputChoose;
        std::cin >> inputChoose;

        switch (inputChoose) {
            case 1: usbRun(); return;
            case 2: audioRun(); return;
            case 3: displayRun(); return;
            default: std::cout << colorText(BRed, "\nWrong input!\n"); break;
        }
    }
}

void DeviceWatcher::usbRun() const {
    io_iterator_t addedIter;
    io_iterator_t removedIter;

    const IONotificationPortRef notifyPort = IONotificationPortCreate(kIOMainPortDefault);
    const CFRunLoopSourceRef runLoopSource = IONotificationPortGetRunLoopSource(notifyPort);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);

    IOServiceAddMatchingNotification(
        notifyPort,
        kIOFirstMatchNotification,
        IOServiceMatching(kIOUSBDeviceClassName),
        usbDeviceAdded,
        nullptr,
        &addedIter
    );
    usbDeviceAdded(nullptr, addedIter);

    IOServiceAddMatchingNotification(
        notifyPort,
        kIOTerminatedNotification,
        IOServiceMatching(kIOUSBDeviceClassName),
        usbDeviceRemoved,
        nullptr,
        &removedIter
    );
    usbDeviceRemoved(nullptr, removedIter);

    std::cout << colorText(BWhite, "\n[*] Waiting for USB device events...\n");
    CFRunLoopRun();
}

void DeviceWatcher::audioRun() const {
    constexpr AudioObjectPropertyAddress addr = {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };
    AudioObjectAddPropertyListener(
        kAudioObjectSystemObject,
        &addr,
        audioDevicesChanged,
        nullptr
    );

    std::cout << colorText(BWhite, "\n[*] Waiting for Audio device events...\n");
    CFRunLoopRun();
}

void DeviceWatcher::displayRun() const {
    CGDirectDisplayID displays[32];
    uint32_t displayCount = 0;
    CGGetOnlineDisplayList(32, displays, &displayCount);

    for (uint32_t i = 0; i < displayCount; ++i) {
        CGDirectDisplayID id = displays[i];
        if (!currentDisplays.contains(id)) {
            std::string name = displayDeviceName(id);
            currentDisplays[id] = name;
            std::cout << colorText(BWhite, currentTime() + " [+] Display connected: " + name + "\n");
            std::cout << colorText(BWhite, std::string(80, '-'));
        }
    }

    std::cout << colorText(BWhite, "\n[*] Waiting for Display events...\n");
    CGDisplayRegisterReconfigurationCallback(displayDeviceChanged, nullptr);

    CFRunLoopRun();
}
