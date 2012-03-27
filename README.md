Concurrent utils
================

A set of C++ header-only data structures to handle multithreaded tasks.  
It is currently relying on [Boost](http://www.boost.org/) for thread platform abstraction but a C++11 compliant version will be added in a near future.

This project is maintained by [Mikros Image](http://www.mikrosimage.eu) R&D Team and distributed under the MIT license.

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
This component is currently in use within [Duke](https://github.com/mikrosimage/duke) to enable image preloading but could be used whenever you need to hide latencies (i.e. I/O over disk or network).

Use
---

* include this folder
* link with boost_thread

Quick start
-----------

### Tests and examples

The simplest way to compile the provided tests and examples in a multiplatform way is to use **Boost.Build** system.

- First get the last version of Boost from the [website](http://www.boost.org/users/download/) and unzip where approriate (_you'll need Boost anyway_)
- export the BOOST_ROOT env and boostrap to generate **b2**

> export BOOST_ROOT=/path/to/boost  
> cd $BOOST_ROOT  
> ./booststrap.sh

- compile and run the tests  

> cd /path/to/concurrent_utils  
> $BOOST_ROOT/b2 --toolset=gcc tests

- compile the examples

> $BOOST_ROOT/b2 --toolset=gcc examples release

The examples will be available in the **examples/bin** directory.  

Unless mentioned otherwise _b2_ will create a debug version, in the case of the examples the release version is build.  
As for make you can add a -jX option  
If you want to use another compiler, just change the toolset ( clang, darwin, icc, msvc ... )  
If you need some more informations about Boost.Build have a look at [this page](http://www.highscore.de/cpp/boostbuild/) or at the [Boost.Build documentation](http://www.boost.org/boost-build2/doc/html/index.html)
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