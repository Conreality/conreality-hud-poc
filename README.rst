**************
Conreality HUD
**************

.. image:: https://img.shields.io/badge/license-Public%20Domain-blue.svg
   :alt: Project license
   :target: https://unlicense.org/

.. image:: https://img.shields.io/travis/conreality/conreality-hud/master.svg
   :alt: Travis CI build status
   :target: https://travis-ci.org/conreality/conreality-hud

|

https://wiki.conreality.org/HUD

Prerequisites
=============

Build Prerequisites
-------------------

* Clang_ (>= 3.4) or GCC_ (>= 5.0)
* `GNU Autoconf`_ (>= 2.69)
* `GNU Automake`_ (>= 1.15)
* `GNU Make`_ (>= 3.81)
* pkg-config_ (>= 0.29)

.. _Clang:        https://clang.llvm.org/
.. _GCC:          https://gcc.gnu.org/
.. _GNU Autoconf: https://www.gnu.org/software/autoconf/
.. _GNU Automake: https://www.gnu.org/software/automake/
.. _GNU Make:     https://www.gnu.org/software/make/
.. _pkg-config:   https://www.freedesktop.org/wiki/Software/pkg-config/

Dependencies
============

Installation
============

Installation from Source Code
-----------------------------

Configuring, building, and installing the program (by default, into
``/usr/local``) are all performed with the standard incantations::

   $ ./autogen.sh   # only needed for the development version from Git

   $ ./configure

   $ make

   $ sudo make install

Configuration
=============

Running
=======

Runtime keyboard hotkeys::

  ESC     - Exit the program.
  E       - Toggle an edge filter.
  Shift-E - Toggle to draw only edges.
  F       - Toggle image flipping.
  I       - Toggle on-screen items for a HMD.
  N       - Toggle on-screen name display.
  T       - Toggle on-screen time display.

See Also
========

* `Conreality Development Environment (DevBox)
  <https://github.com/conreality/conreality-devbox>`__
