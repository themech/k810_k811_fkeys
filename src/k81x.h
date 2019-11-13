// Copyright 2017 Jacek Wozniak <mech@themech.net>
#ifndef K81X_H_
#define K81X_H_

#include <string>

struct DeviceInfo;

class K81x {
 public:
  static K81x* FromDevicePath(const std::string device_path, bool verbose);
  static K81x* FromDeviceSysPath(const std::string device_syspath, bool verbose);
  static K81x* FromAutoFind(bool verbose);
  ~K81x();

  bool SetFnKeysMode(bool enabled);

 private:

  K81x(std::string device_path, int device_file, const DeviceInfo& device_info, bool verbose);

  bool WriteSequence(const unsigned char* sequence, unsigned int size);

  int device_file_;
  std::string device_path_;
  const DeviceInfo& device_info_;
  bool verbose_;
};

#endif  // K81X_H_
