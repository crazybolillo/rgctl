# rgctl
This is an STM8 based RGB LED controller. It uses a STM8S105K6 together with a 16 channel LED Driver (LED1642) to
control the color and brightness of up to 16 LEDs.

## Building
This project is meant to be built with [COSMIC's STM8 compiler](https://cosmic-software.com/stm8.php). The compiler can
be obtained for free by filling out the form on their website. If using Linux you can instead download the Eclipse
plugin, the toolchain is embedded in it as a `.tar.gz` file.

The Makefile provided with the project was built with GNU Make in mind, it is therefore advised to use GNU Make as well.
No attempt to make this build Windows compatible has been made.

## Development
Due to the Cosmic Compiler behaving in non-standard manners, it is difficult to integrate it with CMake, it also does
not generate a `compile_commands.json` file. It also introduces extensions like `@near` or `@interrupt`. All of this
makes it harder to natively integrate with IDEs.

CLion integration has been somewhat achieved by providing a `custom-compiler-cosmic.yaml`, it works with the Makefile
and is able to index include directories. To take advantage of this, configure the
[custom compilers](https://www.jetbrains.com/help/clion/custom-compilers.html) feature on your IDE.
