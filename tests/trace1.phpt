--TEST--
Trace test with fibonacci numbers (format=1)
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
--FILE--
<?php
	$tf = xdebug_start_trace('/tmp/'. uniqid('xdt', TRUE), XDEBUG_TRACE_COMPUTERIZED);
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
Version: 2.%d.%d%s
File format: 2
TRACE START [%d-%d-%d %d:%d:%d]
2	2	1	%f	%d
2	3	0	%f	%d	fibonacci_cache	1		%strace1.php	22
3	4	0	%f	%d	fibonacci_cache	1		%strace1.php	16
4	5	0	%f	%d	fibonacci_cache	1		%strace1.php	16
5	6	0	%f	%d	fibonacci_cache	1		%strace1.php	16
6	7	0	%f	%d	fibonacci_cache	1		%strace1.php	16
7	8	0	%f	%d	fibonacci_cache	1		%strace1.php	16
8	9	0	%f	%d	fibonacci_cache	1		%strace1.php	16
9	10	0	%f	%d	fibonacci_cache	1		%strace1.php	16
10	11	0	%f	%d	fibonacci_cache	1		%strace1.php	16
11	12	0	%f	%d	fibonacci_cache	1		%strace1.php	16
12	13	0	%f	%d	fibonacci_cache	1		%strace1.php	16
13	14	0	%f	%d	fibonacci_cache	1		%strace1.php	16
14	15	0	%f	%d	fibonacci_cache	1		%strace1.php	16
15	16	0	%f	%d	fibonacci_cache	1		%strace1.php	16
16	17	0	%f	%d	fibonacci_cache	1		%strace1.php	16
17	18	0	%f	%d	fibonacci_cache	1		%strace1.php	16
18	19	0	%f	%d	fibonacci_cache	1		%strace1.php	16
19	20	0	%f	%d	fibonacci_cache	1		%strace1.php	16
20	21	0	%f	%d	fibonacci_cache	1		%strace1.php	16
21	22	0	%f	%d	fibonacci_cache	1		%strace1.php	16
22	23	0	%f	%d	fibonacci_cache	1		%strace1.php	16
23	24	0	%f	%d	fibonacci_cache	1		%strace1.php	16
24	25	0	%f	%d	fibonacci_cache	1		%strace1.php	16
25	26	0	%f	%d	fibonacci_cache	1		%strace1.php	16
26	27	0	%f	%d	fibonacci_cache	1		%strace1.php	16
27	28	0	%f	%d	fibonacci_cache	1		%strace1.php	16
28	29	0	%f	%d	fibonacci_cache	1		%strace1.php	16
29	30	0	%f	%d	fibonacci_cache	1		%strace1.php	16
30	31	0	%f	%d	fibonacci_cache	1		%strace1.php	16
31	32	0	%f	%d	fibonacci_cache	1		%strace1.php	16
32	33	0	%f	%d	fibonacci_cache	1		%strace1.php	16
33	34	0	%f	%d	fibonacci_cache	1		%strace1.php	16
34	35	0	%f	%d	fibonacci_cache	1		%strace1.php	16
35	36	0	%f	%d	fibonacci_cache	1		%strace1.php	16
36	37	0	%f	%d	fibonacci_cache	1		%strace1.php	16
37	38	0	%f	%d	fibonacci_cache	1		%strace1.php	16
38	39	0	%f	%d	fibonacci_cache	1		%strace1.php	16
39	40	0	%f	%d	fibonacci_cache	1		%strace1.php	16
40	41	0	%f	%d	fibonacci_cache	1		%strace1.php	16
41	42	0	%f	%d	fibonacci_cache	1		%strace1.php	16
42	43	0	%f	%d	fibonacci_cache	1		%strace1.php	16
43	44	0	%f	%d	fibonacci_cache	1		%strace1.php	16
44	45	0	%f	%d	fibonacci_cache	1		%strace1.php	16
45	46	0	%f	%d	fibonacci_cache	1		%strace1.php	16
46	47	0	%f	%d	fibonacci_cache	1		%strace1.php	16
47	48	0	%f	%d	fibonacci_cache	1		%strace1.php	16
48	49	0	%f	%d	fibonacci_cache	1		%strace1.php	16
49	50	0	%f	%d	fibonacci_cache	1		%strace1.php	16
50	51	0	%f	%d	fibonacci_cache	1		%strace1.php	16
50	51	1	%f	%d
50	52	0	%f	%d	fibonacci_cache	1		%strace1.php	16
50	52	1	%f	%d
49	50	1	%f	%d
49	53	0	%f	%d	fibonacci_cache	1		%strace1.php	16
49	53	1	%f	%d
48	49	1	%f	%d
48	54	0	%f	%d	fibonacci_cache	1		%strace1.php	16
48	54	1	%f	%d
47	48	1	%f	%d
47	55	0	%f	%d	fibonacci_cache	1		%strace1.php	16
47	55	1	%f	%d
46	47	1	%f	%d
46	56	0	%f	%d	fibonacci_cache	1		%strace1.php	16
46	56	1	%f	%d
45	46	1	%f	%d
45	57	0	%f	%d	fibonacci_cache	1		%strace1.php	16
45	57	1	%f	%d
44	45	1	%f	%d
44	58	0	%f	%d	fibonacci_cache	1		%strace1.php	16
44	58	1	%f	%d
43	44	1	%f	%d
43	59	0	%f	%d	fibonacci_cache	1		%strace1.php	16
43	59	1	%f	%d
42	43	1	%f	%d
42	60	0	%f	%d	fibonacci_cache	1		%strace1.php	16
42	60	1	%f	%d
41	42	1	%f	%d
41	61	0	%f	%d	fibonacci_cache	1		%strace1.php	16
41	61	1	%f	%d
40	41	1	%f	%d
40	62	0	%f	%d	fibonacci_cache	1		%strace1.php	16
40	62	1	%f	%d
39	40	1	%f	%d
39	63	0	%f	%d	fibonacci_cache	1		%strace1.php	16
39	63	1	%f	%d
38	39	1	%f	%d
38	64	0	%f	%d	fibonacci_cache	1		%strace1.php	16
38	64	1	%f	%d
37	38	1	%f	%d
37	65	0	%f	%d	fibonacci_cache	1		%strace1.php	16
37	65	1	%f	%d
36	37	1	%f	%d
36	66	0	%f	%d	fibonacci_cache	1		%strace1.php	16
36	66	1	%f	%d
35	36	1	%f	%d
35	67	0	%f	%d	fibonacci_cache	1		%strace1.php	16
35	67	1	%f	%d
34	35	1	%f	%d
34	68	0	%f	%d	fibonacci_cache	1		%strace1.php	16
34	68	1	%f	%d
33	34	1	%f	%d
33	69	0	%f	%d	fibonacci_cache	1		%strace1.php	16
33	69	1	%f	%d
32	33	1	%f	%d
32	70	0	%f	%d	fibonacci_cache	1		%strace1.php	16
32	70	1	%f	%d
31	32	1	%f	%d
31	71	0	%f	%d	fibonacci_cache	1		%strace1.php	16
31	71	1	%f	%d
30	31	1	%f	%d
30	72	0	%f	%d	fibonacci_cache	1		%strace1.php	16
30	72	1	%f	%d
29	30	1	%f	%d
29	73	0	%f	%d	fibonacci_cache	1		%strace1.php	16
29	73	1	%f	%d
28	29	1	%f	%d
28	74	0	%f	%d	fibonacci_cache	1		%strace1.php	16
28	74	1	%f	%d
27	28	1	%f	%d
27	75	0	%f	%d	fibonacci_cache	1		%strace1.php	16
27	75	1	%f	%d
26	27	1	%f	%d
26	76	0	%f	%d	fibonacci_cache	1		%strace1.php	16
26	76	1	%f	%d
25	26	1	%f	%d
25	77	0	%f	%d	fibonacci_cache	1		%strace1.php	16
25	77	1	%f	%d
24	25	1	%f	%d
24	78	0	%f	%d	fibonacci_cache	1		%strace1.php	16
24	78	1	%f	%d
23	24	1	%f	%d
23	79	0	%f	%d	fibonacci_cache	1		%strace1.php	16
23	79	1	%f	%d
22	23	1	%f	%d
22	80	0	%f	%d	fibonacci_cache	1		%strace1.php	16
22	80	1	%f	%d
21	22	1	%f	%d
21	81	0	%f	%d	fibonacci_cache	1		%strace1.php	16
21	81	1	%f	%d
20	21	1	%f	%d
20	82	0	%f	%d	fibonacci_cache	1		%strace1.php	16
20	82	1	%f	%d
19	20	1	%f	%d
19	83	0	%f	%d	fibonacci_cache	1		%strace1.php	16
19	83	1	%f	%d
18	19	1	%f	%d
18	84	0	%f	%d	fibonacci_cache	1		%strace1.php	16
18	84	1	%f	%d
17	18	1	%f	%d
17	85	0	%f	%d	fibonacci_cache	1		%strace1.php	16
17	85	1	%f	%d
16	17	1	%f	%d
16	86	0	%f	%d	fibonacci_cache	1		%strace1.php	16
16	86	1	%f	%d
15	16	1	%f	%d
15	87	0	%f	%d	fibonacci_cache	1		%strace1.php	16
15	87	1	%f	%d
14	15	1	%f	%d
14	88	0	%f	%d	fibonacci_cache	1		%strace1.php	16
14	88	1	%f	%d
13	14	1	%f	%d
13	89	0	%f	%d	fibonacci_cache	1		%strace1.php	16
13	89	1	%f	%d
12	13	1	%f	%d
12	90	0	%f	%d	fibonacci_cache	1		%strace1.php	16
12	90	1	%f	%d
11	12	1	%f	%d
11	91	0	%f	%d	fibonacci_cache	1		%strace1.php	16
11	91	1	%f	%d
10	11	1	%f	%d
10	92	0	%f	%d	fibonacci_cache	1		%strace1.php	16
10	92	1	%f	%d
9	10	1	%f	%d
9	93	0	%f	%d	fibonacci_cache	1		%strace1.php	16
9	93	1	%f	%d
8	9	1	%f	%d
8	94	0	%f	%d	fibonacci_cache	1		%strace1.php	16
8	94	1	%f	%d
7	8	1	%f	%d
7	95	0	%f	%d	fibonacci_cache	1		%strace1.php	16
7	95	1	%f	%d
6	7	1	%f	%d
6	96	0	%f	%d	fibonacci_cache	1		%strace1.php	16
6	96	1	%f	%d
5	6	1	%f	%d
5	97	0	%f	%d	fibonacci_cache	1		%strace1.php	16
5	97	1	%f	%d
4	5	1	%f	%d
4	98	0	%f	%d	fibonacci_cache	1		%strace1.php	16
4	98	1	%f	%d
3	4	1	%f	%d
3	99	0	%f	%d	fibonacci_cache	1		%strace1.php	16
3	99	1	%f	%d
2	3	1	%f	%d
2	100	0	%f	%d	xdebug_stop_trace	0		%strace1.php	23
			%f	%d
TRACE END   [%d-%d-%d %d:%d:%d]
