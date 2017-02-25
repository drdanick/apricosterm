Apricos terminal emulator
=========================

An SDL based terminal emulator for the Apricos CPU.


Dependencies
------------

Apricosterm has the following requirements:

- SDL2
- CMake 2.8+


Compiling
---------

Compiling Apricosterm is as simple as navigating to the base project directory 
in your favourite terminal, and entering the following commands:
```
mkdir build
cd build
cmake ..
make
```

This will place a compiled binary in the build/src directory.
In order to install the binary, simply enter the following as root:
```
make install
```


Running
-------

Apricosterm can be run on its own (without connecting to a running Apricos CPU) by
entering `apricosterm` into your terminal. This will run Apricosterm in an "echo back" mode,
which will print any character you enter directly to the terminal.

If you would like to use the terminal as intended (and actually connect it to a Apricos CPU),
then you will need to specify a FIFO named pipe file with the `-f` command line switch. Both
the Apricos CPU and Apricosterm must specify the same FIFO named pipe file.


Command line options
--------------------

Apricosterm supports the following command line arguments:  
`--version`          /  `-v`   Print version information and exit.  
`--help`             /  `-h`   Print program usage information.  
`--font`             /  `-t`   Set a custom font bitmap.  
`--fifo_tty_file`    /  `-f`   Set fifo tty file.


License
-------

See LICENSE for details.
