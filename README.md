# k810_k811_fkeys
Logitech K810/K811 Keyboard F-keys switcher for Linux (Ubuntu)

## Usage
`sudo k81x_fkeys [-d device_path] [-v] on|off`

## Building
`g++ k81x_fkeys.cpp k81x.cpp -o k81x_fkeys -ludev`

## Installation

I find it more convenient to setup an udev rule for this utility so it automatically sets the desired F-keys functions when the keyboard is connecting.

In order to do so copy the `k81x_fkeys` binary to some location (like `/opt/k81x/`) and add a following `k81x.sh` bash script in the same directory:

```
#!/bin/bash
if [ "$ACTION" == "add" ];
then
    /opt/k811/k81x_fkeys on
fi
```

You can change `on` to `off` it that's your desired setup. And specify a device path with `-d` switch in case the autodetection doesn't work.

Now it is time to hook this bash script into udev. Create `/etc/udev/rules.d/00-k81x.rules` with the following content:

```
KERNEL=="hidraw*", SUBSYSTEM=="hidraw", ATTRS{address}=="XX:XX:XX:XX:XX:XX", RUN+="/opt/k811/k81x.sh %p"
```

The `XX:XX:XX:XX:XX:XX` should be replaced with your keyboard Bluetooth address. To find the address simply do to the Bluetooth setting (`All Settings>Bluetooth`), select the Logitech keyboard in the Devices list and copy its address displayed there.
