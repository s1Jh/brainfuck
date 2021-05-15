# brainfuck
C brainfuck interpreter I made in 2 days, it's not the best piece of work ever, but I tested all features and they all seem to be working just fine.

### usage:
```
Usage: 
    brainfuck [arguments] FILE
    
Arguments:
    -h/--help                 Displays this message.
    -t/--tape [int]           Sets the emulated tape size (default: 30000).
    -p/--program [int]        Sets the emulated program storage size (default: 30000).
    -d/--debug                Enables program debugging.
    -tm/--tape-mode           Sets how the tape behaves when programs index over the specified limit.
    -i/--image                Enables image output.
    -iw/--image-width [int]   Sets the width of the output ascii image, also enables image mode (default: 128).
    -ih/--image-height [int]  Sets the height of the output ascii image, also enables image mode (default: 64).
```
### compilation:
Any standard C compiler should be able to compile it with any sensible combination of arguments, but I recommend:
```
gcc -Wall -O3 -o brainfuck brainfuck.c
```
