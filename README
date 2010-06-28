XDEBUG
------

You need to compile Xdebug separately from the rest of PHP.  Note, however,
that you need access to the scripts "phpize" and "php-config".  If your
system does not have "phpize" and "php-config", you will need to compile
and install PHP from a source tarball first, as these script are
by-products of the PHP compilation and installation processes. It is
important that the source version matches the installed version as there
are slight, but important, differences between PHP versions. 

Once you have access to "phpize" and "php-config", do the following:

1. Unpack the tarball: tar -xzf xdebug-2.2.x.tgz.  Note that you do
not need to unpack the tarball inside the PHP source code tree.
Xdebug is compiled separately, all by itself, as stated above.

2. cd xdebug-2.2.x

3. Run phpize: phpize
   (or /path/to/phpize if phpize is not in your path).

4. ./configure --enable-xdebug (or: ../configure --enable-xdebug
   --with-php-config=/path/to/php-config if php-config is not in your
   path)

5. Run: make

6. cp modules/xdebug.so /to/wherever/you/want/it

7. add the following line to php.ini:
   zend_extension="/wherever/you/put/it/xdebug.so"

8. Restart your webserver.

9. Write a PHP page that calls "phpinfo();" Load it in a browser and
   look for the info on the xdebug module.  If you see it, you have been
   successful!


SUPPORT
-------

If you think that you encountered a bug, please file a detailed bugreport
at http://bugs.xdebug.org . You are required to create an account, this is
so that you can be contacted for additional information and to keep out
spam.


Derick Rethans
derick@xdebug.org
