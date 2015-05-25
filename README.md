# SNESDev-RPi

SNESDev is a user-space driver for the Raspberry Pi.
It implements up to two (S)NES game controllers as HID gamepads and a single keyboard for as many buttons as you like connected over GPIO. 

## Installation
### Dependencies

Setup build system and get libconfuse, which is used for parsing the config file.
```shell
sudo apt-get update
sudo apt-get install -y build-essential git cmake libconfuse-dev
```

Build and install Raspberry Pi GPIO library.
```shell
mkdir ~/src
cd ~/src
wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.44.tar.gz
tar -xvf bcm2835-1.44.tar.gz
cd bcm2835-1.44
./configure
make
sudo make check
sudo make install
```

### Build SNESDev
```shell
mkdir ~/src
cd ~/src
git clone https://github.com/axle-h/SNESDev-RPi.git 
cd SNESDev-RPi
mkdir build && cd build
cmake -D CMAKE_BUILD_TYPE=Release ../
make
sudo make install
```

This will build & install SNESDev as a service and start it.

## Running
In order to run SNESDev make sure that the uinput module is loaded. You can check this with

```shell
lsmod
```

The module is loaded with

```shell
sudo modprobe uinput
```

If you want to have the uinput module automatically loaded, you can add "uinput" to the file ```/etc/modules```.

SNESDev is run as a service. To check whether it is running:

```shell
sudo service SNESDev status
```

You can start SNESDev with

```shell
sudo service SNESDev start
```

## Configuration

SNESDev is configured with the configuration file ```/etc/gpio/snesdev.cfg```.


## Uninstalling

You can uninstall the SNESDev service with the following command:

```shell
cd ~/src/SNESDev-RPi/build
sudo make uninstall
```
