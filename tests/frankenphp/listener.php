<?php
/* Minimal DBGp client used by run-tests.sh: accepts connections from Xdebug,
 * logs each session, answers "run", then completes the stopping handshake.
 *
 * Usage: php listener.php [port] [bind-address]
 */
error_reporting(E_ALL & ~E_WARNING);

function read_packet($conn): string
{
    $data = '';
    while (substr_count($data, "\0") < 2) {
        $chunk = fread($conn, 8192);
        if ($chunk === '' || $chunk === false) {
            break;
        }
        $data .= $chunk;
    }
    return explode("\0", $data)[1] ?? '';
}

$port = $argv[1] ?? '9003';
$bind = $argv[2] ?? '127.0.0.1';
$srv = stream_socket_server("tcp://$bind:$port", $errno, $err);
if (!$srv) {
    fwrite(STDERR, "listen failed: $err\n");
    exit(1);
}
echo "LISTENING\n";

$n = 0;
while (true) {
    $r = [$srv];
    $w = $e = null;
    if (stream_select($r, $w, $e, 60) < 1) {
        break;
    }
    $conn = stream_socket_accept($srv, 5);
    if (!$conn) {
        continue;
    }
    $n++;
    stream_set_timeout($conn, 10);

    $xml = read_packet($conn);
    preg_match('/fileuri="([^"]*)"/', $xml, $m);
    echo 'CONN#' . $n . ' init fileuri=' . ($m[1] ?? '?') . "\n";

    fwrite($conn, "run -i 1\0");
    $xml = read_packet($conn);
    if (strpos($xml, 'stopping') !== false) {
        echo "CONN#$n stopping\n";
        fwrite($conn, "stop -i 2\0");
        read_packet($conn);
        echo "CONN#$n stopped\n";
    } else {
        echo "CONN#$n unexpected: " . substr($xml, 0, 120) . "\n";
    }
    fclose($conn);
}
