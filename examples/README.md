# Example programs

This folder contains a set of example mathplot programs to help
new users to get started using the library.

These examples will build alongside the unit tests when you do a
mathplot build like this:

```bash
# Clone mathplot if you didn't already
git clone git@github.com/sebsjames/mathplot --recurse-submodules
cd mathplot
mkdir build
cd build
cmake ..
make
cd ..
```
You'll find the example program binaries in `build/examples`.

The examples are almost all built on the mplot::Visual environment,
which means you can interact with the mouse. Right-button down allows
you to drag, Left-button down allows you to rotate. Press 't' to
change the axis of rotations. Press 'h' and have a look at stdout to
see some other key presses. 'x' exits. 'a' Resets the view.

There's also a [gallery of the example programs](https://sebsjames.github.io/mathplot/) on the reference site.