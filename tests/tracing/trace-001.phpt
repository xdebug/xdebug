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
2	1	1	%f	%d
2	7	0	%f	%d	fibonacci_cache	1		%strace-001.php	22	1	50
3	8	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	49
4	9	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	48
5	10	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	47
6	11	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	46
7	12	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	45
8	13	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	44
9	14	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	43
10	15	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	42
11	16	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	41
12	17	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	40
13	18	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	39
14	19	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	38
15	20	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	37
16	21	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	36
17	22	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	35
18	23	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	34
19	24	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	33
20	25	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	32
21	26	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	31
22	27	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	30
23	28	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	29
24	29	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	28
25	30	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	27
26	31	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	26
27	32	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	25
28	33	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	24
29	34	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	23
30	35	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	22
31	36	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	21
32	37	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	20
33	38	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	19
34	39	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	18
35	40	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	17
36	41	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	16
37	42	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	15
38	43	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	14
39	44	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	13
40	45	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	12
41	46	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	11
42	47	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	10
43	48	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	9
44	49	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	8
45	50	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	7
46	51	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	6
47	52	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	5
48	53	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	4
49	54	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	3
50	55	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	2
50	55	1	%f	%d
50	56	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	1
50	56	1	%f	%d
49	54	1	%f	%d
49	57	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	2
49	57	1	%f	%d
48	53	1	%f	%d
48	58	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	3
48	58	1	%f	%d
47	52	1	%f	%d
47	59	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	4
47	59	1	%f	%d
46	51	1	%f	%d
46	60	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	5
46	60	1	%f	%d
45	50	1	%f	%d
45	61	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	6
45	61	1	%f	%d
44	49	1	%f	%d
44	62	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	7
44	62	1	%f	%d
43	48	1	%f	%d
43	63	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	8
43	63	1	%f	%d
42	47	1	%f	%d
42	64	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	9
42	64	1	%f	%d
41	46	1	%f	%d
41	65	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	10
41	65	1	%f	%d
40	45	1	%f	%d
40	66	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	11
40	66	1	%f	%d
39	44	1	%f	%d
39	67	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	12
39	67	1	%f	%d
38	43	1	%f	%d
38	68	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	13
38	68	1	%f	%d
37	42	1	%f	%d
37	69	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	14
37	69	1	%f	%d
36	41	1	%f	%d
36	70	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	15
36	70	1	%f	%d
35	40	1	%f	%d
35	71	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	16
35	71	1	%f	%d
34	39	1	%f	%d
34	72	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	17
34	72	1	%f	%d
33	38	1	%f	%d
33	73	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	18
33	73	1	%f	%d
32	37	1	%f	%d
32	74	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	19
32	74	1	%f	%d
31	36	1	%f	%d
31	75	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	20
31	75	1	%f	%d
30	35	1	%f	%d
30	76	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	21
30	76	1	%f	%d
29	34	1	%f	%d
29	77	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	22
29	77	1	%f	%d
28	33	1	%f	%d
28	78	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	23
28	78	1	%f	%d
27	32	1	%f	%d
27	79	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	24
27	79	1	%f	%d
26	31	1	%f	%d
26	80	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	25
26	80	1	%f	%d
25	30	1	%f	%d
25	81	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	26
25	81	1	%f	%d
24	29	1	%f	%d
24	82	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	27
24	82	1	%f	%d
23	28	1	%f	%d
23	83	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	28
23	83	1	%f	%d
22	27	1	%f	%d
22	84	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	29
22	84	1	%f	%d
21	26	1	%f	%d
21	85	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	30
21	85	1	%f	%d
20	25	1	%f	%d
20	86	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	31
20	86	1	%f	%d
19	24	1	%f	%d
19	87	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	32
19	87	1	%f	%d
18	23	1	%f	%d
18	88	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	33
18	88	1	%f	%d
17	22	1	%f	%d
17	89	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	34
17	89	1	%f	%d
16	21	1	%f	%d
16	90	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	35
16	90	1	%f	%d
15	20	1	%f	%d
15	91	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	36
15	91	1	%f	%d
14	19	1	%f	%d
14	92	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	37
14	92	1	%f	%d
13	18	1	%f	%d
13	93	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	38
13	93	1	%f	%d
12	17	1	%f	%d
12	94	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	39
12	94	1	%f	%d
11	16	1	%f	%d
11	95	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	40
11	95	1	%f	%d
10	15	1	%f	%d
10	96	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	41
10	96	1	%f	%d
9	14	1	%f	%d
9	97	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	42
9	97	1	%f	%d
8	13	1	%f	%d
8	98	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	43
8	98	1	%f	%d
7	12	1	%f	%d
7	99	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	44
7	99	1	%f	%d
6	11	1	%f	%d
6	100	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	45
6	100	1	%f	%d
5	10	1	%f	%d
5	101	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	46
5	101	1	%f	%d
4	9	1	%f	%d
4	102	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	47
4	102	1	%f	%d
3	8	1	%f	%d
3	103	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	48
3	103	1	%f	%d
2	7	1	%f	%d
2	104	0	%f	%d	xdebug_stop_trace	0		%strace-001.php	23	0
			%f	%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
