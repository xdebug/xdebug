Xdebug
======

.. image:: https://travis-ci.org/xdebug/xdebug.svg?branch=master
.. image:: https://ci.appveyor.com/api/projects/status/glp9xfsmt1p25nkn?svg=true
.. image:: https://circleci.com/gh/xdebug/xdebug/tree/master.svg?style=svg

These are instructions for installing Xdebug from a Git checkout. Please refer
to https://xdebug.org/support.php for support.

Introduction
------------

You need to compile Xdebug separately from the rest of PHP. You need to have
access to the scripts ``phpize`` and ``php-config``.  If your system does not
have ``phpize`` and ``php-config``, you will need to compile and install PHP
from a source tarball first, or install a ``php-dev`` package if your
distribution provides one. These scripts are by-products of the PHP
compilation and installation processes and are needed to compile external
extensions. It is important that the source version matches the installed
version as there are slight, but important, differences between PHP versions. 

Clone
-----

You can clone the Xdebug source directory with::

   git clone https://github.com/xdebug/xdebug.git

Then move into this new directory::

	cd xdebug

Although it is recommended to run the latest version from the **master**
branch, older versions are available through tags. For example to checkout the
2.5.5 release, you can switch to it with ``git checkout XDEBUG_2_5_5``.

Compile
-------

If PHP is installed in a normal, and uncomplicated way, with default locations
and configuration, all you will need to do is to run the following script::

	./rebuild.sh

This will run ``phpize``, ``./configure``, ``make clean``, ``make`` and ``make
install``.

The long winded way of installation is:

#. Run phpize: ``phpize``
   (or ``/path/to/phpize`` if phpize is not in your path).

#. ``./configure --enable-xdebug`` (or: ``../configure --enable-xdebug
   --with-php-config=/path/to/php-config`` if ``php-config`` is not in your
   path)

#. Run: ``make clean``

#. Run: ``make``

#. Run: ``make install``

#. Add the following line to ``php.ini`` (which you can find by running ``php
   --ini``, or look at ``phpinfo()`` output): ``zend_extension="xdebug.so"``.

   Please note, that sometimes the ``php.ini`` file is different for the
   command line and for your web server. Make sure you pick the right one.

#. Unless you exclusively use the command line with PHP, restart your webserver.

#. Write a PHP page that calls ``phpinfo();``. Load it in a browser and
   look for the info on the Xdebug module.  If you see it, you have been
   successful! Alternatively, you can run ``php -v`` on the command line to
   see that Xdebug is loaded::

	$ php -v
	PHP 7.2.0RC6 (cli) (built: Nov 23 2017 10:30:56) ( NTS DEBUG )
	Copyright (c) 1997-2017 The PHP Group
	Zend Engine v3.2.0-dev, Copyright (c) 1998-2017 Zend Technologies
		with Xdebug v2.6.0-dev, Copyright (c) 2002-2017, by Derick Rethans

Support
-------

For questions regarding compile issues, please write to the **xdebug-general**
email list which you can find at https://xdebug.org/support.php#list

You can also find support on IRC: ``freenode/#xdebug``. You can do that with
your favourite client, or by using their webchat_.

.. _webchat: http://webchat.freenode.net/?channels=#xdebug

If you think that you encountered a bug, please file a detailed bug report
at https://bugs.xdebug.org. You are required to create an account, this is
so that you can be contacted for additional information and to keep out
spam.


Derick Rethans — derick@xdebug.org
