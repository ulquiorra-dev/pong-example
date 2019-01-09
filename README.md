# Wii U Pong Example
This is a re-write of the long-existing Wii U Pong example project, with a greatly simplified codebase that should be easier for less-experienced programmers to understand, and a revised build system that allows building the program using the latest Wii U tools.

Note: The program is in an almost-finished state - the only thing I believe is missing is that the TV screen does not get used, the game only displays on the GamePad screen. The TV screen is larger than the GamePad screen, and so ideally, I would want to scale the screen contents up so that they fit on the TV screen. This can be performed using AVM functions, but this has proven to be difficult, and so for now, the TV screen is unused.

## History and Rationale
Pong was originally released for the Wii U mid-2015 as an example project for [libwiiu](https://github.com/wiiudev/libwiiu), a method of creating software payloads that run within an exploited web browser on the Wii U. Limitations of the browser/exploit environment meant that certain features were unsupported, such as global variables and simple access to function exports. Workarounds were used, such as storing all the variables in a `struct` and passing it as an argument to all of the functions, but as a programming newcomer, this made the code somewhat more difficult for me to understand.

When the [Homebrew Launcher](https://github.com/dimok789/homebrew_launcher) was released, giving us the ability to reliably load software in the ELF format from an SD card, [the Pong example was ported over](https://github.com/dimok789/pong_port), and aside from a new method of creating/managing the screen framebuffer, the code inherited many of the quirks of the browser exploit version.

At this point, [WUT](https://github.com/decaf-emu/wut) was in a functional state, allowing Homebrew developers to compile software into the Wii U's native RPX executable format, which is an ELF executable with fancy things like compression and a custom dynamic linking system. The HBL version of Pong was [subsequently ported to WUT](https://github.com/decaf-emu/wut/tree/legacy_cmake/samples/pong) and provided as an example project within the WUT source repository on GitHub.

WUT subsequently went through a few more iterations, and as the Pong example continued to be left unmaintained, it was removed from the repository.

I set out to re-write the Pong example such that it would not only be compatible with the latest Wii U homebrew tools, but such that the quirks of the browser environment are removed, making the code (hopefully) easier to understand.

This project was written after an extended absence from Homebrew development due to increased workload in other aspects of my life, and so it also served to me as a re-introduction to Homebrew programming.

## Repository Contents
This Git repository contains three major components - the build script (`CMakeLists.txt`), the source code (in the `src` folder) and the deployment assets (in the `deploy` folder). Each serve their own purpose and are of importance to the project.

## Building
If you have not already, [follow these instructions](https://github.com/yawut/ProgrammingOnTheU) to set up devkitPPC and WUT on your computer.

If you have Git installed on your machine, you can clone this repository and its history by simply running:

```
$ cd <wherever you want the repository to be stored, e.g. ~/Desktop/projects>
$ git clone https://github.com/CreeperMario/pong-example
$ cd pong-example
```

Alternatively, GitHub allows you to download a snapshot of the repository in its current state as a zip archive.

From here, you can create a build directory and generate the build files.

```
$ mkdir build
$ cd build
$ cmake ../
```

And finally, if all goes well, you should be able to build the program using:

```
$ make
```

This will generate `pong.rpx` which, on its own, is not very useful.

If you have `wiiload` installed, and your computer and Wii U are connected to the same network, you can remotely launch the game on the console. First, open an app like [FTPiiU](https://github.com/dimok789/ftpiiu) which shows the console's IP address, and then run the following command on your computer:

```
$ export WIILOAD=tcp:<ip address goes here>
```

You can then open the Homebrew Launcher and run the game on your console using:

```
$ make run
```

The `deploy` folder contains a number of assets that can be used to package the program with icons, metadata and splash screens, allowing you to distribute Pong as a Homebrew Launcher app or an installable channel app on your console.

```
$ make install
```

This will create the folder `build/hbl_sd_root` which contains a `wiiu` folder that can be copied to your SD card and used to distribute an HBL version of Pong.

There is also a `build/unpacked_root` folder which can be used with Decaf or Loadiine GX2.

If your computer has [`makefst`](https://github.com/shinyquagsire23/makefst) installed, the install step will also create `build/wupinstaller_sd_root` which contains an `install` folder that can be copied to your SD card, allowing  Pong to be installed on your console using WUPInstaller Y-Mod or WUPInstaller GX2. There is also a `pong-example.woomy` file which can be used to install the program using Woomïnstaller or Woomïnstaller GX2.

These deployment folders are also automatically zipped and are ready to distribute.

Note that your console will need to be running an IOSU custom firmware (e.g. [Mocha](https://github.com/dimok789/mocha), [Haxchi](https://github.com/FIX94/haxchi)) or some other method of preventing IOSU from trying to verify signatures in order to install/run the channel version of Pong. I only ever tried installing Pong on redNAND, but it should be installable on a sysNAND CFW as well.

## How it works
The code files and build scripts are very thoroughly documented using code comments, simply open any of the source files to find all the information you should need.

## Deployment Assets
Inside the `deploy` folder are two folders which contain the assets for two different distributions of the app. The `hbl` folder contains the `icon.png` and `meta.xml` files to make the program look good in the Homebrew Launcher's list of apps. And the `channel` folder contains the `code` and `meta` (`content` is unused in this example) for distributing the app as an installable 'channel' app on the Wii U.

I created all the icons and splash screens myself using GIMP. The XML files were also written by myself, based on examples provided by the Homebrew Launcher source code. The channel boot sound is simply five seconds of silence created using Audacity and `wav2btsnd`. The `bootMovie.h264` file was ~~stolen~~ borrowed from the Homebrew Launcher.

## License
The contents of this repository are available under the GNU General Public License, version 3 or later. Be sure to read the license text (see `LICENSE.md` in the repository) before making use of this software, but basically, you're free to use this program and its corresponding source code for your own projects, as long as you make your program and source code freely available, and you credit the developers who worked on this. You also cannot hold me or any other developer responsible for any damage to your console or other property caused as a result of downloading or using the program or its source code.

## Credits
The contents of this repository are mine, but of course, are based on previous iterations of the Pong example project. So credit is given to all previous contributors to the project (see the top of any source code file for more information).

In addition to this, I'd also like to thank everyone who contributed to development of Wii U Homebrew software, including those who developed software exploits, created the build tools we use, performed research into the SDK libraries and system functionality of the Wii U and developed other Homebrew software. You were all a huge inspiration to myself, and I wouldn't be writing this right now if it wasn't for your contributions. Thank you all so much!

I also want to thank the Homebrew community for showing interest in development activity and giving me goals to work towards. You have also helped inspire me to pursue Homebrew development.

## Contact
I have spent the last few days writing nothing but documentation, and I eventually reached the point where I couldn't be bothered anymore. So some information may be incorrect or poorly explained.

So if there's anything here you don't understand, or you otherwise want to ask questions or say hi, you can contact me via [GBAtemp](https://gbatemp.net/conversations/add?to=CreeperMario), [Twitter](https://twitter.com/CreeperMario258), [Mastodon](https://fosstodon.org/@creepermario), e-mail (see my [GitHub profile](https://github.com/CreeperMario)) or Discord (CreeperMario#6152, you can find me in the Decaf, Wii U Plugin System and For The Users Discord servers)
