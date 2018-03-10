# Motd

Motd is a hilariously over-engineered utility to deliver a single quote
("motto-of-the-day", hence the name) per invocation, picked at random
from a pool of predefined messages.

## Background

The roots of this code reach way back to the early nineties, when it had
to run on an excruciatingly slow machine using a random number generator
with notoriously abysmal distribution. To be honest, a half decent
replacement could probably be written in a few lines of shell or awk
script.

Think of it as a fun proof-of-concept project way past its expiration
date, that I nonetheless kept up-to-date and over the decades ported
to whatever platform I was using at the time. While its usefulness is
debatable, some of the quotes are still quite entertaining.

## Build

Run `make`. You may then edit the generated `config.h` file to match
your tastes and rebuild with the modified settings.

## Installation

Put the binary wherever you see fit. Running `motd -h` displays a short
help text that will give you an idea of how to invoke it and where to
place the quotes file. Enjoy!

## License

Motd is distributed under the Modified ("3-clause") BSD License. See
`LICENSE` file for more information.

----------------------------------------------------------------------
