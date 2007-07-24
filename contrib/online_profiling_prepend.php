<?php

/* 
 * Online profiling dump - Written by Jani Taskinen <sniper@iki.fi> A.D. 2007
 *
 * $Id: online_profiling_prepend.php,v 1.3 2007-07-24 10:34:15 sniper Exp $
 *
 * Usage:
 *
 * You can either have this file included by using the php.ini
 * directive "auto_prepend_file" or including it in your script.
 * 
 * Passing XDEBUG_PROFILE in GET/POST/COOKIE enables the output.
 *
 * Example of download.php:

  <?php
   
  if (file_exists("/{$_GET['file']}"))
  {
    $filesize=filesize("/{$_GET['file']}");
    $file=basename("/{$_GET['file']}");

// If you want to have always same filename uncomment this:
//  header("Content-Disposition: attachment; filename=\"cachegrind.out\"");
    header("Content-Type: application/x-kcachegrind");
    header("Content-Length: {$filesize}");
    passthru("cat /tmp/xdebug/{$file}",$err);
  }

  exit;

  ?>

 *
 * Example of php.ini options:
 *

 [xdebug]
 zend_extension_debug = ${extension_dir}"/xdebug.so"
 xdebug.profiler_enable = Off
 xdebug.profiler_enable_trigger = On
 xdebug.profiler_output_dir = /tmp/xdebug
 xdebug.profiler_append = On
 xdebug.profiler_aggregate = Off
 xdebug.profiler_output_name = %H.%S ; <HTTP_HOST>.<SESSION_ID>
 xdebug.extended_info = 1
 
 * 
 */

function xdebug_profiler_shutdown_cb()
{
  $is_xmlhttprequest = (isset($_SERVER['HTTP_X_REQUESTED_WITH']) && strtolower($_SERVER['HTTP_X_REQUESTED_WITH']) == 'xmlhttprequest'); 

  if (isset($_REQUEST['XDEBUG_PROFILE']))
  {
    $used_memory = xdebug_memory_usage();
    $sizename = array(" Bytes", " KB", " MB", " GB");
    $used_memory = round($used_memory / pow(1024, ($i = floor(log($used_memory, 1024)))), 2) . $sizename[$i];
    $elapsed_time = round(xdebug_time_index() * 1000, 3);
    $profile = xdebug_get_profiler_filename();
    $profile_id = md5($profile);

    /* Show result box */
    if (!$is_xmlhttprequest) // FIXME: How to provide profiler links without breaking possible json?
    {
      if ($profile === false)
      {
        $path = ini_get('xdebug.profiler_output_dir');

        if ($path != '')
        {
          $reason = is_dir($path) ? 'Directory is not writeable' : (file_exists($path) ? "'{$path}' is not directory" : "'$path' does not exist");
          $output = sprintf('Error: Could not create profile dump in %s<br />(Reason: %s)', $path, $reason);
        }
        else
        {
          $output = 'Error: xdebug.profiler_output_dir is not set';
        }
      }
      else
      {
        $output = "
<b>Page generated in</b> {$elapsed_time} ms <b>Used memory:</b> {$used_memory}
<b>Profiler dump:</b> <a href='/download.php?file={$profile}'>{$profile}</a>
";

        if ($_REQUEST['XDEBUG_PROFILE'] == 'long')
        {
          $output.= shell_exec("/usr/bin/callgrind_annotate --inclusive=yes --tree=both $profile");
        }
      }

      echo <<< DATA
<div style="position: absolute; top: 0; z-index: 5000; border: dashed black 1px; background-color: #fff;" id="xdebug_profile_{$profile_id}">
 <a href="#" style="font-size: 11px;" onclick="javascript: document.getElementById('xdebug_profile_{$profile_id}').style.display = 'none'; return false;">[close]</a>
 <pre style="padding: 5px;">{$output}</pre>
 <a href="#" style="font-size: 11px;" onclick="javascript: document.getElementById('xdebug_profile_{$profile_id}').style.display = 'none'; return false;">[close]</a>
</div>
DATA;
    }
  }

  /* Output box with toggles to enable/disable profiler and annotation */
  if (!$is_xmlhttprequest)
  {
    $profiler = isset($_REQUEST['XDEBUG_PROFILE']) ? 
    array
    (
      'enabled' => 1,
      'checked' => 'checked="checked"',
      'display' => 'inline',
    ) : 
    array 
    (
      'enabled' => 0,
      'checked' => '',
      'display' => 'none',
    );

    $profiler['checked_annotate'] = isset($_REQUEST['XDEBUG_PROFILE']) && $_REQUEST['XDEBUG_PROFILE'] == 'long' ? 'checked="checked"' : '';

    echo <<< DATA
<!-- XDEBUG Dynamic Profiler -->
<script type="text/javascript">
<!--
var xdebug_Profiler = {$profiler['enabled']};
function xdebug_setCookie(value)
{
  if (value == '')
    document.cookie = "XDEBUG_PROFILE=; path=/; expires=Thu, 01-Jan-1970 00:00:01 GMT";
  else
    document.cookie = "XDEBUG_PROFILE=" + value + "; path=/; expires=Fri, 01-Jan-2038 00:00:01 GMT";
}
function xdebug_toggleProfiler(output)
{
  var annotate = document.getElementById('xdebug_profiler_annotate');

  if (xdebug_Profiler) {
    xdebug_setCookie('');
    xdebug_Profiler = 0;
    annotate.style.display = 'none';
  } else {
    xdebug_setCookie(output);
    xdebug_Profiler = 1;
    annotate.style.display = 'inline';
  }
  return xdebug_Profiler;
}
// -->
</script>
<div style="padding: 5px; border: dashed black 1px; background-color: #fff; z-index: 1000; position: absolute; top: 0px; right: 5px; " id="xdebug_profile_enable_cookie">
 <label for="xdebug_toggler" style="vertical-align: top">Toggle Profiler</label>
 <input id="xdebug_toggler" type="checkbox" onclick="this.checked = xdebug_toggleProfiler(this.value);" value="short" {$profiler['checked']} />
 <div id="xdebug_profiler_annotate" style="display: {$profiler['display']}">
  <label for="xdebug_annotate" style="vertical-align: top">Annotate</label>
  <input id="xdebug_annotate" type="checkbox" onclick="xdebug_setCookie((this.checked)?this.value:'short');" value="long" {$profiler['checked_annotate']} />
 </div>
</div>
DATA;
  }
}

/* Register shutdown function */
if (PHP_SAPI != 'cli')
{
  register_shutdown_function('xdebug_profiler_shutdown_cb');
}
