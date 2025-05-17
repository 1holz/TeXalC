# TeXalC

_This is in an alpha state_

This will become a little calulator which is able to parse LaTeX.
Currently it can add, subtract, multiply and divide big ints (division can currently be quite slow for very large numbers).
May or may not have some memory leaks (working on it), etc.

### Usage examples
```
1 + 1 \\
= 2 \\

1 - 1 \\
= 0 \\

2 \cdot 2 \\
= 4 \\

\frac{15}{6} \\
= \frac{5}{2} \\
```
Can also parse binary (`0b`/`0B`) and hexadecimal (`0x`/`0X`).

### Build process
It should run in any environment complying with POSIX.1-2001 or newer.
For building the following additional requirements have to be met:
- bison instead of yacc
- GNU make instead of make

Can be build by just running `make` in the top directory.
