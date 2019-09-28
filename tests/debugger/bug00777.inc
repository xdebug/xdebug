<?php
   if (!function_exists('mysqli_init') && !extension_loaded('mysqli')) {
      echo('MySQLi extension for PHP is not installed. ');
   } else {
      $connection = mysqli_init();
      var_dump($connection);
   }
?>
