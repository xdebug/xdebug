进度
------------
* 基于 xdebug2.7(https://github.com/xdebug/xdebug), 目前修改基本完毕了
* QQ 群: 897623858

注意事项
------------
* 为了避免 swoole 的检测 xdebug 警告, 扩展注册的名称是 sdebug, 如果想使用 Phpunit CodeCoverage , 需要手动把检测 xdebug 的判断修改成 sdebug
* 单步调试: 如果php不是7.3的, 建议使用 sdebug_2_6(https://github.com/mabu233/sdebug/tree/sdebug), sdebug_2_7 可能需要与phpstorm2019搭配使用

Clone
-----

You can clone the Sdebug source directory with::

   git clone https://github.com/mabu233/sdebug.git

Then move into this new directory::

	cd sdebug

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
		with Sdebug v2.x.x-dev, Copyright (c) 2002-2017, by Derick Rethans
