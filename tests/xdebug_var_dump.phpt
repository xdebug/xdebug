--TEST--
Test for correct display with xdebug_var_dump() (ZE2)
--SKIPIF--
<?php if(version_compare(zend_version(), "2.0.0-dev", '<')) echo "skip Zend Engine 2 needed\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.profiler_enable=0
html_errors=1
date.timezone=Europe/Oslo
--FILE--
<?php
	class TimeStuff {
		private $timestamp;
		private $user_defined;
		private $self;
		protected $tm;
		public $date;

		function TimeStuff($ts = null)
		{
			$this->self = &$this;
			$this->timestamp = $ts === null ? time() : $ts;
			$this->user_defined = ($ts !== null);
			$this->date = date("Y-m-d H:i:s T", $this->timestamp);
			$this->tm = getdate($this->timestamp);
		}
	}

	$ts1 = new TimeStuff(1092515106);

	var_dump($ts1);
?>
--EXPECT--
<pre>
<b>object</b>(<i>TimeStuff</i>)[<i>1</i>]
  <i>private</i> 'timestamp' <font color='#777777'>=&gt;</font> <font color='#00bb00'>1092515106</font>
  <i>private</i> 'user_defined' <font color='#777777'>=&gt;</font> <font color='#0000ff'>true</font>
  <i>private</i> 'self' <font color='#777777'>=&gt;</font> 
    <i>&</i><b>object</b>(<i>TimeStuff</i>)[<i>1</i>]
  <i>protected</i> 'tm' <font color='#777777'>=&gt;</font> 
    <b>array</b>
      'seconds' <font color='#777777'>=&gt;</font> <font color='#00bb00'>6</font>
      'minutes' <font color='#777777'>=&gt;</font> <font color='#00bb00'>25</font>
      'hours' <font color='#777777'>=&gt;</font> <font color='#00bb00'>22</font>
      'mday' <font color='#777777'>=&gt;</font> <font color='#00bb00'>14</font>
      'wday' <font color='#777777'>=&gt;</font> <font color='#00bb00'>6</font>
      'mon' <font color='#777777'>=&gt;</font> <font color='#00bb00'>8</font>
      'year' <font color='#777777'>=&gt;</font> <font color='#00bb00'>2004</font>
      'yday' <font color='#777777'>=&gt;</font> <font color='#00bb00'>226</font>
      'weekday' <font color='#777777'>=&gt;</font> <font color='#bb00bb'>'Saturday'</font> <i>(length=8)</i>
      'month' <font color='#777777'>=&gt;</font> <font color='#bb00bb'>'August'</font> <i>(length=6)</i>
      0 <font color='#777777'>=&gt;</font> <font color='#00bb00'>1092515106</font>
  <i>public</i> 'date' <font color='#777777'>=&gt;</font> <font color='#bb00bb'>'2004-08-14 22:25:06 CEST'</font> <i>(length=24)</i>
</pre>
