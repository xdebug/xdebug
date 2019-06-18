
#. Mantis: Create new version if needed, and move "Fixed in version" from -dev
   to release.
#. Make sure both the master and release branch (i.e. xdebug_2_7) are fully
   synced and merged.
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
#. ``git push origin master xdebug_2_7 && git push --tags``
#. Disable extra AppVeyor build (the one without the tag)
#. Update www.xdebug.org/html/updates.php
#. Update www.xdebug.org/html/include/phpinfo_scanner.php
#. Update www.xdebug.org/html/docs/include/basic.php
#. Write news item in www.xdebug.org/html/news
#. Upload the source package to PECL
#. Write Patreon post taking the rendered news article as starting point
#. Add files from AppVeyor and source to www.xdebug.org.html/files
#. Add the downloads, DDLs, and news file to git and commit with "Go with
   2.7.0RC1"
#. Mantis: "release" the version, and make sure there is a new one.
#. In the release branch, update template.rc and php_xdebug.h to the new
   version
#. Commit template.rc and php_xdebug.h with "Back to -dev"
#. Check out master branch, and run: git merge --strategy=ours xdebug_2_7
#. ``git push origin master xdebug_2_7``
