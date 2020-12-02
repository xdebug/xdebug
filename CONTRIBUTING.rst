Contributing
============

Xdebug is hosted on Github. The source code can be browsed there and can be
checked out with::

  git clone git://github.com/xdebug/xdebug.git

If you think you want to fix a bug or work on a new feature, then you need to
follow the instructions below. Please reach out first to discuss your
suggested changes as well.

Initial Set-up
--------------

- Fork Xdebug on GitHub.
- Make sure you have configured your Author Name and Author Email with GIT.
  Xdebug doesn't accept contributions from accounts with unnatural names.
- Clone the repository::

    git clone git@github.com:{your username}/xdebug.git

- Change into the ``xdebug`` repository::

    cd xdebug

- Add the original repository as ``upstream`` remote::

    git remote add upstream git://github.com/xdebug/xdebug.git
    git fetch upstream

- Add a tracking branch for ``xdebug 3.0``::

    git checkout --track origin/xdebug_3_0

Branches
--------

There are two branches in operation:

``master``
	This is were all new feature Pull Requests should be targeted at
``xdebug_3_0``
	This is were all bug fix Pull Requests should be targeted at. The
	maintainer will add them to ``master`` too when merging the Pull Request.

Working on a Pull Request
-------------------------

- Make sure that your ``master`` and ``xdebug_3_0`` branches are up to date
  with the ``upstream`` repository.
- Create an issue in the `issue tracker <https://bugs.xdebug.org>`_ (if none
  exists yet).
- Switch to the right target branch (``master`` for features, ``xdebug_3_0``
  for bug fixes).
- Create a feature branch::

    git checkout -b issue{issue number}-{description}

  For example::

    git checkout -b issue1893-crash-with-fiber

- For a bug fix, write one or more test cases to verify that the problem
  currently exists, and also to define what the output should be. Xdebug uses
  PHP's `phpt tests <https://qa.php.net/write-test.php>`_. The ``README.rst``
  file contains information on how to run the tests.

  Each of Xdebug's modes has a specific directory where to place tests. For
  example, for code coverage that is ``tests/coverage``. Test case names
  should follow the following pattern::

  	tests/{feature-group}bug0{issue-number}.phpt

  If you need more than one test, append ``-001`` after the issue number.

  Pull Requests without tests won't be accepted.

- Fix and/or write the code.

- Before you submit a PR, make sure each commit is a single logical unit. The
  main commit that implements the issue, should have as commit message ``Fixed
  bug #1893: `` followed by the Summary of the issue in the issue tracker. The
  message should state what the change was about. For example::

  	Fixed bug #1893: Crash with ext-fiber with xdebug.mode=coverage

- Before you submit a PR, make sure to rebase first on the branch that you
  will be targeting, for example to rebase against the current bug fix
  branch::

    git fetch upstream && git rebase upstream/xdebug_3_0

- Push your changes to your remote repository::

    git push origin {branch-name}

  For example::

    git push origin issue1893-crash-with-fiber

- Once you're satisfied, generate a pull request. Make sure that the title is
  in one line, it's fine if it's a few characters larger than what GitHub
  likes. Do not let the title spill over into the description with ``...```.

  In the description, explain what you changed, and why, and how your solution
  is the right one. Feel free to include questions, and pointers to specific
  things that need close review.
