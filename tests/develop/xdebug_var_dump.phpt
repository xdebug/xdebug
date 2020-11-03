--TEST--
Test for correct display with xdebug_var_dump()
--INI--
xdebug.mode=develop
html_errors=1
date.timezone=Europe/Oslo
xdebug.var_display_max_children=11
xdebug.file_link_format=
--FILE--
<?php
	class TimeStuff {
		private $timestamp;
		private $user_defined;
		private $self;
		protected $tm;
		public $date;

		function __construct($ts = null)
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
--EXPECTF--
<pre class='xdebug-var-dump' dir='ltr'>
<small>%sxdebug_var_dump.php:21:</small>
<b>object</b>(<i>TimeStuff</i>)[<i>%d</i>]
  <i>private</i> 'timestamp' <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>1092515106</font>
  <i>private</i> 'user_defined' <font color='#888a85'>=&gt;</font> <small>boolean</small> <font color='#75507b'>true</font>
  <i>private</i> 'self' <font color='#888a85'>=&gt;</font> 
    <i>&amp;</i><b>object</b>(<i>TimeStuff</i>)[<i>%d</i>]
  <i>protected</i> 'tm' <font color='#888a85'>=&gt;</font> 
    <b>array</b> <i>(size=11)</i>
      'seconds' <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>6</font>
      'minutes' <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>25</font>
      'hours' <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>22</font>
      'mday' <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>14</font>
      'wday' <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>6</font>
      'mon' <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>8</font>
      'year' <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>2004</font>
      'yday' <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>226</font>
      'weekday' <font color='#888a85'>=&gt;</font> <small>string</small> <font color='#cc0000'>'Saturday'</font> <i>(length=8)</i>
      'month' <font color='#888a85'>=&gt;</font> <small>string</small> <font color='#cc0000'>'August'</font> <i>(length=6)</i>
      0 <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>1092515106</font>
  <i>public</i> 'date' <font color='#888a85'>=&gt;</font> <small>string</small> <font color='#cc0000'>'2004-08-14 22:25:06 CEST'</font> <i>(length=24)</i>
</pre>
