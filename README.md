# Wii U Pong Example
This is a re-write of the long-existing Wii U Pong example project, with a greatly simplified codebase that should be easier for less-experienced programmers to understand, and a revised build system that allows building the program using the latest Wii U tools.

## History and Rationale
Pong was originally released for the Wii U mid-2015 as an example project for [libwiiu](https://github.com/wiiudev/libwiiu), a method of creating software payloads that run within an exploited web browser on the Wii U. Limitations of the browser/exploit environment meant that certain features were unsupported, such as global variables and simple access to function exports. Workarounds were used, such as storing all the variables in a `struct` and passing it as an argument to all of the functions, but as a programming newcomer, this made the code somewhat more difficult for me to understand.

When the [Homebrew Launcher](https://github.com/dimok789/homebrew_launcher) was released, giving us the ability to reliably load software in the ELF format from an SD card, [the Pong example was ported over](https://github.com/dimok789/pong_port), and aside from a new method of creating/managing the screen framebuffer, the code inherited many of the quirks of the browser exploit version.

At this point, [WUT](https://github.com/decaf-emu/wut) was in a functional state, allowing Homebrew developers to compile software into the Wii U's native RPX executable format, which is an ELF executable with fancy things like compression and a custom dynamic linking system. The HBL version of Pong was [subsequently ported to WUT](https://github.com/decaf-emu/wut/tree/legacy_cmake/samples/pong) and provided as an example project within the WUT source repository on GitHub.

WUT subsequently went through a few more iterations, and as the Pong example continued to be left unmaintained, it was removed from the repository.

I set out to re-write the Pong example such that it would not only be compatible with the latest Wii U homebrew tools, but such that the quirks of the browser environment are removed, making the code (hopefully) easier to understand.

This project was written after an extended absence from Homebrew development due to increased workload in other aspects of my life, and so it also served to me as a re-introduction to Homebrew programming.

## Repository Contents
This Git repository contains three major components - the build script (`CMakeLists.txt`), the source code (in the `src` folder) and the deployment assets (in the `deploy` folder). Each serve their own purpose and are of importance to the project.

## License
The contents of this repository are available under the GNU General Public License, version 3 or later. Be sure to read the license text (see `LICENSE.md` in the repository) before making use of this software, but basically, you're free to use this program and its corresponding source code for your own projects, as long as you make your program and source code freely available, and you credit the developers who worked on this. You also cannot hold me or any other developer responsible for any damage to your console or other property caused as a result of downloading or using the program or its source code.

## Credits
The contents of this repository are mine, but of course, are based on previous iterations of the Pong example project. So credit is given to all previous contributors to the project (see the top of any source code file for more information).

In addition to this, I'd also like to thank everyone who contributed to development of Wii U Homebrew software, including those who developed software exploits, created the build tools we use, performed research into the SDK libraries and system functionality of the Wii U and developed other homebrew software. You were all a huge inspiration to myself, and I wouldn't be writing this right now if it wasn't for your contributions. Thank you all so much!

I also want to thank the Homebrew community for showing interest in development activity and giving me goals to work towards. You have also helped inspire me to pursue Homebrew development.

## Contact
If there's anything here you don't understand, or you otherwise want to ask questions or say hi, you can contact me via [GBAtemp](https://gbatemp.net/conversations/add?to=CreeperMario), [Twitter](https://twitter.com/CreeperMario258), [Mastodon](https://fosstodon.org/@creepermario) or e-mail (see my [GitHub profile](https://github.com/CreeperMario)), or by posting in #technical-discussion on the [Wii U Plugin System Discord server](https://discord.gg/bZ2rep2).
