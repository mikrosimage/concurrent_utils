Concurrent utils
================

A set of C++ header-only data structures to handle multithreaded tasks.  
It is currently relying on [Boost](http://www.boost.org/) for thread platform abstraction but a C++11 compliant version will be added in a near future.

This project is maintained by [Mikros Image](http://www.mikrosimage.eu) and distributed under the MIT license.

Design Rational
---------------
Emphasis has been put on minimal interfaces and correctness.

- - -

Provided facilities
-------------------

Have a look at the _example_ folder for some code. But here is a quick tour :

### concurrent::notifier
* A simple object allowing to wait for an acknowledgement.

### concurrent::slot
* An object holder with notification capabilities.

### concurrent::queue
* An unlimited concurrent queue for passing messages between threads.

### concurrent::bounded_queue
* A bounded concurrent queue for passing messages between threads.

### concurrent::cache::lookahead_cache
* A cache that fills itself automagically with the help of one or more worker threads.
This component is currently in use within [Duke](https://github.com/mikrosimage/duke) to enable image preloading but could be use anywhere you need to hide latencies (i.e. I/O over disk or network).

- - -

Tested compilers
----------------

* GCC 4.4.1 on OpenSUSE 11.2
* Clang 3.0 on Gentoo
* GCC 4.5.3 on Gentoo
* GCC 4.6.2 on Gentoo
* GCC 4.5.2 on Windows XP [GCC TDM](http://tdm-gcc.tdragon.net)

- - -

Copyright
---------

Copyright (C) 2011-2012 Guillaume Chatelet. See LICENSE.txt for further details.