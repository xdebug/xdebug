--TEST--
Test for bug #701: Functions as array indexes.
--INI--
xdebug.default_enable=1
xdebug.profiler_enable=0
xdebug.auto_trace=0
xdebug.trace_format=0
xdebug.collect_vars=1
xdebug.collect_params=4
xdebug.collect_return=1
xdebug.collect_assignments=1
--FILE--
<?php
$tf = xdebug_start_trace('/tmp/'. uniqid('xdt', TRUE));

class EE_Functions {
    var $action_ids = array();
        function fetch_action_id($class, $method)
    {
        if ($class == '' OR $method == '')
        {
            return FALSE;
        }

        $this->action_ids[ucfirst($class)][$method] = $method;
        
        return 'AID:'.ucfirst($class).':'.$method;
    }
}
$obj = new EE_Functions;
echo $obj->fetch_action_id("class", "method"), "\n";

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
AID:Class:method
TRACE START [%d-%d-%d %d:%d:%d]
                         => $tf = '/tmp/%s.%s.xt' %sbug00701.php:2
                         => $obj = class EE_Functions { public $action_ids = array () } %sbug00701.php:18
%w%f %w%d     -> EE_Functions->fetch_action_id($class = 'class', $method = 'method') %sbug00701.php:19
%w%f %w%d       -> ucfirst('class') %sbug00701.php:13
                             >=> 'Class'
                           => $this->action_ids['Class']['method'] = 'method' %sbug00701.php:13
%w%f %w%d       -> ucfirst('class') %sbug00701.php:15
                             >=> 'Class'
                           >=> 'AID:Class:method'
%w%f %w%d     -> xdebug_stop_trace() %sbug00701.php:21
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
