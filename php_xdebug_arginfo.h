/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 1f39dc9f957609f28ad544ac0e4d6cef0a57d789 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xdebug_break, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xdebug_call_class, 0, 0, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, depth, IS_LONG, 0, "2")
ZEND_END_ARG_INFO()

#define arginfo_xdebug_call_file arginfo_xdebug_call_class

#define arginfo_xdebug_call_function arginfo_xdebug_call_class

#define arginfo_xdebug_call_line arginfo_xdebug_call_class

#define arginfo_xdebug_code_coverage_started arginfo_xdebug_break

ZEND_BEGIN_ARG_INFO_EX(arginfo_xdebug_debug_zval, 0, 0, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, varname, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_xdebug_debug_zval_stdout arginfo_xdebug_debug_zval

ZEND_BEGIN_ARG_INFO_EX(arginfo_xdebug_dump_superglobals, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xdebug_get_code_coverage, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xdebug_get_collected_errors, 0, 0, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, emptyList, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xdebug_get_function_count, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_xdebug_get_function_stack arginfo_xdebug_get_code_coverage

#define arginfo_xdebug_get_gc_run_count arginfo_xdebug_get_function_count

#define arginfo_xdebug_get_gc_total_collected_roots arginfo_xdebug_get_function_count

#define arginfo_xdebug_get_gcstats_filename arginfo_xdebug_dump_superglobals

#define arginfo_xdebug_get_headers arginfo_xdebug_get_code_coverage

#define arginfo_xdebug_get_monitored_functions arginfo_xdebug_get_code_coverage

#define arginfo_xdebug_get_profiler_filename arginfo_xdebug_dump_superglobals

#define arginfo_xdebug_get_stack_depth arginfo_xdebug_get_function_count

#define arginfo_xdebug_get_tracefile_name arginfo_xdebug_dump_superglobals

#define arginfo_xdebug_info arginfo_xdebug_dump_superglobals

#define arginfo_xdebug_is_debugger_active arginfo_xdebug_break

#define arginfo_xdebug_memory_usage arginfo_xdebug_get_function_count

#define arginfo_xdebug_peak_memory_usage arginfo_xdebug_get_function_count

ZEND_BEGIN_ARG_INFO_EX(arginfo_xdebug_print_function_stack, 0, 0, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, message, IS_STRING, 0, "\"user triggered\"")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xdebug_set_filter, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, group, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, listType, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, configuration, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xdebug_start_code_coverage, 0, 0, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

#define arginfo_xdebug_start_error_collection arginfo_xdebug_dump_superglobals

ZEND_BEGIN_ARG_INFO_EX(arginfo_xdebug_start_function_monitor, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, listOfFunctionsToMonitor, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xdebug_start_gcstats, 0, 0, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, gcstatsFile, IS_STRING, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xdebug_start_trace, 0, 0, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, traceFile, IS_STRING, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xdebug_stop_code_coverage, 0, 0, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, cleanUp, _IS_BOOL, 0, "true")
ZEND_END_ARG_INFO()

#define arginfo_xdebug_stop_error_collection arginfo_xdebug_dump_superglobals

#define arginfo_xdebug_stop_function_monitor arginfo_xdebug_dump_superglobals

#define arginfo_xdebug_stop_gcstats arginfo_xdebug_dump_superglobals

#define arginfo_xdebug_stop_trace arginfo_xdebug_dump_superglobals

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xdebug_time_index, 0, 0, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xdebug_var_dump, 0, 0, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, variable, IS_MIXED, 0)
ZEND_END_ARG_INFO()


ZEND_FUNCTION(xdebug_break);
ZEND_FUNCTION(xdebug_call_class);
ZEND_FUNCTION(xdebug_call_file);
ZEND_FUNCTION(xdebug_call_function);
ZEND_FUNCTION(xdebug_call_line);
ZEND_FUNCTION(xdebug_code_coverage_started);
ZEND_FUNCTION(xdebug_debug_zval);
ZEND_FUNCTION(xdebug_debug_zval_stdout);
ZEND_FUNCTION(xdebug_dump_superglobals);
ZEND_FUNCTION(xdebug_get_code_coverage);
ZEND_FUNCTION(xdebug_get_collected_errors);
ZEND_FUNCTION(xdebug_get_function_count);
ZEND_FUNCTION(xdebug_get_function_stack);
ZEND_FUNCTION(xdebug_get_gc_run_count);
ZEND_FUNCTION(xdebug_get_gc_total_collected_roots);
ZEND_FUNCTION(xdebug_get_gcstats_filename);
ZEND_FUNCTION(xdebug_get_headers);
ZEND_FUNCTION(xdebug_get_monitored_functions);
ZEND_FUNCTION(xdebug_get_profiler_filename);
ZEND_FUNCTION(xdebug_get_stack_depth);
ZEND_FUNCTION(xdebug_get_tracefile_name);
ZEND_FUNCTION(xdebug_info);
ZEND_FUNCTION(xdebug_is_debugger_active);
ZEND_FUNCTION(xdebug_memory_usage);
ZEND_FUNCTION(xdebug_peak_memory_usage);
ZEND_FUNCTION(xdebug_print_function_stack);
ZEND_FUNCTION(xdebug_set_filter);
ZEND_FUNCTION(xdebug_start_code_coverage);
ZEND_FUNCTION(xdebug_start_error_collection);
ZEND_FUNCTION(xdebug_start_function_monitor);
ZEND_FUNCTION(xdebug_start_gcstats);
ZEND_FUNCTION(xdebug_start_trace);
ZEND_FUNCTION(xdebug_stop_code_coverage);
ZEND_FUNCTION(xdebug_stop_error_collection);
ZEND_FUNCTION(xdebug_stop_function_monitor);
ZEND_FUNCTION(xdebug_stop_gcstats);
ZEND_FUNCTION(xdebug_stop_trace);
ZEND_FUNCTION(xdebug_time_index);
ZEND_FUNCTION(xdebug_var_dump);


static const zend_function_entry ext_functions[] = {
	ZEND_FE(xdebug_break, arginfo_xdebug_break)
	ZEND_FE(xdebug_call_class, arginfo_xdebug_call_class)
	ZEND_FE(xdebug_call_file, arginfo_xdebug_call_file)
	ZEND_FE(xdebug_call_function, arginfo_xdebug_call_function)
	ZEND_FE(xdebug_call_line, arginfo_xdebug_call_line)
	ZEND_FE(xdebug_code_coverage_started, arginfo_xdebug_code_coverage_started)
	ZEND_FE(xdebug_debug_zval, arginfo_xdebug_debug_zval)
	ZEND_FE(xdebug_debug_zval_stdout, arginfo_xdebug_debug_zval_stdout)
	ZEND_FE(xdebug_dump_superglobals, arginfo_xdebug_dump_superglobals)
	ZEND_FE(xdebug_get_code_coverage, arginfo_xdebug_get_code_coverage)
	ZEND_FE(xdebug_get_collected_errors, arginfo_xdebug_get_collected_errors)
	ZEND_FE(xdebug_get_function_count, arginfo_xdebug_get_function_count)
	ZEND_FE(xdebug_get_function_stack, arginfo_xdebug_get_function_stack)
	ZEND_FE(xdebug_get_gc_run_count, arginfo_xdebug_get_gc_run_count)
	ZEND_FE(xdebug_get_gc_total_collected_roots, arginfo_xdebug_get_gc_total_collected_roots)
	ZEND_FE(xdebug_get_gcstats_filename, arginfo_xdebug_get_gcstats_filename)
	ZEND_FE(xdebug_get_headers, arginfo_xdebug_get_headers)
	ZEND_FE(xdebug_get_monitored_functions, arginfo_xdebug_get_monitored_functions)
	ZEND_FE(xdebug_get_profiler_filename, arginfo_xdebug_get_profiler_filename)
	ZEND_FE(xdebug_get_stack_depth, arginfo_xdebug_get_stack_depth)
	ZEND_FE(xdebug_get_tracefile_name, arginfo_xdebug_get_tracefile_name)
	ZEND_FE(xdebug_info, arginfo_xdebug_info)
	ZEND_FE(xdebug_is_debugger_active, arginfo_xdebug_is_debugger_active)
	ZEND_FE(xdebug_memory_usage, arginfo_xdebug_memory_usage)
	ZEND_FE(xdebug_peak_memory_usage, arginfo_xdebug_peak_memory_usage)
	ZEND_FE(xdebug_print_function_stack, arginfo_xdebug_print_function_stack)
	ZEND_FE(xdebug_set_filter, arginfo_xdebug_set_filter)
	ZEND_FE(xdebug_start_code_coverage, arginfo_xdebug_start_code_coverage)
	ZEND_FE(xdebug_start_error_collection, arginfo_xdebug_start_error_collection)
	ZEND_FE(xdebug_start_function_monitor, arginfo_xdebug_start_function_monitor)
	ZEND_FE(xdebug_start_gcstats, arginfo_xdebug_start_gcstats)
	ZEND_FE(xdebug_start_trace, arginfo_xdebug_start_trace)
	ZEND_FE(xdebug_stop_code_coverage, arginfo_xdebug_stop_code_coverage)
	ZEND_FE(xdebug_stop_error_collection, arginfo_xdebug_stop_error_collection)
	ZEND_FE(xdebug_stop_function_monitor, arginfo_xdebug_stop_function_monitor)
	ZEND_FE(xdebug_stop_gcstats, arginfo_xdebug_stop_gcstats)
	ZEND_FE(xdebug_stop_trace, arginfo_xdebug_stop_trace)
	ZEND_FE(xdebug_time_index, arginfo_xdebug_time_index)
	ZEND_FE(xdebug_var_dump, arginfo_xdebug_var_dump)
	ZEND_FE_END
};
