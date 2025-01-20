# Raspberry Pi Pico FreeRTOS Lightweight IP Stack (LWIP) Example

This is an example of how to use the [FreeRTOS](https://www.freertos.org/) LWIP (Lightweight IP) stack on the [Raspberry Pi Pico W and Pico2 W](https://www.raspberrypi.com/products/raspberry-pi-pico/). Creating it was not a pleasant process. I hope using it is.

At present there is a bug in the [Pico SDK 2.1.0](https://github.com/raspberrypi/pico-sdk) support for the Infineon CYW43439 device on the Pico W (the part that provides Wifi and Bluetooth support), so this repo uses a fork containing the fix, which I found documented in [issue 2101 in the Pico SDK repo](https://github.com/raspberrypi/pico-sdk/issues/2101). But we are getting ahead of ourselves.

You should see my main [Pico/FreeRTOS example repo](https://github.com/tlberglund/pico-freertos-example) for more details about how to build this project.

I owe a substantial debt to [@jondurrant](https://github.com/jondurrant)'s [Pico W SNTP example](https://github.com/jondurrant/RPIPicoWSNTP), which didn't run for me, but formed the basis of this work.

## What Does This Actually Do?

This project joins a wifi network, uses DHCP to get an IP, and starts an SNTP (Simple Network Time Protocol) thread that fetches the time every hour. You might not need desperately need NTP in your life, but this is a springboard into anything you want to do with the TCP/IP stack. There are several other application-level protocols implemeted in the Pico SDK in the same directory the NTP code is located in (peruse `CMakeLists.txt` for this path), so you can go have some fun. By which I mean hours of frustration culmiating in a mildly satisfactory result.

The poorly-documented `WifiConnection` class is derived from [@jondurrant](https://github.com/jondurrant)'s helpful WifiHelper class. It's a singleton that provides basic services to initialize the CYW43 SoC and join the configured wireless network. It does the intializing and network-joining in a FreeRTOS thread that continually checks the network status and attempts to rejoin as needed. The `wait_for_wifi_init()` method allows other threads to block on the networking joining. There are probably bugs lurking in the un-joining and re-joining code, which is not super battle-hardened.

Right now it's got a bunch of chatty debug code in it that I hope to upgrade to some kind of sensible logging framework soon, as if there's any such thing as a sensible logging framework. And when it's finished booting up and joining the network, it won't emit any further debug—it'll just sit there, updating the time once an hour, not saying anything—a substantially blank canvas ready to receive your contributions, like the pretentious, entitled, angst-ridden LiveJournal page you never had, except it's an embedded system.

## Secrets

The SSID and password aren't stored in this repo. We are not animals.

There is a file called `include/secrets.h` which is listed in `.gitignore`. It should look like this (but with your actual SSID and password):

```C
#define WIFI_SSID "promisedlan"
#define WIFI_PASSWORD "tellyourwifisaidhi"
```

The project won't build without it.

## Required Tooling

I've only run this on MacOS, and if I recall correctly, you'll need these tools installed for a good command-line experience:

* `brew install picotool`
* `brew install cmake`
* `brew install ninja`
* `brew install minicom` (optional)


## Supported Platforms

* Pico W ([datasheet](https://datasheets.raspberrypi.com/picow/pico-w-datasheet.pdf))
* Pico 2W ([datasheet](https://datasheets.raspberrypi.com/picow/pico-2-w-datasheet.pdf))

The Pico 2W is a new device as of this writing, and getting this build running on it was virtually zero hastle compared to the process on the Pico W. The build is configurable via the `PICO_BOARD` variable in the CMake build (see Building below). Towards the top of `CMakeLists.txt`, you'll see these lines:

```CMake
set(PICO_BOARD pico_w CACHE STRING "Board type")
# set(PICO_BOARD pico2_w CACHE STRING "Board type")
```

Uncomment just one of them, and you're off and running.

## Cloning

This repo uses submodules for the [Pico SDK](https://github.com/raspberrypi/pico-sdk) and [FreeRTOS](https://github.com/FreeRTOS/FreeRTOS-Kernel). Look, I'm sorry. It's C and C++ code. This is how we do it here.

The submodules are set to use my own fork of the Pico SDK with an unreleased bug fix in it, plus an unreleased version of FreeRTOS. What can I say? It's a cursed creation. We soldier on.

To clone the repo, get into your favorite working directory and type:

`git clone --recurse-submodules https://github.com/tlberglund/pico-freertos-example.git`

But we both know you already cloned without that submodule switch. If that's you, then get into the directory you cloned into and type:

`git submodule update --init --recursive`

Next time, read the README first.

## Building

The build uses [CMake](https://cmake.org) and [Ninja](https://ninja-build.org/), which is standard for Pico C/C++ projects. If you've found this repo because you're looking to get started on the Pico, you might also be new to CMake, for which I affirm you. Of course, if you're jumping in to the Pico and CMake and hoping to build a project that uses an RTOS and TCP/IP all at the same time, that's probably a bit too ambitious, but I know you're not going to listen to me. So just get into the clone directory and type these things:

1. `mkdir build`
2. `CMake -G Ninja -S . -B build`
3. `ninja -C build`

Now, CMake "isn't a build system," but "is actually a system for describing a build," which the incredibly annoying kind of thing that the authors of build systems generally say. But its output really is a listing of commands for an Actual Build Tool called Ninja, which is what invokes the compiler and linker. If you're iterating on an example and changing only your C or C++ code, you can just re-run the `ninja` command without regenerating the build with CMake. This actually is as fast as it claims to be, and isn't a terrible workflow once you get into it. There are around 200 source files between FreeRTOS and PicoSDK before we even get to `main.cpp`, so building only your changes is a big win.

## Debugging

Remember up top when I told you to see my main [Pico/FreeRTOS example repo](https://github.com/tlberglund/pico-freertos-example) for more details about this project? Well, seriously, go do that. It's got some good stuff about debugging there.
