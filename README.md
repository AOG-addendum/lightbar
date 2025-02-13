# esp32-lightbar
Software to display a lightbar using AgOpenGPS.

* The ESP32 will create a Wifi hotspot and try to connect to the main Wifi. If main Wifi is found, ESP32 hotspot will discontinue. If ESP32 hotspot is connected to, main Wifi will no longer try to connect
* ESP32 hotspot will be called Lightbar Module XXXXXX. with the MAC address of the ESP32 appended to avoid confusion between multiple ESP32s

# Features
* Uses a multi-threaded aproach. Tries to use other libraries as much as possible and is clearly structured with meaningful names
* Cool and comfortable WebUI, automatically creates a hotspot on first start, the Wifi to connect to can be configured in the WebUI
* Everything is configured from the WebUI.
* Settings are stored into flash on user request. New features normally require an apply&reboot cycle to activate, the WebUI clearly shows this.
* Uses ESP32 tasks for updating Neopixel, not assembly code designed for Arduino family of chips
* JSON config downloads will be named after Hostname to reduce confusion from multiple downloads
* Lightbar distance per XTE will self-correct for erroneous values.
* Has CDS input for brightness sensing and potentiometer for user adjustment

# Caveats
* For best results, use the lightbar board found in the [Github repo](https://github.com/AOG-addendum/PCB-modules/tree/master/Machine%20control)
* **As the WebUI is quite taxing on the Websocket implementation; the ESP32 can crash without warning if left connected too long. Without open connection to a browser, no such crashes are documented. On my hardware, a longtime stresstest of more the five days of uptime was completed successfully, if the tab with the WebUI is closed after using it.** The cause of the crashes is in the implementation of the used TCP-stack/Websocket-API. Not much can be done about it, as it is the default implementation which comes with framework (ESPAsyncWebServer), is really fast/performant and is used by the library to generate the WebUI.

# Schematics

![Schematics](doc/schema.png)

The configuration has to match the connections for the ESP32 and the work and steer switches. This schematics is with modules from Adafruit, but you can use other brands, if you adjust the firmware for it. This is usually done by changing the `board` in `platformio.ini`. 

An example of a itemlist is below:

Amount | Id | Supplier | Description
--- | ---- | --------- | ----------------------------------------------------------------------------
1x  |      | Adafruit  | QT Py ESP32-S3 Board
1x  |      | PCB       | PCB from [here](https://github.com/AOG-addendum/PCB-modules/tree/master/Machine%20control) name: Lightbar Rev x.fzz; use latest version

## PCB module
All docs you need should be in the Lightbar PCB repo. If parts are missing in the Readme, you can install Fritzing and open the .fzz file, the parts should be listed with the proper values.

## Windows
Follow this [guide](http://iot-bits.com/esp32/esp32-flash-download-tool-tutorial/), but enter the files/addresses as below:

Address   | File
--------- | ----------------------
`0x1000`  | `bootloader_dio_40m.bin`
`0x8000`  | `partitions.bin`
`0xe000`  | `boot_app0.bin`
`0x10000` | `firmware.bin`

Tick the option to enable the flashing of these files and press on `Start`. The defaults should work, but depending on your version of the ESP32, other settings are necessary, which you can find in the documentation of the manufacturer.

## Linux / command line
Install `esptool.py`, preferably with the packet management system of your distribution. Open a new terminal window and change to the directory with the already decompressed archive. Then enter:
```
esptool.py --chip esp32 --before default_reset --after hard_reset write_flash --flash_size detect 0x1000 bootloader_dio_40m.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 firmware.bin
```

## OTA update
If you already have flashed a version of it on you ESP32, you can navigate to the update page by clicking on the link in the last tab. Then updload the file `firmware.bin`. Attention: this firmware uses a custom flash-partition-layout, so it has to be flashed once by a flasher as described above. Using another OTA flasher (like an OTA example) doesn't work.


# Installation
TL;DR: If you want to do your own development, you have to install [platformio](https://platformio.org/), clone the repository with all the submodules, build and upload it.

## Windows
### Warning before you start
Read this file through, before starting to half-ass it. It is not so hard to get a working system, just give it enough time and install it in this order.
Some packets take a real long time to install with no visible progress. Just wait until finished.

Atom.io does no longer work with PlatformIO as described previously. This code base now works with Visual Studio.

### Install Prerequisites (updated for VS Code)
1. install VS Code: https://code.visualstudio.com/
1. inside VSCode:
   1. click on File>Preferences>Extensions
   1. search for "PlatformIO IDE" and install it
   1. it's taking forever, so please be patient
   1. restart as asked
1. install git: https://git-scm.com/downloads
1. install all needed drivers for your platform. This is usually done by installing the CP210x-driver, but consult the documentation of the manufacturer of your esp32.

### Downloading the repository
1. open a folder in the explorer, preferably not too deep inside the drive. `C:\` or a folder under it should work
1. right click on it and choose "Git Bash Here"
1. enter `git clone --recursive https://github.com/AOG-addendum/lightbar`

### Compiling
1. right click the created folder and open with VS Code
1. wait for PlatformIO to finish loading tasks (status is shown along the bottom)
1. click build (the button with the tick along the bottom), the missing dependencies should be installed automatically

### Upload to the ESP32-S3
1. connect the ESP32 over USB
1. click on upload (the button with the arrow)

Alternatively you can use the OTA-update in the WebUI: go to the last tab and upload a new image.

## Linux
Install `platformio` with the package management system of your distribution and find a guide to help you set it up for your particular editor/IDE.

To do it on the console, clone the repository with all the submodules (`git clone` with `--recurse-submodules`), then change into the cloned folder and enter:
```
platformio run -t upload
```

This takes care of all the required libraries and uploads it to a connected ESP32.

## Configuration

All configuration is done in the WebUI. To connect to the created hotspot of the esp32, using a mobile device is normally the simplest solution.

To open a web page of the ESP32, type 192.168.xxx.83 into your web browser if using a dedicated router Wifi, or 192.168.1.1 if using the built in hotspot on initial setup. Alternatively, connect the ESP32 to the USB and open a monitor on it. It should print the SSID/IP address when booting.

After login in to the WebUI, you can then change the wifi to whatever you like. The esp32 tries to login, and if that fails, makes a new hotspot with the given ssid/password. 

**The configuration is saved as long as there is no complete clearing of the flash of the esp32.** To  reset to the defaults, you either press the button in the WebUi or erase the flash of the esp32 with platformio/your editor. A new upload of the firmware doesn't affect the saved settings. 

## Updating the repository
As there are sometimes new repositories/submodules added, enter the following to update:
1. go to the esp32-aog repository and open a git bash-terminal
1. `git pull`
1. `git submodule sync`
1. `git submodule init`
1. `git submodule update`

Repeat them as needed.

# Donation
If you like the software, you can donate me some money.

[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://paypal.me/eringerli)
