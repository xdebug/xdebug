<?php
ignore_user_abort(true);

$handler = static function () {
    $n = $_GET['n'] ?? '?';
    $pid = getmypid();
    echo "req=$n pid=$pid\n";
};

while (frankenphp_handle_request($handler)) {
}
