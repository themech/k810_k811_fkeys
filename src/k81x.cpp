// Copyright 2017 Jacek Wozniak <mech@themech.net>
#include "./k81x.h"
#include <errno.h>
#include <fcntl.h>
#include <libudev.h>
#include <linux/hidraw.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstddef>
#include <cstring>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;
using std::string;

#define LOGITECH_VENDOR (__u32)0x046d
#define PRODUCT_K810 (__s16)0xb319
#define PRODUCT_K811 (__s16)0xb317

const unsigned char fn_keys_on[] = {0x10, 0xff, 0x06, 0x15, 0x00, 0x00, 0x00};
const unsigned char fn_keys_off[] = {0x10, 0xff, 0x06, 0x15, 0x01, 0x00, 0x00};

K81x::K81x(string device_path, int device_file, bool verbose) {
  device_path_ = device_path;
  device_file_ = device_file;
  verbose_ = verbose;
}

K81x::~K81x() {
  if (device_file_ >= 0) {
    close(device_file_);
  }
}

K81x* K81x::FromDevicePath(const string device_path, bool verbose) {
  int device_file_ = open(device_path.c_str(), O_RDWR | O_NONBLOCK);
  if (device_file_ < 0) {
    if (verbose) cerr << "Unable to open device " << device_path << endl;
    return NULL;
  }
  struct hidraw_devinfo info;
  memset(&info, 0x0, sizeof(info));
  K81x* result = NULL;
  if (ioctl(device_file_, HIDIOCGRAWINFO, &info) >= 0) {
    if (verbose)
      cout << "Checking whether " << device_path
           << " is a Logitech K810/K811 keyboard." << endl;
    if (info.bustype != BUS_BLUETOOTH || info.vendor != LOGITECH_VENDOR ||
        (info.product != PRODUCT_K810 && info.product != PRODUCT_K811)) {
      if (verbose)
        cerr << "Cannot identify " << device_path
             << " as a supported Logitech Keyboard" << endl;

    } else {
      result = new K81x(device_path, device_file_, verbose);
    }
  } else {
    if (verbose)
      cerr << "Cannot fetch parameter of a device: " << device_path << endl;
  }
  if (result == NULL) {
    close(device_file_);
  }
  return result;
}

K81x* K81x::FromAutoFind(bool verbose) {
  struct udev* udev;
  udev = udev_new();
  if (!udev) {
    if (verbose) cerr << "Cannot create udev." << endl;
    return NULL;
  }
  if (verbose) cout << "Looking for hidraw devices" << endl;
  udev_enumerate* enumerate = udev_enumerate_new(udev);
  udev_enumerate_add_match_subsystem(enumerate, "hidraw");
  udev_enumerate_scan_devices(enumerate);
  udev_list_entry* devices = udev_enumerate_get_list_entry(enumerate);
  udev_list_entry* dev_list_entry;
  K81x* result = NULL;

  udev_list_entry_foreach(dev_list_entry, devices) {
    const char* sysfs_path = udev_list_entry_get_name(dev_list_entry);
    if (verbose) cout << "Found hidraw device: " << sysfs_path << endl;
    udev_device* raw_dev = udev_device_new_from_syspath(udev, sysfs_path);
    string device_path = udev_device_get_devnode(raw_dev);
    if (verbose) cout << "Device path: " << device_path << endl;
    result = K81x::FromDevicePath(device_path, verbose);
    if (NULL != result) break;
  }
  udev_enumerate_unref(enumerate);
  udev_unref(udev);
  if (NULL == result) {
    if (verbose) cerr << "Couldn't find a Logitech K810/K811 keyboard." << endl;
  }

  return result;
}

bool K81x::SetFnKeysMode(bool enabled) {
  if (enabled) {
    return WriteSequence(fn_keys_on, sizeof(fn_keys_on));
  }
  return WriteSequence(fn_keys_off, sizeof(fn_keys_off));
}

bool K81x::WriteSequence(const unsigned char* sequence, unsigned int size) {
  if (write(device_file_, sequence, size) < 0) {
    if (verbose_)
      cerr << "Error while writing to the device: " << string(strerror(errno))
           << endl;
    return false;
  } else {
    if (verbose) cout << "Successfully set the mode of keyboard F-keys!" << endl;
  }
  return true;
}
