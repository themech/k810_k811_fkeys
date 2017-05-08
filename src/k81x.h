// Copyright 2017 Jacek Wozniak <mech@themech.net>
#ifndef K81X_H_
#define K81X_H_

#include <string>

class K81x {
 public:
  static K81x* FromDevicePath(const std::string device_path, bool verbose);
  static K81x* FromAutoFind(bool verbose);
  ~K81x();

  bool SetFnKeysMode(bool enabled);

 private:
  K81x(std::string device_path, int device_file, bool verbose);

  bool WriteSequence(const unsigned char* sequence, unsigned int size);

  int device_file_;
  std::string device_path_;
  bool verbose_;
};

#endif  // K81X_H_
