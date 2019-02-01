
#. Mantis: Create new version if needed, and move "Fixed in version" from -dev
   to release.
#. For first release in minor version (i.e. 2.6.x), merge package.xml from old
   bug fix branch into master and commit::

       git diff HEAD..xdebug_2_5 package.xml | patch -p1

#. Update template.rc and php_xdebug.h with new version number. Use lower
   case "rc".
#. Move existing release entry down in package.xml
#. Create a new release entry in package.xml, use upper case "RC".
#. Reword package.xml so that it all makes sense!
#. Run "pecl package"
#. Install new package with ``pecl install xdebug-*.tgz``
#. Commit template.rs php_xdebug.h and RELEASE_PROCESS.rst with text:
   ``Go with 2.7.0rc1`` (use lower case 'rc').
#. Tag package with ``pecl tag <version number>``
#. Write news item in www.xdebug.org/html/news
#. Update www.xdebug.org/html/updates.php
#. Update www.xdebug.org/html/include/phpinfo_scanner.php
#. Update www.xdebug.org/html/docs/include/basic.php



