//
// Created by Marat on 13.08.25.
//

#pragma once
#include "IDeviceWatcher.h"
#include <set>
#include <IOKit/IOKitLib.h>
#include <CoreAudio/CoreAudio.h>
#include <CoreGraphics/CoreGraphics.h>


class DeviceWatcher final : public IDeviceWatcher {
public:
    void execute(const std::vector<std::string>& args) override;
    void usbRun() const override;
    void audioRun() const override;
    void displayRun() const override;

private:
    static std::string usbDeviceName(io_object_t device);
    static void usbDeviceAdded(void* refCon, io_iterator_t iterator);
    static void usbDeviceRemoved(void* refCon, io_iterator_t iterator);

    static std::string audioDeviceName(AudioObjectID device);
    static OSStatus audioDevicesChanged(AudioObjectID, UInt32, const AudioObjectPropertyAddress*, void*);

    static std::string displayDeviceName(CGDirectDisplayID displayID);
    static void displayDeviceChanged(CGDirectDisplayID displayID, CGDisplayChangeSummaryFlags flags, void*);
};
