# Wireless-tool-in-C

## Introduction
This project is writing wireless tool that can do the following in C:
 - Capture wireless frames
 - Build and send wireless frames

## Installation

Go to your working directory and clone this repository via `git clone` command.

```Bash
$ git clone https://github.com/david232818/Wireless-tool-in-C.git
```

Then "Wireless-tool-in-C" directory will be made in your working directory. Go to this repo directory and execute `make` command.

```Bash
$ cd Wireless-tool-in-C
$ make
``` 

Above `make` command will compile the source and generate a elf file called "wltools". If this executable is generated, execute it with `-c` option and your wireless adapter's name.

```Bash
$ sudo ./wltools -c [your wireless adapter's name]
```

If you typed it correctly, the program will start capturing.
