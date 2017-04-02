// Copyright 2017 Jacek Wozniak <mech@themech.net>
#include "./k81x.h"
#include <unistd.h>
#include <cstring>
#include <iostream>

using std::cerr;
using std::cout;
using std::endl;

void usage() {
  cout << "Usage: sudo k81x_fkeys [-d device_path] [-v] on|off" << endl;
  cout << "Controls the functions of Logitech K810/K811 Keyboard F-keys" << endl
       << endl;

  cout << "As seen above, this tool needs root privileges to operate. Options:"
       << endl;
  cout << "\t-d device_path\tDevice path of the Logitech keyboard, usually"
       << endl
       << "\t\t\t/dev/hidraw0. Autodetecion is peformed if this" << endl
       << "\t\t\tparameter is omitted." << endl;
  cout << "\t-v\t\tVerbose mode." << endl;
  cout << "\ton|off\t\t\"on\" causes the F-keys to act like standard" << endl
       << "\t\t\tF1-F12 keys, \"off\" enables the enhanced functions." << endl;
}

int main(int argc, char **argv) {
  bool verbose = false, switch_on;
  const char *device_path = NULL;

  // Fetch the command line arguments.
  int opt;
  while ((opt = getopt(argc, argv, "d:v")) != -1) {
    switch (opt) {
      case 'd':
        device_path = optarg;
        break;
      case 'v':
        verbose = true;
        break;
    }
  }
  if (optind >= argc) {
    // No on/off argument.
    usage();
    return 1;
  }
  if (!strcmp("on", argv[optind])) {
    switch_on = true;
  } else if (!strcmp("off", argv[optind])) {
    switch_on = false;
  } else {
    cerr << "Invalid switch value, should be either \"on\" or \"off\"." << endl;
    usage();
    return 1;
  }

  // Check the privileges.
  if (geteuid() != 0) {
    cerr << "Warning: Program not running as root. It will most likely fail."
         << endl;
  }

  // Initialize the device.
  K81x *k81x = NULL;
  if (device_path == NULL) {
    k81x = K81x::FromAutoFind(verbose);
    if (NULL == k81x) {
      cerr << "Error while looking for a Logitech K810/K811 keyboard." << endl;
    }
  } else {
    k81x = K81x::FromDevicePath(device_path, verbose);
    if (NULL == k81x) {
      cerr
          << "Device " << device_path
          << " cannot be recognized as a supported Logitech K810/K811 keyboard."
          << endl;
    }
  }

  int result = 0;
  if (k81x != NULL) {
    // Switch the Kn keys mode.
    if (!k81x->SetFnKeysMode(switch_on)) {
      cerr << "Error while setting the F-keys mode." << endl;
      result = 1;
    }

    delete k81x;
  } else {
    result = 1;
  }
  if (result && !verbose) {
    cerr << "Try running with -v parameter to get more details." << endl;
  }
  return result;
}
