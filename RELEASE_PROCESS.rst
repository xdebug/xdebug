
#. Mantis: Create new version if needed, and move "Fixed in version" from -dev
   to release: https://bugs.xdebug.org/manage_proj_edit_page.php?project_id=1
#. Make sure both the master and release branch (i.e. xdebug_2_8) are fully
   synced and merged.
#. For first release in minor version (i.e. 2.8.x), merge package.xml from old
   bug fix branch into master and commit::

       git diff HEAD..xdebug_2_7 package.xml | patch -p1

#. Update template.rc and php_xdebug.h with new version number. Use upper
   case "RC".
#. Move existing release entry down in package.xml
#. Create a new release entry in package.xml, use upper case "RC".
#. Reword package.xml so that it all makes sense!
#. Rebuild from source with: ``~/bin/rebuild.sh``
#. Run xdebug.ini update script from xdebug.org repository:
   ``php html/docs/convert.php  > ~/dev/php/xdebug-xdebug/xdebug.ini``
#. Run "pecl package"
#. Install new package with ``pecl install xdebug-*.tgz``
#. Commit template.rc, php_xdebug.h, package.xml, xdebug.ini, and
   RELEASE_PROCESS.rst with text: ``Go with 2.8.0RC1`` (use upper case 'RC').
#. Tag package with ``git tag -u ${GPGKEY} -m <version number> <version number>``
   (use upper case "RC").
#. ``git push origin master xdebug_2_8 && git push --tags``
#. Disable extra AppVeyor build (the one without the tag)
#. Update www.xdebug.org views/home/updates.php
#. Update www.xdebug.org src/XdebugVersion.php
#. Update www.xdebug.org src/Controller/DocsController.php
#. Write news item in www.xdebug.org data/news
#. Write Patreon post taking the rendered news article as starting point
#. Wait until AppVeyor is ready
#. Upload the source package to PECL
#. Add files from AppVeyor and source to www.xdebug.org html/files
#. Create sha256 files for the new releases::

   for i in *2.9.2*{tgz,dll}; do \
     echo $i; sha256sum $i | sed 's/\ .*//' > $i.sha256.txt; \
   done

#. Add the downloads, DDLs, SHA256 files, and news file to git and commit with
   "Go with 2.8.0alpha1"
#. Mantis: "release" the version, and make sure there is a new one.
#. In the release branch, update template.rc and php_xdebug.h to the new
   version
#. Commit template.rc and php_xdebug.h with "Back to -dev"
#. Check out master branch, and run: git merge --strategy=ours xdebug_2_8
#. ``git push origin master xdebug_2_8``
