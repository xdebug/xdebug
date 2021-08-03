Xdebug
======

.. image:: https://github.com/xdebug/xdebug/workflows/Build/badge.svg
   :target: https://github.com/xdebug/xdebug/actions?query=workflow%3ABuild
.. image:: https://ci.appveyor.com/api/projects/status/glp9xfsmt1p25nkn?svg=true
   :target: https://ci.appveyor.com/project/derickr/xdebug
.. image:: https://circleci.com/gh/xdebug/xdebug/tree/master.svg?style=svg
   :target: https://circleci.com/gh/xdebug/xdebug

Xdebug is a debugging tool for PHP. It provides step-debugging and a whole
range of development helpers, such as stack traces, a code profiler, features to
dump the full execution of your script to a file, and more.

Requirements
------------

Xdebug requires a `supported version <https://www.php.net/supported-versions.php>`_ of PHP. For
installation it requires the `pecl` tool (available through the `php-pear`
package), unless your Linux distribution has an Xdebug package (`php-xdebug`).

Installation
------------

On most Linux distributions you can install Xdebug through its package
manager. You can also compile from source with the `pecl` tool through `pecl
install xdebug`. The latter also works for MacOS as long as PHP is installed
with Homebrew.

On Windows, you need to `download <https://xdebug.org/download#releases>`_ a
binary. Use the `Wizard <https://xdebug.org/wizard>`_.

Unless you have installed Xdebug with a package manager on Linux, you also
need to add the following line to your `php.ini` file, or create a new Xdebug
specific ini file `xdebug.ini` in the `conf.d` directory. In either case, it
needs the following line added::

	zend_extension=xdebug

For more extensive installation instructions, see the documentation at
https://xdebug.org/docs/install

Configuration
-------------

Most features in Xdebug have to be opted in into. Each feature has a specific
opt-in. For example to use the `step debugger
<https://xdebug.org/docs/remote>`_ you need to set `xdebug.remote_enable=1` in
your configuration file. The step debugger requires an IDE (client), of which
there are many `available <https://xdebug.org/docs/remote#clients>`_.

The documentation has instructions for each of Xdebug's features:
https://xdebug.org/docs/ and a full list of `settings
<https://xdebug.org/docs/all_settings>`_ is also available there.

Contributing
------------

Xdebug is written in C, and extensive knowledge of PHP's internals is
necessary to be able to contribute. Contributing guidance is available
`separately <https://github.com/xdebug/xdebug/blob/master/CONTRIBUTING.rst>`_.

Before you begin to contribute, please reach out first. Either through email
(address at the bottom), an issue in the `issue tracker
<https://bugs.xdebug.org>`_ or preferably through IRC on Freenode's #xdebug
channel.

Testing
-------

If you are familiar with compiling PHP extension from source, have a local
checkout of Xdebug's GitHub repository, and have compiled Xdebug in that
directory following the instructions under `installation
<https://xdebug.org/docs/install#source>`_ you can run Xdebug's tests by
running::

	php run-xdebug-tests.php

The test framework requires that the PHP binary on the path has Xdebug loaded,
with remote debugging enabled through `xdebug.mode=debug`. It is possible
to skip remote debugging tests by exporting the `SKIP_DBGP_TESTS=1` environment
variable.

The `SKIP_UNPARALLEL_TESTS=1` can be used to skip tests that can not run in
parallel environments, and the `SKIP_SLOW_TESTS=1` environment variable to skip
slow tests. The `OPCACHE` environment variable can either be `yes` or `no` and
controls whether the test framework enables or disables OpCache.

Licensing
---------

Xdebug is released under `The Xdebug License
<https://github.com/xdebug/xdebug/blob/master/LICENSE>`_, which is based on
`The PHP License <https://github.com/php/php-src/blob/master/LICENSE>`_. It is
an Open Source license (though not explicitly endorsed by the Open Source
Initiative).

Further Reading
---------------

Xdebug has extensive documentation on its `website <https://xdebug.org/docs>`_.
There are over a hundred settings and many functions documented. Please have a
look through the wealth of information that Xdebug can provide to make your
every day development with PHP easier.

Support
-------

For questions regarding Xdebug, please use `StackOverflow
<https://stackoverflow.com/questions/tagged/xdebug>`_, and tag your question
with `xdebug`.

You can also find ad-hoc and sporadic support on IRC: ``freenode/#xdebug``.
You can do that with your favourite client, or by using their `webchat
<http://webchat.freenode.net/?channels=#xdebug>`_.

If you think that you encountered a bug, please file a detailed bug report
at https://bugs.xdebug.org. You are required to create an account, this is
so that you can be contacted for additional information and to keep out
spam.


Derick Rethans — derick@xdebug.org
