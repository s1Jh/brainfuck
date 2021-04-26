# brainfuck
C brainfuck interpreter I made in 2 days, it's not the best piece of work ever, but I tested all features and they all seem to be working just fine.

## usage (ripped straight from the program):
```
Usage: brainfuck [arguments] FILE

Arguments:
  -h/--help                 Displays this message.
  -t/--tape [int]           Sets the emulated tape size (default: 30000).
  -p/--program [int]        Sets the emulated program storage size (default: 30000).
  -d/--debug                Enables program debugging.
  -tm/--tape-mode [string]  Sets how the tape behaves when programs index over the specified limit. Acceptable values are "terminate", "wrap" and "clamp".
```
