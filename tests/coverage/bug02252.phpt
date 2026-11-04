--TEST--
Test for bug #2252: Segfault in xdebug_branch_info_mark_reached due to stale last_branch_nr
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.1');
?>
--INI--
xdebug.mode=coverage
--FILE--
<?php
/* Root cause of bug #2252:
 *
 * Functions defined before xdebug_set_filter() is called are compiled with
 * filter_type_code_coverage == XDEBUG_FILTER_NONE, so xdebug_coverage_init_oparray()
 * leaves op_array->reserved[filter_offset] == 0.  The opcode handler therefore
 * fires for them regardless of any later xdebug_set_filter() call.
 *
 * When xdebug_set_filter() is then set to an include list that EXCLUDES those
 * functions, fse->filtered_code_coverage == 1 and start_of_function() is
 * skipped — so last_branch_nr[D] is never reset for those functions.
 *
 * If a prior function at the same stack depth D left last_branch_nr[D] at an
 * opcode index >= the excluded function's branch_info->size, the subsequent
 * access to branch_info->branches[last_branch_nr[D]] is out-of-bounds.
 *
 * The fix adds a bounds check in xdebug_branch_info_mark_reached(): when
 * last_branch_nr[D] >= branch_info->size, the stale value is reset to -1 and
 * the out-of-bounds access is prevented.
 */

/* This function has many branches. It runs at stack depth D and leaves
 * last_branch_nr[D] at a high opcode index.
 * What an argument near the middle of the range (v=100), roughly 99 if-checks
 * fail before the matching return is reached. Each failed check advances the
 * executed opcode counter by ~3, leaving last_branch_nr[D] ≈ 300 after the
 * call. That value is then used as an index into fewBranches()'s tiny
 * branch_info->branches array (size ≈ 3), producing an out-of-bounds access
 * that falls ≈ 100 KB past the allocation boundary and into unmapped virtual
 * memory. */
