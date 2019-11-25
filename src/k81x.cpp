// Copyright 2017 Jacek Wozniak <mech@themech.net>
#include "./k81x.h"
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <libudev.h>
#include <linux/hidraw.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <unistd.h>

using std::cout;
using std::cerr;
using std::endl;
using std::string;

constexpr __u32 LOGITECH_VENDOR = 0x046d;
constexpr __s16 PRODUCT_K810 = 0xb319;
constexpr __s16 PRODUCT_K811 = 0xb317;
constexpr __s16 PRODUCT_K380 = 0xb342;
constexpr __s16 PRODUCT_K780 = 0xc52b;

const unsigned char k81x_fn_keys_on[]  = {0x10, 0xff, 0x06, 0x15, 0x00, 0x00, 0x00};
const unsigned char k81x_fn_keys_off[] = {0x10, 0xff, 0x06, 0x15, 0x01, 0x00, 0x00};
const unsigned char k380_fn_keys_on[]  = {0x10, 0xff, 0x0b, 0x1e, 0x00, 0x00, 0x00};
const unsigned char k380_fn_keys_off[] = {0x10, 0xff, 0x0b, 0x1e, 0x01, 0x00, 0x00};
const unsigned char k780_fn_keys_on[]  = {0x10, 0x02, 0x0c, 0x1c, 0x00, 0x00, 0x00};
const unsigned char k780_fn_keys_off[] = {0x10, 0x02, 0x0c, 0x1c, 0x01, 0x00, 0x00};


struct DeviceInfo {
  const string name;
  const __s16 product;
  const unsigned char* fn_keys_on;
  const unsigned int fn_keys_on_size;
  const unsigned char* fn_keys_off;
  const unsigned int fn_keys_off_size;
};


const DeviceInfo KnownDevices[] = {
  { "K810", PRODUCT_K810, k81x_fn_keys_on, sizeof(k81x_fn_keys_on), k81x_fn_keys_off, sizeof(k81x_fn_keys_off) },
  { "K811", PRODUCT_K811, k81x_fn_keys_on, sizeof(k81x_fn_keys_on), k81x_fn_keys_off, sizeof(k81x_fn_keys_off) },
  { "K380", PRODUCT_K380, k380_fn_keys_on, sizeof(k380_fn_keys_on), k380_fn_keys_off, sizeof(k380_fn_keys_off) },
  { "K780", PRODUCT_K780, k780_fn_keys_on, sizeof(k780_fn_keys_on), k780_fn_keys_off, sizeof(k780_fn_keys_off) },
};


K81x::K81x(const string& device_path, int device_file, const DeviceInfo& device_info, bool verbose) :
 device_path_(device_path),
 device_file_(device_file),
 device_info_(device_info),
 verbose_(verbose) {
}

K81x::~K81x() {
  if (device_file_ >= 0) {
    close(device_file_);
  }
}

K81x* K81x::FromDevicePath(const string& device_path, bool verbose) {
  int device_file_ = open(device_path.c_str(), O_RDWR | O_NONBLOCK);
  if (device_file_ < 0) {
    if (verbose) { cerr << "Unable to open device " << device_path << endl; }
    return nullptr;
  }
  struct hidraw_devinfo info;
  memset(&info, 0x0, sizeof(info));
  K81x* result = nullptr;
  if (ioctl(device_file_, HIDIOCGRAWINFO, &info) >= 0) {
    if (verbose) {
      cout << "Checking whether " << device_path
           << " is a supported Logitech keyboard." << endl;
      cout << "Bus type: 0x" << std::hex << info.bustype << "\n";
      cout << "Vendor:   0x" << std::hex << info.vendor << "\n";
      cout << "Product:  0x" << std::hex << info.product  << "\n";
    }
    if (info.bustype == BUS_BLUETOOTH && info.vendor == LOGITECH_VENDOR) {
      for (const DeviceInfo &device_info : KnownDevices) {
        if (info.product == device_info.product) {
          if (verbose) {
            cout << "Device " << device_path << " recognized as the Logitech "
              << device_info.name << " keyboard." << endl;
          }
          result = new K81x(device_path, device_file_, device_info, verbose);
          break;
        }
      }
    }
    if (result == nullptr && verbose) {
      cerr << "Cannot identify " << device_path
           << " as a supported Logitech Keyboard" << endl;
    }
  } else {
    if (verbose) {
      cerr << "Cannot fetch parameter of a device: " << device_path << endl;
    }
  }
  if (result == nullptr) {
    close(device_file_);
  }
  return result;
}

K81x* K81x::FromDeviceSysPath(const string& device_syspath, bool verbose) {
  struct udev* udev;
  udev = udev_new();
  if (!udev) {
    if (verbose) { cerr << "Cannot create udev." << endl; }
    return nullptr;
  }
  udev_device* raw_dev = udev_device_new_from_syspath(udev, device_syspath.c_str());
  udev_unref(udev);
  if (!raw_dev) {
    if (verbose) { cerr << "Unknown udev device " <<  device_syspath << endl; }
    return nullptr;
  }
  string device_path = udev_device_get_devnode(raw_dev);
  if (verbose) { cout << "Device path: " << device_path << endl; }

  return K81x::FromDevicePath(device_path, verbose);
}

K81x* K81x::FromAutoFind(bool verbose) {
  struct udev* udev;
  udev = udev_new();
  if (!udev) {
    if (verbose) { cerr << "Cannot create udev." << endl; }
    return nullptr;
  }
  if (verbose) { cout << "Looking for hidraw devices" << endl; }
  udev_enumerate* enumerate = udev_enumerate_new(udev);
  udev_enumerate_add_match_subsystem(enumerate, "hidraw");
  udev_enumerate_scan_devices(enumerate);
  udev_list_entry* devices = udev_enumerate_get_list_entry(enumerate);
  udev_list_entry* dev_list_entry;
  K81x* result = nullptr;

  udev_list_entry_foreach(dev_list_entry, devices) {
    const char* sysfs_path = udev_list_entry_get_name(dev_list_entry);
    if (verbose) { cout << "Found hidraw device: " << sysfs_path << endl; }
    udev_device* raw_dev = udev_device_new_from_syspath(udev, sysfs_path);
    string device_path = udev_device_get_devnode(raw_dev);
    if (verbose) { cout << "Device path: " << device_path << endl; }
    result = K81x::FromDevicePath(device_path, verbose);
    if (nullptr != result) break;
  }
  udev_enumerate_unref(enumerate);
  udev_unref(udev);
  if (nullptr == result) {
    if (verbose) { cerr << "Couldn't find supported Logitech keyboard." << endl; }
  }

  return result;
}

bool K81x::SetFnKeysMode(bool enabled) {
  if (enabled) {
    return WriteSequence(device_info_.fn_keys_on, device_info_.fn_keys_on_size);
  }
  return WriteSequence(device_info_.fn_keys_off, device_info_.fn_keys_off_size);
}

bool K81x::WriteSequence(const unsigned char* sequence, unsigned int size) {
  if (write(device_file_, sequence, size) < 0) {
    if (verbose_) {
      cerr << "Error while writing to the device: " << string(strerror(errno))
           << endl;
    }
    return false;
  }
  if (verbose_) { cout << "Successfully set the mode of keyboard F-keys!" << endl; }
  return true;
}
