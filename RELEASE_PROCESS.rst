
#. Mantis: Create new version if needed, and move "Fixed in version" from -dev
   to release.
#. For first release in minor version (i.e. 2.6.x), merge package.xml from old
   bug fix branch into master and commit::

       git diff HEAD..xdebug_2_5 package.xml | patch -p1

#. Update template.rc and php_xdebug.h with new version number. Use upper
   case "RC".
#. Move existing release entry down in package.xml
#. Create a new release entry in package.xml, use upper case "RC".
#. Reword package.xml so that it all makes sense!
#. Run "pecl package"
#. Install new package with ``pecl install xdebug-*.tgz``
#. Commit template.rc, php_xdebug.h, package.xml, and RELEASE_PROCESS.rst with
   text: ``Go with 2.7.0RC1`` (use upper case 'RC').
#. Tag package with ``pecl tag <version number>`` (use upper case "RC").
#. ``git push && git push --tags`` (this kicks off AppVeyor builds too)
#. Update www.xdebug.org/html/updates.php
#. Update www.xdebug.org/html/include/phpinfo_scanner.php
#. Update www.xdebug.org/html/docs/include/basic.php
#. Write news item in www.xdebug.org/html/news
#. Add files from AppVeyor and source to www.xdebug.org.html/files
#. Add the downloads, DDLs, and news file to git and commit with "Go with
   2.7.0RC1"
#. In Mantis, "release" the version, and make sure there is a new one.


