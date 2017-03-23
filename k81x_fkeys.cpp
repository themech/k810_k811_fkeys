#include <errno.h>
#include <fcntl.h>
#include <linux/hidraw.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

using namespace std;

#define PROGRAM_NAME "k81x_fkeys"
#define LOGITECH_VENDOR (__u32)0x046d
#define PRODUCT_K810 (__s16)0xb319
#define PRODUCT_K811 (__s16)0xb317

const unsigned char fn_keys_on[] = {0x10, 0xff, 0x06, 0x15, 0x00, 0x00, 0x00};
const unsigned char fn_keys_off[] = {0x10, 0xff, 0x06, 0x15, 0x01, 0x00, 0x00};

class FnKeysSwitcher {
 public:
  FnKeysSwitcher() {
    device_path_ = "";
    device_file_ = 0;
    fn_mode_on_ = false;
  }

  ~FnKeysSwitcher() {
    if (device_file_ > 0) {
      close(device_file_);
    }
  }

  bool SetFnMode() {
    device_file_ = open(device_path_.c_str(), O_RDWR | O_NONBLOCK);
    if (device_file_ < 0) {
      cerr << PROGRAM_NAME << ": unable to open device '" << device_path_ << "'"
           << endl
           << endl;
      return false;
    }
    struct hidraw_devinfo info;
    memset(&info, 0x0, sizeof(info));
    if (ioctl(device_file_, HIDIOCGRAWINFO, &info) < 0) {
      cerr << PROGRAM_NAME << ": cannot fetch info from device '"
           << device_path_ << "'" << endl
           << endl;
      return false;
    }
    if (info.bustype != BUS_BLUETOOTH || info.vendor != LOGITECH_VENDOR ||
        (info.product != PRODUCT_K810 && info.product != PRODUCT_K811)) {
      cerr << PROGRAM_NAME << ": cannot identify '" << device_path_
           << "' as a supported Logitech keyboard" << endl
           << endl;
      return false;
    }
    if (!fn_mode_on_) {
      return WriteSequence(fn_keys_off, sizeof(fn_keys_off));
    }
    return WriteSequence(fn_keys_on, sizeof(fn_keys_on));
  }

  bool ParseArguments(int argc, char **argv) {
    string *param = NULL;
    string fn_mode;
    for (int i = 1; i < argc; i++) {
      if (param != NULL) {
        *param = argv[i];
        param = NULL;
      } else {
        if (!std::strcmp("-fn", argv[i])) {
          param = &fn_mode;
        } else if (!strcmp("-d", argv[i])) {
          param = &device_path_;
        } else {
          cerr << PROGRAM_NAME << ": unknown parameter '" << argv[i] << "'"
               << endl
               << endl;
          return false;
        }
      }
    }
    if (device_path_.empty()) {
      cerr << PROGRAM_NAME << ": '-d' parameter not specified" << endl << endl;
      return false;
    }

    if (fn_mode == "on") {
      fn_mode_on_ = true;
    } else if (fn_mode != "off") {
      cerr << PROGRAM_NAME << ": '-fn' parameter must be either 'on' or 'off'"
           << endl
           << endl;
      return false;
    }
    return true;
  }

  void PrintHelp() {
    cout << "Logitech k810/k811 Keyboard Fn-keys switcher" << endl << endl;
    cout << "Usage: " << PROGRAM_NAME << " -d <device_path> -fn {on|off}"
         << endl
         << endl;
    cout << "Parameters:" << endl;
    cout << "  -fn\tSet to \"on\" to use F1..F12 keys as standard function keys"
         << endl;
    cout << "  -d \tPath to one of the /dev/hidraw* devices, usually "
            "/dev/hidraw0"
         << endl;
  }

 private:
  bool WriteSequence(const unsigned char *sequence, unsigned int size) {
    if (write(device_file_, sequence, size) < 0) {
      cerr << PROGRAM_NAME << ": error while writing to the device. "
           << strerror(errno) << endl;
      cerr << "Did you run this tool as root?" << endl;
    }
    return true;
  }

  string device_path_;
  int device_file_;
  bool fn_mode_on_;
};

int main(int argc, char **argv) {
  FnKeysSwitcher switcher;
  if (!switcher.ParseArguments(argc, argv)) {
    switcher.PrintHelp();
    return 1;
  }
  if (!switcher.SetFnMode()) {
    return 1;
  }
  return 0;
}
