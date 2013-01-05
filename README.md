Concurrent utils
================

A set of C++11 header-only data structures to handle multithreaded tasks.

This code is highly inspired by [Anthony Williams](https://twitter.com/a_williams) [C++ Concurrency in Action](http://www.manning.com/williams/)

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

- - -

Use
---

* include this folder

- - -

Quick start
-----------

### Tests and examples
> make examples

Provides a few examples of how to use the library.

> make test

You will need [gtest](http://code.google.com/p/googletest/) to build the test suite.

- - -

Tested compilers
----------------

* GCC 4.7.2 on Gentoo Linux
* ~~Clang 3.2~~ runs into this [GCC issue](http://clang-developers.42468.n3.nabble.com/Problem-with-gnu-libc-4-7-s-chrono-in-Clang-3-2-td4029343.html)

- - -

Licence
---------
MIT license

Copyright
---------

Copyright (C) 2011-2013 Guillaume Chatelet. See LICENSE.txt for further details.