function manyBranches(int $v): string
{
	if ($v ===   1) return '1';   if ($v ===   2) return '2';   if ($v ===   3) return '3';   if ($v ===   4) return '4';   if ($v ===   5) return '5';
	if ($v ===   6) return '6';   if ($v ===   7) return '7';   if ($v ===   8) return '8';   if ($v ===   9) return '9';   if ($v ===  10) return '10';
	if ($v ===  11) return '11';  if ($v ===  12) return '12';  if ($v ===  13) return '13';  if ($v ===  14) return '14';  if ($v ===  15) return '15';
	if ($v ===  16) return '16';  if ($v ===  17) return '17';  if ($v ===  18) return '18';  if ($v ===  19) return '19';  if ($v ===  20) return '20';
	if ($v ===  21) return '21';  if ($v ===  22) return '22';  if ($v ===  23) return '23';  if ($v ===  24) return '24';  if ($v ===  25) return '25';
	if ($v ===  26) return '26';  if ($v ===  27) return '27';  if ($v ===  28) return '28';  if ($v ===  29) return '29';  if ($v ===  30) return '30';
	if ($v ===  31) return '31';  if ($v ===  32) return '32';  if ($v ===  33) return '33';  if ($v ===  34) return '34';  if ($v ===  35) return '35';
	if ($v ===  36) return '36';  if ($v ===  37) return '37';  if ($v ===  38) return '38';  if ($v ===  39) return '39';  if ($v ===  40) return '40';
	if ($v ===  41) return '41';  if ($v ===  42) return '42';  if ($v ===  43) return '43';  if ($v ===  44) return '44';  if ($v ===  45) return '45';
	if ($v ===  46) return '46';  if ($v ===  47) return '47';  if ($v ===  48) return '48';  if ($v ===  49) return '49';  if ($v ===  50) return '50';
	if ($v ===  51) return '51';  if ($v ===  52) return '52';  if ($v ===  53) return '53';  if ($v ===  54) return '54';  if ($v ===  55) return '55';
	if ($v ===  56) return '56';  if ($v ===  57) return '57';  if ($v ===  58) return '58';  if ($v ===  59) return '59';  if ($v ===  60) return '60';
	if ($v ===  61) return '61';  if ($v ===  62) return '62';  if ($v ===  63) return '63';  if ($v ===  64) return '64';  if ($v ===  65) return '65';
	if ($v ===  66) return '66';  if ($v ===  67) return '67';  if ($v ===  68) return '68';  if ($v ===  69) return '69';  if ($v ===  70) return '70';
	if ($v ===  71) return '71';  if ($v ===  72) return '72';  if ($v ===  73) return '73';  if ($v ===  74) return '74';  if ($v ===  75) return '75';
	if ($v ===  76) return '76';  if ($v ===  77) return '77';  if ($v ===  78) return '78';  if ($v ===  79) return '79';  if ($v ===  80) return '80';
	if ($v ===  81) return '81';  if ($v ===  82) return '82';  if ($v ===  83) return '83';  if ($v ===  84) return '84';  if ($v ===  85) return '85';
	if ($v ===  86) return '86';  if ($v ===  87) return '87';  if ($v ===  88) return '88';  if ($v ===  89) return '89';  if ($v ===  90) return '90';
	if ($v ===  91) return '91';  if ($v ===  92) return '92';  if ($v ===  93) return '93';  if ($v ===  94) return '94';  if ($v ===  95) return '95';
	if ($v ===  96) return '96';  if ($v ===  97) return '97';  if ($v ===  98) return '98';  if ($v ===  99) return '99';  if ($v === 100) return '100';
	if ($v === 101) return '101'; if ($v === 102) return '102'; if ($v === 103) return '103'; if ($v === 104) return '104'; if ($v === 105) return '105';
	if ($v === 106) return '106'; if ($v === 107) return '107'; if ($v === 108) return '108'; if ($v === 109) return '109'; if ($v === 110) return '110';
	if ($v === 111) return '111'; if ($v === 112) return '112'; if ($v === 113) return '113'; if ($v === 114) return '114'; if ($v === 115) return '115';
	if ($v === 116) return '116'; if ($v === 117) return '117'; if ($v === 118) return '118'; if ($v === 119) return '119'; if ($v === 120) return '120';
	if ($v === 121) return '121'; if ($v === 122) return '122'; if ($v === 123) return '123'; if ($v === 124) return '124'; if ($v === 125) return '125';
	if ($v === 126) return '126'; if ($v === 127) return '127'; if ($v === 128) return '128'; if ($v === 129) return '129'; if ($v === 130) return '130';
	if ($v === 131) return '131'; if ($v === 132) return '132'; if ($v === 133) return '133'; if ($v === 134) return '134'; if ($v === 135) return '135';
	if ($v === 136) return '136'; if ($v === 137) return '137'; if ($v === 138) return '138'; if ($v === 139) return '139'; if ($v === 140) return '140';
	if ($v === 141) return '141'; if ($v === 142) return '142'; if ($v === 143) return '143'; if ($v === 144) return '144'; if ($v === 145) return '145';
	if ($v === 146) return '146'; if ($v === 147) return '147'; if ($v === 148) return '148'; if ($v === 149) return '149'; if ($v === 150) return '150';
	if ($v === 151) return '151'; if ($v === 152) return '152'; if ($v === 153) return '153'; if ($v === 154) return '154'; if ($v === 155) return '155';
	if ($v === 156) return '156'; if ($v === 157) return '157'; if ($v === 158) return '158'; if ($v === 159) return '159'; if ($v === 160) return '160';
	if ($v === 161) return '161'; if ($v === 162) return '162'; if ($v === 163) return '163'; if ($v === 164) return '164'; if ($v === 165) return '165';
	if ($v === 166) return '166'; if ($v === 167) return '167'; if ($v === 168) return '168'; if ($v === 169) return '169'; if ($v === 170) return '170';
	if ($v === 171) return '171'; if ($v === 172) return '172'; if ($v === 173) return '173'; if ($v === 174) return '174'; if ($v === 175) return '175';
	if ($v === 176) return '176'; if ($v === 177) return '177'; if ($v === 178) return '178'; if ($v === 179) return '179'; if ($v === 180) return '180';
	if ($v === 181) return '181'; if ($v === 182) return '182'; if ($v === 183) return '183'; if ($v === 184) return '184'; if ($v === 185) return '185';
	if ($v === 186) return '186'; if ($v === 187) return '187'; if ($v === 188) return '188'; if ($v === 189) return '189'; if ($v === 190) return '190';
	if ($v === 191) return '191'; if ($v === 192) return '192'; if ($v === 193) return '193'; if ($v === 194) return '194'; if ($v === 195) return '195';
	if ($v === 196) return '196'; if ($v === 197) return '197'; if ($v === 198) return '198'; if ($v === 199) return '199'; if ($v === 200) return '200';
	return 'other';
}

/* This function has very few branches.  Both functions are compiled before any
 * filter is active, so reserved[filter_offset] == 0 for both. After
 * xdebug_set_filter() excludes this function at runtime, start_of_function()
 * is skipped for it while the opcode handler still fires — leaving
 * last_branch_nr[D] stale from manyBranches(). */
function fewBranches(int $v): string
{
	return $v > 0 ? 'positive' : 'non-positive';
}

/* Step 1 – start path coverage so start_of_function() is wired up.
 * hugeBranches(100) executes ~99 failed if-checks before returning, leaving
 * last_branch_nr[D] ≈ 300 (≈ 99 × 3 opcodes per check). */
xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);

/* Step 2 – call manyBranches to (a) prefill branch-info for the whole file
 * (including fewBranches) and (b) leave last_branch_nr[D] at a high value. */
manyBranches(100);

/* Step 3 – configure a filter that EXCLUDES the current file at runtime.
 * Because the functions were compiled before this call (XDEBUG_FILTER_NONE at
 * compile time), reserved[filter_offset] is still 0 and the opcode handler
 * continues to fire for them. */
xdebug_set_filter(
	XDEBUG_FILTER_CODE_COVERAGE,
	XDEBUG_PATH_INCLUDE,
	['/this/path/does/not/exist/']
);

/* Step 4 – call fewBranches at the same depth D.
 * opcode handler fires (reserved[filter_offset] == 0)
 * start_of_function is skipped (fse->filtered_code_coverage == 1)
 * last_branch_nr[D] is stale from manyBranches → OOB without the fix. */
fewBranches(1);

xdebug_stop_code_coverage();

/* Reaching this line means no crash occurred — the fix works. */
echo "Done\n";
?>
--EXPECT--
Done
