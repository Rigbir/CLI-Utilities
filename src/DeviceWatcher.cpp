//
// Created by Marat on 13.08.25.
//

#include "DeviceWatcher.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>
#include <IOKit/usb/IOUSBLib.h>
#include <CoreFoundation/CoreFoundation.h>

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
    for (auto dev : newSet) {
        if (currentAudioDevices.find(dev) == currentAudioDevices.end()) {
            std::string name = audioDeviceName(dev);
            if (name != "Unknown Audio Device") {
                if (!printedNames.count(name)) {
                    currentAudioDevices[dev] = name;
                    std::cout << colorText(BWhite, currentTime() + " [+] Audio device: " + name + " connected\n");
                    std::cout << colorText(BWhite, std::string(80, '-'));
                    printedNames.insert(name);
                }
            }
        }
    }

    for (auto it = currentAudioDevices.begin(); it != currentAudioDevices.end(); ) {
        if (newSet.find(it->first) == newSet.end()) {
            std::cout << colorText(BWhite, currentTime() + " [-] Audio device: " + it->second + " disconnected\n");
            std::cout << colorText(BWhite, std::string(80, '-'));
            it = currentAudioDevices.erase(it);
        } else {
            ++it;
        }
    }

    return noErr;
}

void DeviceWatcher::execute(const std::vector<std::string> &args) {
    (void) args;

    //usbRun();
    audioRun();
    //displayRun();
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
    return;
}


