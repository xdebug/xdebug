
#. Mantis: Create new version if needed, and move "Fixed in version" from -dev
   to release: https://bugs.xdebug.org/manage_proj_edit_page.php?project_id=1
#. Make sure both the master and release branch (i.e. xdebug_3_3) are fully
   synced and merged.
#. Update composer.json, package.xml, config.m4, and config.w32 to reflect
   supported range of PHP versions.
#. For first release in minor version (i.e. 3.5.0alpha1), merge package.xml
   from old bug fix branch into master and new branch and commit::

       git diff HEAD..xdebug_3_4 package.xml | patch -p1

#. Run: ``php .build.scripts/make-release.php <version>``
#. Move existing release entry down in ``package.xml``
#. Include new snippet in ``/tmp/package.xml``
#. Run commands from output
#. Create a release from the tag on GitHub
#. Update www.xdebug.org ``views/home/updates.php`` with snippet
#. Update www.xdebug.org ``src/XdebugVersion.php`` with snippet
#. Update pre-generated www.xdebug.org ``data/news/...`` file
#. Mantis: "release" the version, and make sure there is a new one.

Wait until GitHub Actions has created the artefacts.

#. Write Patreon post taking the rendered news article as starting point
#. Write GitHub release notes taking the rendered news article as starting
   point, and save with "Publish release"

#. In the release branch, update template.rc and php_xdebug.h to the new
   version
#. Commit template.rc and php_xdebug.h with ``Back to -dev``
#. Check out master branch, and run: ``git merge --strategy=ours xdebug_3_5``
#. ``git push origin master xdebug_3_5``
#. Add files from GHA and source to www.xdebug.org html/files
#. Create sha256 files for the new releases::

    for i in *3.5.*{tgz,dll}; do \
      echo $i; sha256sum $i | sed 's/\ .*//' > $i.sha256.txt; \
    done

#. Add the downloads, DDLs, SHA256 files, and news file to git and commit with
   "Go with 3.5.1"
#. Upload the source package to PECL
