--TEST--
Trace test with fibonacci numbers (format=1)
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.trace_format=0
--FILE--
<?php
	$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE), XDEBUG_TRACE_COMPUTERIZED);
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
	echo file_get_contents($tf);
	unlink($tf);
?>
--EXPECTF--
Version: %d.%s
File format: %d
TRACE START [%d-%d-%d %d:%d:%d.%d]
2	4	0	%f	%d	fibonacci_cache	1		%strace-001.php	22	1	50
3	5	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	49
4	6	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	48
5	7	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	47
6	8	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	46
7	9	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	45
8	10	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	44
9	11	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	43
10	12	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	42
11	13	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	41
12	14	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	40
13	15	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	39
14	16	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	38
15	17	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	37
16	18	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	36
17	19	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	35
18	20	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	34
19	21	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	33
20	22	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	32
21	23	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	31
22	24	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	30
23	25	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	29
24	26	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	28
25	27	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	27
26	28	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	26
27	29	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	25
28	30	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	24
29	31	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	23
30	32	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	22
31	33	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	21
32	34	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	20
33	35	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	19
34	36	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	18
35	37	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	17
36	38	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	16
37	39	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	15
38	40	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	14
39	41	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	13
40	42	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	12
41	43	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	11
42	44	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	10
43	45	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	9
44	46	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	8
45	47	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	7
46	48	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	6
47	49	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	5
48	50	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	4
49	51	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	3
50	52	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	2
50	52	1	%f	%d
50	53	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	1
50	53	1	%f	%d
49	51	1	%f	%d
49	54	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	2
49	54	1	%f	%d
48	50	1	%f	%d
48	55	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	3
48	55	1	%f	%d
47	49	1	%f	%d
47	56	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	4
47	56	1	%f	%d
46	48	1	%f	%d
46	57	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	5
46	57	1	%f	%d
45	47	1	%f	%d
45	58	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	6
45	58	1	%f	%d
44	46	1	%f	%d
44	59	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	7
44	59	1	%f	%d
43	45	1	%f	%d
43	60	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	8
43	60	1	%f	%d
42	44	1	%f	%d
42	61	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	9
42	61	1	%f	%d
41	43	1	%f	%d
41	62	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	10
41	62	1	%f	%d
40	42	1	%f	%d
40	63	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	11
40	63	1	%f	%d
39	41	1	%f	%d
39	64	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	12
39	64	1	%f	%d
38	40	1	%f	%d
38	65	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	13
38	65	1	%f	%d
37	39	1	%f	%d
37	66	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	14
37	66	1	%f	%d
36	38	1	%f	%d
36	67	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	15
36	67	1	%f	%d
35	37	1	%f	%d
35	68	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	16
35	68	1	%f	%d
34	36	1	%f	%d
34	69	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	17
34	69	1	%f	%d
33	35	1	%f	%d
33	70	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	18
33	70	1	%f	%d
32	34	1	%f	%d
32	71	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	19
32	71	1	%f	%d
31	33	1	%f	%d
31	72	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	20
31	72	1	%f	%d
30	32	1	%f	%d
30	73	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	21
30	73	1	%f	%d
29	31	1	%f	%d
29	74	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	22
29	74	1	%f	%d
28	30	1	%f	%d
28	75	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	23
28	75	1	%f	%d
27	29	1	%f	%d
27	76	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	24
27	76	1	%f	%d
26	28	1	%f	%d
26	77	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	25
26	77	1	%f	%d
25	27	1	%f	%d
25	78	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	26
25	78	1	%f	%d
24	26	1	%f	%d
24	79	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	27
24	79	1	%f	%d
23	25	1	%f	%d
23	80	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	28
23	80	1	%f	%d
22	24	1	%f	%d
22	81	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	29
22	81	1	%f	%d
21	23	1	%f	%d
21	82	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	30
21	82	1	%f	%d
20	22	1	%f	%d
20	83	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	31
20	83	1	%f	%d
19	21	1	%f	%d
19	84	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	32
19	84	1	%f	%d
18	20	1	%f	%d
18	85	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	33
18	85	1	%f	%d
17	19	1	%f	%d
17	86	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	34
17	86	1	%f	%d
16	18	1	%f	%d
16	87	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	35
16	87	1	%f	%d
15	17	1	%f	%d
15	88	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	36
15	88	1	%f	%d
14	16	1	%f	%d
14	89	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	37
14	89	1	%f	%d
13	15	1	%f	%d
13	90	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	38
13	90	1	%f	%d
12	14	1	%f	%d
12	91	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	39
12	91	1	%f	%d
11	13	1	%f	%d
11	92	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	40
11	92	1	%f	%d
10	12	1	%f	%d
10	93	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	41
10	93	1	%f	%d
9	11	1	%f	%d
9	94	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	42
9	94	1	%f	%d
8	10	1	%f	%d
8	95	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	43
8	95	1	%f	%d
7	9	1	%f	%d
7	96	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	44
7	96	1	%f	%d
6	8	1	%f	%d
6	97	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	45
6	97	1	%f	%d
5	7	1	%f	%d
5	98	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	46
5	98	1	%f	%d
4	6	1	%f	%d
4	99	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	47
4	99	1	%f	%d
3	5	1	%f	%d
3	100	0	%f	%d	fibonacci_cache	1		%strace-001.php	16	1	48
3	100	1	%f	%d
2	4	1	%f	%d
2	101	0	%f	%d	xdebug_stop_trace	0		%strace-001.php	23	0
			%f	%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
