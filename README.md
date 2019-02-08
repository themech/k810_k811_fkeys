# k810_k811_fkeys
Logitech K810/K811 Keyboard F-keys switcher for Linux (Ubuntu)

## Usage
`sudo k81x-fkeys [-d device_path] [-v] on|off`

"On" enables the regular functions of F1-F13 keys and special functions (like volume or brightness) are accessible after pressing the Fn key. "Off" does the opposite.

## Binary installation
If your system supports deb files, you can simply grab an automatic build from https://github.com/themech/k810_k811_fkeys/releases and install it:

`sudo dpkg -i k81x-fkeys_<version>_amd64.deb`

For Arch Linux you can use AUR package:
```
gpg --recv-keys --keyserver hkp://pool.sks-keyservers.net 7FBFBFD17A45CAE7
pacaur -S k81x-fkeys
```

## Installing from source code

### Building
Simply download the source and either run `make` or compile it yourself by running:

```g++ k81x-fkeys.cpp k81x.cpp -o k81x-fkeys -ludev```

### Installing via make
You can type `sudo make install` to install the binary and udev the scripts. 

### Manual installation
I find it convenient to setup an udev rule for this utility so it automatically sets the desired F-keys functions when the keyboard is connecting.

In order to do so copy the `k81x-fkeys` binary to some location (like `/opt/k81x/`) and add a following `k81x.sh` bash script in the same directory:

```
#!/bin/bash
if [ -z "$1" ];
then
    /opt/k81x/k81x-fkeys -s on
else
    /opt/k81x/k81x-fkeys -s -u $1 on
fi
```

You can change `on` to `off` it that's your desired setup. And specify a device path with `-d` switch in case the autodetection doesn't work. Also remember about the execute permission: `chmod a+x /opt/k81x/k81x.sh`.

Now it is time to hook this bash script into udev. Create `/etc/udev/rules.d/00-k81x.rules` with the following content:

```
ACTION=="add", KERNEL=="hidraw*", SUBSYSTEM=="hidraw", ATTRS{address}=="XX:XX:XX:XX:XX:XX", RUN+="/opt/k81x/k81x.sh"
```
The `XX:XX:XX:XX:XX:XX` should be replaced with your keyboard Bluetooth address. To find the address simply go to the Bluetooth settings (`All Settings>Bluetooth`), select the Logitech keyboard on the Devices list and copy its address displayed there. 
You can also skip the `ATTRS{address}=="XX:XX:XX:XX:XX:XX",` part. In that case `k81x.sh` will be also executed for other devices but won't do anything.

Note: On some systems the `address` attribute is not available. You can try removing it and using this generic rule instead:
```
ACTION=="add", KERNEL=="hidraw*", SUBSYSTEM=="hidraw", RUN+="/opt/k81x/k81x.sh %S%p"
```
This means the `k81x.sh` can also be triggered for other HID devices than you keyboard. But as we're passing
the device path as the parameter, calls from those devices will be ignored.
