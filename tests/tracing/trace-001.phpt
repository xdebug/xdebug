--TEST--
Trace test with fibonacci numbers (format=1)
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.trace_format=1
--FILE--
<?php
require_once 'capture-trace.inc';
function fibonacci_cache ($n)
{
	if (isset ($GLOBALS['fcache'][$n])) {
		return $GLOBALS['fcache'][$n];
	}

	if ($n == 0) {
		return 0;
	} else if ($n == 1) {
		return 1;
	} else if ($n == 2) {
		return 1;
	} else {
		$t = fibonacci_cache($n - 1) + fibonacci_cache($n - 2);
		$GLOBALS['fcache'][$n] = $t;
		return $t;
	}
}

fibonacci_cache(50);
xdebug_stop_trace();
?>
--EXPECTF--
Version: %d.%s
File format: %d
TRACE START [%d-%d-%d %d:%d:%d.%d]
2	%d	1	%f	%d
2	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	22	1	50
3	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	49
4	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	48
5	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	47
6	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	46
7	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	45
8	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	44
9	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	43
10	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	42
11	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	41
12	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	40
13	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	39
14	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	38
15	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	37
16	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	36
17	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	35
18	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	34
19	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	33
20	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	32
21	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	31
22	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	30
23	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	29
24	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	28
25	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	27
26	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	26
27	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	25
28	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	24
29	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	23
30	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	22
31	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	21
32	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	20
33	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	19
34	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	18
35	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	17
36	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	16
37	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	15
38	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	14
39	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	13
40	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	12
41	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	11
42	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	10
43	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	9
44	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	8
45	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	7
46	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	6
47	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	5
48	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	4
49	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	3
50	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	2
50	%d	1	%f	%d
50	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	1
50	%d	1	%f	%d
49	%d	1	%f	%d
49	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	2
49	%d	1	%f	%d
48	%d	1	%f	%d
48	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	3
48	%d	1	%f	%d
47	%d	1	%f	%d
47	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	4
47	%d	1	%f	%d
46	%d	1	%f	%d
46	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	5
46	%d	1	%f	%d
45	%d	1	%f	%d
45	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	6
45	%d	1	%f	%d
44	%d	1	%f	%d
44	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	7
44	%d	1	%f	%d
43	%d	1	%f	%d
43	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	8
43	%d	1	%f	%d
42	%d	1	%f	%d
42	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	9
42	%d	1	%f	%d
41	%d	1	%f	%d
41	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	10
41	%d	1	%f	%d
40	%d	1	%f	%d
40	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	11
40	%d	1	%f	%d
39	%d	1	%f	%d
39	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	12
39	%d	1	%f	%d
38	%d	1	%f	%d
38	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	13
38	%d	1	%f	%d
37	%d	1	%f	%d
37	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	14
37	%d	1	%f	%d
36	%d	1	%f	%d
36	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	15
36	%d	1	%f	%d
35	%d	1	%f	%d
35	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	16
35	%d	1	%f	%d
34	%d	1	%f	%d
34	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	17
34	%d	1	%f	%d
33	%d	1	%f	%d
33	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	18
33	%d	1	%f	%d
32	%d	1	%f	%d
32	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	19
32	%d	1	%f	%d
31	%d	1	%f	%d
31	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	20
31	%d	1	%f	%d
30	%d	1	%f	%d
30	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	21
30	%d	1	%f	%d
29	%d	1	%f	%d
29	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	22
29	%d	1	%f	%d
28	%d	1	%f	%d
28	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	23
28	%d	1	%f	%d
27	%d	1	%f	%d
27	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	24
27	%d	1	%f	%d
26	%d	1	%f	%d
26	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	25
26	%d	1	%f	%d
25	%d	1	%f	%d
25	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	26
25	%d	1	%f	%d
24	%d	1	%f	%d
24	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	27
24	%d	1	%f	%d
23	%d	1	%f	%d
23	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	28
23	%d	1	%f	%d
22	%d	1	%f	%d
22	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	29
22	%d	1	%f	%d
21	%d	1	%f	%d
21	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	30
21	%d	1	%f	%d
20	%d	1	%f	%d
20	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	31
20	%d	1	%f	%d
19	%d	1	%f	%d
19	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	32
19	%d	1	%f	%d
18	%d	1	%f	%d
18	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	33
18	%d	1	%f	%d
17	%d	1	%f	%d
17	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	34
17	%d	1	%f	%d
16	%d	1	%f	%d
16	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	35
16	%d	1	%f	%d
15	%d	1	%f	%d
15	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	36
15	%d	1	%f	%d
14	%d	1	%f	%d
14	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	37
14	%d	1	%f	%d
13	%d	1	%f	%d
13	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	38
13	%d	1	%f	%d
12	%d	1	%f	%d
12	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	39
12	%d	1	%f	%d
11	%d	1	%f	%d
11	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	40
11	%d	1	%f	%d
10	%d	1	%f	%d
10	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	41
10	%d	1	%f	%d
9	%d	1	%f	%d
9	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	42
9	%d	1	%f	%d
8	%d	1	%f	%d
8	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	43
8	%d	1	%f	%d
7	%d	1	%f	%d
7	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	44
7	%d	1	%f	%d
6	%d	1	%f	%d
6	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	45
6	%d	1	%f	%d
5	%d	1	%f	%d
5	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	46
5	%d	1	%f	%d
4	%d	1	%f	%d
4	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	47
4	%d	1	%f	%d
3	%d	1	%f	%d
3	%d	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	48
3	%d	1	%f	%d
2	%d	1	%f	%d
2	%d	0	%f	%d	xdebug_stop_trace	0		%strace-001.php	23	0
			%f	%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
