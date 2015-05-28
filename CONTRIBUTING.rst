Contributing
============

Xdebug is hosted on Github. The source code can be browsed github and can be
checked out with::

  git clone git://github.com/xdebug/xdebug.git

If you think you want to fix a bug or work on a new feature, then you need to
follow the instructions below.

Initial Set-up
--------------

Fork Xdebug on github.

- Clone the repository::

    git clone git@github.com:{your username}/xdebug.git

  for example::

    git clone git@github.com:jamesbond/xdebug.git

- Change into the ``xdebug`` repository::

    cd xdebug

- Make sure to set your git name and email::

    git config --get user.name
    git config --get user.email

  If they are not correct, set them to the correct value::

    git config user.name {your name}
    git config user.email {your email}

  for example::

    git config user.name "Derick Rethans"
    git config user.email "derick@xdebug.org"

- Add the original repository as remote (after removing the old one)::

    git remote add upstream git://github.com/xdebug/xdebug.git
    git fetch upstream

- Add a tracking branch for ``xdebug 2.3``::

    git checkout --track origin/xdebug_2_3

Keeping up-to-date
------------------

- Change into the ``xdebug`` repository (if you haven't done yet)::

    cd xdebug

  Run::

    git checkout master
    git fetch upstream
    git rebase upstream/master

  Run::

    git checkout xdebug_2_3
    git fetch upstream
    git rebase upstream/xdebug_2_3

Working on a bug fix
--------------------

The steps for this are the same as for working on new features except that you
make a branch of ``xdebug_2_3`` instead of ``master``.

- First of all, make sure you're up-to-date.
- Checkout the xdebug_2_3 branch::

    git checkout xdebug_2_3

- Create a feature branch::

    git checkout -b issue{issue number}

  for example::

    git checkout -b issue681

  If there is no bug report yet, then you need to create one. If you want, you
  can add a description of the feature after the issue681 part, for example:
  ``issue623-debug-static-properties``.

- Work on the code, and add one or more tests in the tests directory, with as
  name ``tests/bug00{issue number}.phpt``, for example:
  ``tests/bug00623.phpt``.

- Commit it to your local repository::

    git commit ...

- Repeat the previous two steps as long as you want.

- Bring things up-to-date with the original repository, especially important
  if it took some time since you branched::

    git fetch upstream && git rebase upstream/xdebug_2_3

- Push your changes to your remote repository::

    git push origin {issue number}:{issue number}

  for example::

    git push origin issue681:issue681

- Once you're satisfied, generate a pull request, by navigating to your
  repository (https://github.com/{username}/xdebug), select the branch you
  just created (``issue681``), and then select the "Pull Request" button in
  the upper right. Select the user xdebug as the recipient.

- Alternatively you can nagivate to
  https://github.com/{username}/xdebug/pull/new/issue{issue number}.

Working on a new feature
------------------------

The steps for this are the same as for fixing bugs except that you make a
branch of master instead of xdebug_2_3.

- First of all, make sure you're up-to-date.
- Checkout the master branch::

    git checkout master

- Create a feature branch::

    git checkout -b issue{issue number}

  for example::

    git checkout -b issue681

  If there is no bug report yet, then you need to create one. If you want, you
  can add a description of the feature after the issue681 part, for example:
  ``issue623-debug-static-properties``.

- Work on the code, and add one or more tests in the tests directory, with as
  name ``tests/bug00{issue number}.phpt``, for example:
  ``tests/bug00623.phpt``.

- Commit it to your local repository::

    git commit ...

- Repeat the previous two steps as long as you want.

- Bring things up-to-date with the original repository, especially important
  if it took some time since you branched::
  
    git fetch xdebug
    git rebase xdebug/master

- Push your changes to your remote repository::

    git push origin {issue number}:{issue number}

  for example::

    git push origin issue681:issue681

- Once you're satisfied, generate a pull request, by navigating to your
  repository (https://github.com/{username}/xdebug), select the branch you
  just created (``issue681``), and then select the "Pull Request" button in the
  upper right. Select the user xdebug as the recipient.

  Alternatively you can nagivate to
  https://github.com/{username}/xdebug/pull/new/issue{issue number}.

