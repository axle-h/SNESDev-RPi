SNESDev-RPi
===========

SNESDev is a user-space driver for the RetroPie GPIO Adapter for the Raspberry Pi. It implements two (S)NES game controllers and a virtual keyboard for up to two (S)NES controllers and a button that are connected to the GPIO pins of the Raspberry Pivia the RetroPie GPIO Adapter (http://blog.petrockblock.com/2012/10/21/the-retropie-gpio-adapter/). 

Installation
------------

Manual installation:

First of all, make sure that Git is installed:

```shell
sudo apt-get update
sudo apt-get install -y build-essential git cmake libconfuse-dev
```


SNESDev is downloaded and installed with

```shell
cd
git clone git://github.com/axle-h/SNESDev-RPi.git
cd SNESDev-RPi
mkdir build && cd build
cmake -D CMAKE_BUILD_TYPE=Release ../
make
sudo make install
```

The lines above build and install two needed libraries and SNESDev-Rpi. The sudo-command is needed for the installation of the libraries.

Running
-------

In order to run SNESDev mae sure that the uinput module is loaded. You can check this with

```shell
lsmod
```

The module is loaded with

```shell
sudo modprobe uinput
```

If you want to have the uinput module automatically loaded, you can add "uinput" to the file 
/etc/modules.

SNESDev has to be run as background process with

```shell
sudo service SNESDev start
```

In order to access the uinput device SNESDev has to be run as root. This is (obviously) not so nice and is currently an issue. If you have a solution or suggestion for that, feel free to submit a pull request or send me a mail!

Configuring SNESDev-Rpi
-----------------------

SNESDev-Rpi is configured with the help of the configuration file ```/etc/gpio/snesdev.cfg```.



Uninstalling SNESDev service
----------------------------

You can uninstall the SNESSDev-Rpi service with the following command:

```shell
sudo make uninstall
```
 

Have fun!


Raspberry Pi is a trademark of the Raspberry Pi Foundation.
