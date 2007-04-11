<?php

/* 
 * Online profiling dump - Written by Jani Taskinen <sniper@iki.fi> A.D. 2007
 *
 * $Id: online_profiling_prepend.php,v 1.1 2007-04-11 22:37:11 sniper Exp $
 *
 * Usage:
 *
 * You can either have this file included by using the php.ini
 * directive "auto_prepend_file" or including it in your script.
 * 
 * Passing XDEBUG_PROFILE in GET/POST enables the output.
 *
 * Example of download.php:

  <?php
   
  if (file_exists("/{$_GET['file']}"))
  {
    $filesize=filesize("/{$_GET['file']}");
    $file=basename("/{$_GET['file']}");

    header("Content-Type: application/x-kcachegrind");
    header("Content-Disposition: attachment; filename=\"cachegrind.out\"");
    header("Content-Length: {$filesize}");
    passthru("cat /tmp/xdebug/{$file}",$err);
  }

  exit;

  ?>

 * 
 */

function xdebug_profiler_shutdown()
{
  if (isset($_REQUEST['XDEBUG_PROFILE']))
  {
    $dump = '';
    $used_memory = xdebug_memory_usage();
    $sizename = array(" Bytes", " KB", " MB", " GB");
    $used_memory = round($used_memory/pow(1024, ($i = floor(log($used_memory, 1024)))), 2) . $sizename[$i];
    $elapsed_time = round(xdebug_time_index() * 1000, 3);
    $profile = xdebug_get_profiler_filename();

    if (file_exists($profile))
    {
      $view_type_url = '<a href="?XDEBUG_PROFILE">[Annotate]</a>';

      if ($_REQUEST['XDEBUG_PROFILE'] != 'short')
      {
        $view_type_url = '<a href="?XDEBUG_PROFILE=short">[No annotation]</a>';
        $dump = shell_exec("/usr/bin/callgrind_annotate --inclusive=yes --tree=both $profile");
      }
    }

    echo <<< DATA
<hr />
<pre style="border: dashed black 1px; margin: 5px; padding: 5px; background-color: #fff;">
 <b>Page generated in</b> {$elapsed_time} ms <b>Used memory:</b> {$used_memory}
 <b>Profiler dump:</b> <a href="/download.php?file={$profile}">$profile</a> {$view_type_url}
{$dump}</pre>
DATA;
  }
}

/* Only register shutdown function if profiling is enabled */
if (isset($_REQUEST['XDEBUG_PROFILE']))
{
  register_shutdown_function('xdebug_profiler_shutdown');
}
