#include "php.h"
#include "TSRM.h"
#include "php_globals.h"
#include "php_xdebug.h"
#include "xdebug_llist.h"
#include "xdebug_profiler.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

inline double get_mtimestamp()
{
	struct timeval tv;
#ifdef HAVE_GETTIMEOFDAY
	if (gettimeofday(&tv, NULL) == 0) {
		return (double) (tv.tv_sec + (tv.tv_usec / MICRO_IN_SEC));
	}
#endif
	return 0;
}

inline static void init_profile_modes (char ***mode_titles)
{
	*mode_titles = xdmalloc(XDEBUG_PROFILER_MODES * sizeof(char *));
	
	(*mode_titles)[0] = XDEBUG_PROFILER_LBL_D;
	(*mode_titles)[1] = XDEBUG_PROFILER_CPU_D;
	(*mode_titles)[2] = XDEBUG_PROFILER_NC_D;
	(*mode_titles)[3] = XDEBUG_PROFILER_FS_AV_D;
	(*mode_titles)[4] = XDEBUG_PROFILER_FS_SUM_D;
	(*mode_titles)[5] = XDEBUG_PROFILER_FS_NC_D;
	(*mode_titles)[6] = XDEBUG_PROFILER_SD_LBL_D;
	(*mode_titles)[7] = XDEBUG_PROFILER_SD_CPU_D;
	(*mode_titles)[8] = XDEBUG_PROFILER_SD_NC_D;
	(*mode_titles)[9] = XDEBUG_PROFILER_FC_SUM_D;
}

inline static void free_profile_modes (char ***mode_titles)
{
	xdfree(*mode_titles);
}

inline static int time_taken_cmp (function_stack_entry **p1, function_stack_entry **p2)
{
	if ((*p1)->time_taken < (*p2)->time_taken) {
		return 1;
	} else if ((*p1)->time_taken > (*p2)->time_taken) {
		return -1;
	} else {
		return 0;
	}
}

inline static int n_calls_cmp (function_stack_entry **p1, function_stack_entry **p2)
{
	if ((*p1)->f_calls < (*p2)->f_calls) {
		return 1;
	} else if ((*p1)->f_calls > (*p2)->f_calls) {
		return -1;
	} else {
		return 0;
	}
}

inline static int line_numbers (function_stack_entry **p1, function_stack_entry **p2)
{
	if ((*p1)->lineno > (*p2)->lineno) {
		return 1;
	} else if ((*p1)->lineno < (*p2)->lineno) {
		return -1;
	} else {
		return 0;
	}
}

inline static int avg_time_cmp (function_stack_entry **p1, function_stack_entry **p2)
{
	double avg1, avg2;

	avg1 = (*p1)->time_taken / (*p1)->f_calls;
	avg2 = (*p2)->time_taken / (*p2)->f_calls;

	if (avg1 < avg2) {
		return 1;
	} else if (avg1 > avg2) {
		return -1;
	} else {
		return 0;
	}
}

inline static int time_taken_tree_cmp (xdebug_tree_out **p1, xdebug_tree_out **p2)
{
	double i = ((*p1)->fse)->time_taken - ((*p2)->fse)->time_taken;

	if (i < 0) {
		return 1;
	} else if (i > 0) {
		return -1;
	} else {
		return 0;
	}
}

inline static int n_calls_tree_cmp (xdebug_tree_out **p1, xdebug_tree_out **p2)
{
	if (((*p1)->fse)->f_calls < ((*p2)->fse)->f_calls) {
		return 1;
	} else if (((*p1)->fse)->f_calls > ((*p2)->fse)->f_calls) {
		return -1;
	} else {
		return 0;
	}
}

inline static int n_line_no_cmp (xdebug_tree_out **p1, xdebug_tree_out **p2)
{
	if (((*p1)->fse)->lineno > ((*p2)->fse)->lineno) {
		return 1;
	} else if (((*p1)->fse)->lineno < ((*p2)->fse)->lineno) {
		return -1;
	} else {
		return 0;
	}
}

inline static void add_function_entry(xdebug_hash *hasht, function_stack_entry *ent)
{
	xdebug_hash_add(hasht, ent->function.function, strlen(ent->function.function), ent);
}

inline static int find_and_inc_function_entry (xdebug_hash *hasht, function_stack_entry *ent, int all)
{
	function_stack_entry *found_ent;
	
	if (ent->function.function && xdebug_hash_find(hasht, ent->function.function, strlen(ent->function.function), (void *) &found_ent)) {
		if (!all && (found_ent->lineno != ent->lineno || strcasecmp(found_ent->filename, ent->filename))) {
			return 0;
		}
		
		if (all == 2 && found_ent->level != ent->level) {
			return 0;
		}
		
		if (found_ent->function.type == ent->function.type && found_ent->function.internal == ent->function.internal && found_ent->function.class == ent->function.class) {
			if (found_ent->function.class && strcasecmp(found_ent->function.class, ent->function.class)) {
				return 0;
			}
			if (all > 0) {
				found_ent->time_taken += ent->time_taken;
				found_ent->f_calls++;
			}	
			return 1;
		}
	}
	
	return 0;
}

static inline function_stack_entry **fetch_tree_profile (int mode, int *size, double total_time TSRMLS_DC)
{
	xdebug_llist_element *le;

	if (XG(trace) && XDEBUG_LLIST_COUNT(XG(trace))) {
		function_stack_entry  *ent, **list_out;
		unsigned int           n_elem = XDEBUG_LLIST_COUNT(XG(trace));
		int                    j, level_cnt, i = 0;
		xdebug_tree_out	      *cur = NULL, *parent = NULL, **list, **pos;
		xdebug_hash           *function_hash;

		list = (xdebug_tree_out **) xdmalloc(n_elem * sizeof(xdebug_tree_out *));
		function_hash = xdebug_hash_alloc(n_elem, NULL);
		pos = &list[0];

		level_cnt = 1;

		cur = xdcalloc(1, sizeof(struct xdebug_tree_out));
		pos[i++] = cur;

		for (le = XDEBUG_LLIST_HEAD(XG(trace)); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
			if (XDEBUG_LLIST_IS_TAIL(le) && !XG(auto_profile) != 2) {
				break;
			}

			ent = XDEBUG_LLIST_VALP(le);
			ent->f_calls = 1;
			
			if (ent->function.function) {
				if (!find_and_inc_function_entry(function_hash, ent, 2)) {
					add_function_entry(function_hash, ent);
				} else {
					continue;
				}
			}
			
			if (ent->level == 1 && ent->time_taken > 1000000.00) {
				ent->time_taken = total_time;
			}

			if (level_cnt < ent->level) { /* level down */
				parent = cur;
				parent->nelem++;
				level_cnt++;
			} else if (level_cnt > ent->level) { /* level up */
				parent = cur->parent;
				while (level_cnt > ent->level) {
					parent = parent->parent;
					level_cnt--;	
				}

				if (parent) {
					parent->nelem++;
				}
			} else {
				if (cur && cur->parent) {
					parent = cur->parent;
					parent->nelem++;
				}	
			}

			if (ent->level == 1) {
				cur->fse = ent;
				pos[0] = cur;
			} else {
				cur = xdcalloc(1, sizeof(struct xdebug_tree_out));
				cur->fse = ent;
				pos[i++] = cur;
				if (!cur->parent) {
					cur->parent = parent;
				}
			}	
		}

		xdebug_hash_destroy(function_hash);
		
		/* build a list of children & sort them based on the specified sorting scheme */
		for (j = 0; j < i; j++) {
			if (list[j]->nelem) {
				list[j]->children = xdmalloc(list[j]->nelem * sizeof(xdebug_tree_out *));
			}
			if (list[j]->parent) {
				(list[j]->parent)->children[(list[j]->parent)->pos] = list[j];
				(list[j]->parent)->pos++;
			}
			if (list[j]->parent && (list[j]->parent)->pos == (list[j]->parent)->nelem) {
				if ((list[j]->parent)->nelem > 1) {
					switch (mode) {
						case XDEBUG_PROFILER_SD_CPU:
							qsort((list[j]->parent)->children, (list[j]->parent)->nelem, sizeof(xdebug_tree_out *), (int (*)()) &time_taken_tree_cmp);
							break;
						case XDEBUG_PROFILER_SD_NC:
							qsort((list[j]->parent)->children, (list[j]->parent)->nelem, sizeof(xdebug_tree_out *), (int (*)()) &n_calls_tree_cmp);
							break;	
						default:
							qsort((list[j]->parent)->children, (list[j]->parent)->nelem, sizeof(xdebug_tree_out *), (int (*)()) &n_line_no_cmp);
							break;
					}
				}	
				(list[j]->parent)->pos = 0;
			}
		}
		
		/* Creation of output linked list and freeing of previously allocated memory */
		list_out = (function_stack_entry **) xdmalloc(i * sizeof(function_stack_entry *));
		*size = 0;
		cur = pos[0];
		
		while (cur->pos < cur->nelem) {
			if (cur->fse) {
				list_out[(*size)++] = cur->fse;
				cur->fse = NULL;
			}
			cur = cur->children[cur->pos++];
			if (cur->fse) {
				list_out[(*size)++] = cur->fse;
				cur->fse = NULL;
			}
			if (!cur->nelem) {
				while (cur->nelem == cur->pos && cur->parent) {
					cur = cur->parent;
				}
			}
		}	
			
		/* deallocate memory */
		for (j = 0; j < i; j++) {
			if (list[j]->nelem) {
				xdfree(list[j]->children);
			}
			xdfree(list[j]);
		}
		xdfree(list);
		
		return list_out;
	}
	return (function_stack_entry **) NULL;
}

inline static int find_same_function (xdebug_hash *hasht, xdebug_fs *cur, function_stack_entry *parent)
{
	xdebug_fs *found_ent = NULL;
	
	if (parent && cur->fse->function.function && xdebug_hash_find(hasht, cur->fse->function.function, strlen(cur->fse->function.function), (void *) &found_ent)) {
		if (found_ent->fse->function.class == cur->fse->function.class) {
			if (cur->fse->function.class && strcasecmp(found_ent->fse->function.class, cur->fse->function.class)) {
				goto add_new_ent;
			}
			
			found_ent->fse->f_calls++;
			found_ent->fse->time_taken += cur->fse->time_taken;
			
			found_ent->nelem_p++;
			found_ent->parents = xdrealloc(found_ent->parents, found_ent->nelem_p * sizeof(function_stack_entry *));
			found_ent->parents[found_ent->nelem_p-1] = parent;
					
			return 1;
		}
	}	

add_new_ent:
	
	cur->nelem_c = 0;
	
	if (parent) {
		cur->nelem_p = 1;
		cur->parents = xdmalloc(cur->nelem_p * sizeof(function_stack_entry *));
		cur->parents[0] = parent;
	}

	cur->time = cur->fse->time_taken;
	if (cur->fse->function.function) {
		xdebug_hash_add(hasht, cur->fse->function.function, strlen(cur->fse->function.function), cur);
	}	
	
	return 0;
}

inline static xdebug_fs *find_parent(xdebug_hash *hasht, char *func_name)
{
	xdebug_fs *found_ent = NULL;

	xdebug_hash_find(hasht, func_name, strlen(func_name), (void *) &found_ent);
	
	return found_ent;
}

static inline xdebug_fs **fetch_fcall_summary (int mode, int *size, double total_time TSRMLS_DC)
{
	xdebug_llist_element *le;

	if (XG(trace) && XDEBUG_LLIST_COUNT(XG(trace))) {
		function_stack_entry  *ent, **llist;
		int                    j, i = 0, k;
		xdebug_fs	      *cur = NULL, *parent = NULL, **pos, **list, **list_out;
		xdebug_hash           *function_hash;

		llist = xdcalloc(XG(max_nesting_level), sizeof(function_stack_entry *));
		list = (xdebug_fs **) xdmalloc(XDEBUG_LLIST_COUNT(XG(trace)) * sizeof(xdebug_fs *));
		pos = &list[0];
		
		function_hash = xdebug_hash_alloc(XDEBUG_LLIST_COUNT(XG(trace)), NULL);

		for (le = XDEBUG_LLIST_HEAD(XG(trace)); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
			if (XDEBUG_LLIST_IS_TAIL(le) && !XG(auto_profile) != 2) {
				break;
			}
			
			ent = XDEBUG_LLIST_VALP(le);
			ent->f_calls = 1;
			
			if (ent->level == 1 && ent->time_taken > 1000000.00) {
				ent->time_taken = total_time;
			}

			llist[ent->level] = ent;
			cur = xdcalloc(1, sizeof(struct xdebug_fs));
			
			pos[i] = cur;
			pos[i]->fse = ent;

			if (llist[ent->level - 1] && (parent = find_parent(function_hash, llist[ent->level - 1]->function.function))) {
				parent->nelem_c++;
				parent->children = xdrealloc(parent->children, parent->nelem_c * sizeof(function_stack_entry *));
				parent->children[parent->nelem_c-1] = ent;
			} else {
				parent = NULL;
			}
			
			if (!find_same_function(function_hash, pos[i], llist[ent->level - 1])) {
				i++;
			} else {
				xdfree(cur);
			}	
		}
		
		xdebug_hash_destroy(function_hash);
		xdfree(llist);
		
		list_out = (xdebug_fs **) xdmalloc(i * sizeof(xdebug_fs *));
		
		for (j = 0; j < i; j++) {
			list_out[j] = xdcalloc(1, sizeof(struct xdebug_fs));
			list_out[j]->fse = list[j]->fse;
			list_out[j]->time = list_out[j]->fse->time_taken;
		
			if (list[j]->nelem_c) {
				function_hash = xdebug_hash_alloc(list[j]->nelem_c, NULL);
				list_out[j]->nelem_c = 0;
				
				for (k = 0; k < list[j]->nelem_c; k++) {
					ent = list[j]->children[k];
					if (ent->function.function) {
						list_out[j]->time -= ent->time_taken;
					
						if (!find_and_inc_function_entry(function_hash, ent, 1)) {
							list_out[j]->nelem_c++;
							list_out[j]->children = xdrealloc(list_out[j]->children, list_out[j]->nelem_c * sizeof(function_stack_entry *));
							list_out[j]->children[list_out[j]->nelem_c - 1] = xdmalloc(sizeof(function_stack_entry));
							list_out[j]->children[list_out[j]->nelem_c - 1]->function = ent->function;
							list_out[j]->children[list_out[j]->nelem_c - 1]->f_calls = 1;
							list_out[j]->children[list_out[j]->nelem_c - 1]->time_taken = ent->time_taken;
							list_out[j]->children[list_out[j]->nelem_c - 1]->filename = ent->filename;
							list_out[j]->children[list_out[j]->nelem_c - 1]->lineno =ent->lineno;
							add_function_entry(function_hash, list_out[j]->children[list_out[j]->nelem_c - 1]);
 						}
 					}		
				}	
				
				xdfree(list[j]->children);
				xdebug_hash_destroy(function_hash);
			}

			if (list[j]->nelem_p) {
				function_hash = xdebug_hash_alloc(list[j]->nelem_p, NULL);
				list_out[j]->nelem_p = 0;
				
				for (k = 0; k < list[j]->nelem_p; k++) {
					ent = list[j]->parents[k];
					if (!find_and_inc_function_entry(function_hash, ent, 1)) {
						list_out[j]->nelem_p++;
						list_out[j]->parents = xdrealloc(list_out[j]->parents, list_out[j]->nelem_p * sizeof(function_stack_entry *));
						list_out[j]->parents[list_out[j]->nelem_p - 1] = xdmalloc(sizeof(function_stack_entry));
						list_out[j]->parents[list_out[j]->nelem_p - 1]->function = ent->function;
						list_out[j]->parents[list_out[j]->nelem_p - 1]->f_calls = 1;
						list_out[j]->parents[list_out[j]->nelem_p - 1]->time_taken = ent->time_taken;
						list_out[j]->parents[list_out[j]->nelem_p - 1]->filename = ent->filename;
						list_out[j]->parents[list_out[j]->nelem_p - 1]->lineno =ent->lineno;
						add_function_entry(function_hash, list_out[j]->parents[list_out[j]->nelem_p - 1]);
					}
				}
				
				xdfree(list[j]->parents);
				xdebug_hash_destroy(function_hash);
			}
			
			xdfree(list[j]);
		}
		
		xdfree(list);
		
		*size = i;
		return list_out;
	}
			
	return (xdebug_fs **) NULL;
}


static inline function_stack_entry **fetch_simple_profile (int mode, int *size, double total_time TSRMLS_DC)
{
	xdebug_llist_element *le;

	if (XG(trace) && XDEBUG_LLIST_COUNT(XG(trace))) {
		function_stack_entry       **list, **start_p, *ent;
		unsigned int                 n_elem = XDEBUG_LLIST_COUNT(XG(trace));
		int                          i = 0;
		xdebug_hash                  *function_hash;
		int                          mode_op;
		
		list = (function_stack_entry **) xdmalloc(n_elem * sizeof(function_stack_entry *));
		function_hash = xdebug_hash_alloc(n_elem, NULL);
		start_p = &list[0];
		
		if (mode >= XDEBUG_PROFILER_FS_AV) {
			mode_op = 1;
		} else {
			mode_op = 0;
		}
		
		for (le = XDEBUG_LLIST_HEAD(XG(trace)); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
			/* skip the last function because it is our very own profile function.
			 * since it is the last function, we may as well stop the loop.
			 */
			if (XDEBUG_LLIST_IS_TAIL(le) && !XG(auto_profile) != 2) {
				break;
			}
		
			ent = XDEBUG_LLIST_VALP(le);
			
			if (ent->level == 1 && ent->time_taken > 1000000.00) {
				ent->time_taken = total_time;
			}
			
			if (ent->function.function) {
				if (!find_and_inc_function_entry(function_hash, ent, mode_op)) {
					start_p[i] = XDEBUG_LLIST_VALP(le);
					start_p[i]->f_calls = 1;
					add_function_entry(function_hash, start_p[i]);
					i++;
				}
			} else {
				start_p[i] = XDEBUG_LLIST_VALP(le);
				start_p[i]->f_calls = 1;
				i++;
			}
		}
		
		*size = n_elem = i;
		xdebug_hash_destroy(function_hash);
		
		switch (mode) {
			case XDEBUG_PROFILER_CPU:
			case XDEBUG_PROFILER_FS_SUM:
				qsort(list, n_elem, sizeof(function_stack_entry *), (int (*)()) &time_taken_cmp);
				break;
			case XDEBUG_PROFILER_NC:
			case XDEBUG_PROFILER_FS_NC:
				qsort(list, n_elem, sizeof(function_stack_entry *), (int (*)()) &n_calls_cmp);
				break;
			case XDEBUG_PROFILER_FS_AV:
				qsort(list, n_elem, sizeof(function_stack_entry *), (int (*)()) &avg_time_cmp);
				break;	
			default:
				qsort(list, n_elem, sizeof(function_stack_entry *), (int (*)()) &line_numbers);
				break;
		}
		
		return list;
	}

	return (function_stack_entry **) NULL;
}

static inline void fetch_full_function_name (function_stack_entry *ent, char *buf)
{
	char *p;
	
	p = buf;
	
	if (ent->user_defined == XDEBUG_EXTERNAL) {
		sprintf(buf, "*");
		p++;
	}
	if (ent->function.class) {
		if (ent->function.type == XFUNC_MEMBER) { 
			snprintf(p, XDEBUG_MAX_FUNCTION_LEN - (p-buf), "%s->%s", ent->function.class, ent->function.function);
		} else {
			snprintf(p, XDEBUG_MAX_FUNCTION_LEN - (p-buf), "%s::%s", ent->function.class, ent->function.function);
		}
		return;
	}
	if (ent->function.function) {
		snprintf(p, XDEBUG_MAX_FUNCTION_LEN - (p-buf), "%s", ent->function.function);
	}

	switch (ent->function.type) {
		case XFUNC_NEW:
			sprintf(buf, "%s", "{new}");
			break;
		case XFUNC_EVAL:
			sprintf(buf, "%s", "{eval}");
			break;
		case XFUNC_INCLUDE:
			sprintf(buf, "%s", "{include}");
			break;
		case XFUNC_INCLUDE_ONCE:
			sprintf(buf, "%s", "{include_once}");
			break;
		case XFUNC_REQUIRE:
			sprintf(buf, "%s", "{require}");
			break;
		case XFUNC_REQUIRE_ONCE:
			sprintf(buf, "%s", "{require_once}");
			break;
		default:
			buf = NULL;
			break;
	}
}

void print_profile(int html, int mode TSRMLS_DC)
{
	FILE                  *data_output;
	char                 **mode_titles;
	double                 total_time = get_mtimestamp() - XG(total_execution_time);
	double                 total_function_exec = 0.0;
	function_stack_entry  *ent, **list;
	char                   buffer[XDEBUG_MAX_FUNCTION_LEN];
	int                    i, size = 0;

	if (mode < 0 || mode >= XDEBUG_PROFILER_MODES) {
		php_error(E_WARNING, "'%d' is not a valid profiling flag\n", mode);
		return;
	}

	init_profile_modes(&mode_titles);
	
	if (!html) {
		if (XG(profile_file)) {
			data_output = XG(profile_file);
		} else {
			data_output = stdout;
		}
		fprintf(data_output, "\n%s\n", mode_titles[mode]);
	} else {
		php_printf("<br />\n<table border='1' cellspacing='0'>\n");
		php_printf("<tr><th bgcolor='#aaaaaa' colspan='4'>%s</th></tr>\n", mode_titles[mode]);
	}	

	switch (mode) {
		case XDEBUG_PROFILER_LBL:
		case XDEBUG_PROFILER_CPU:
		case XDEBUG_PROFILER_NC: {
			if (!(list = fetch_simple_profile(mode, &size, total_time TSRMLS_CC))) {
				goto err;
			}

			if (!html) {
				fprintf(data_output, "-----------------------------------------------------------------------------------\n");
				fprintf(data_output, "Time Taken    Number of Calls    Function Name    Location\n");
				fprintf(data_output, "-----------------------------------------------------------------------------------\n");
			} else {
				php_printf("<tr><th bgcolor='#cccccc'>Time Taken</th><th bgcolor='#cccccc'>Number of Calls</th><th bgcolor='#cccccc'>Function Name</th><th bgcolor='#cccccc'>Location</th></tr>\n");
			}

			for (i = 0; i < size; i++) {
				ent = list[i];

				if (ent->level == 2) {
					total_function_exec += ent->time_taken;
				}

				fetch_full_function_name(ent, buffer);

				if (html) {
					php_printf("<tr><td>%10.10f</td><td>%u</td><td>%s()</td><td>%s:%d</td></tr>\n", ent->time_taken, ent->f_calls, buffer, ent->filename, ent->lineno);
				} else { 
					fprintf(data_output, "%10.10f    %u    %s()    %s:%d\n", ent->time_taken, ent->f_calls, buffer, ent->filename, ent->lineno);
				}
			}
			
			/* free memory allocated by fetch_simple_profile() */
			xdfree(list);
			break;
		}
		case XDEBUG_PROFILER_FS_AV:
		case XDEBUG_PROFILER_FS_SUM:
		case XDEBUG_PROFILER_FS_NC: {
			if (!(list = fetch_simple_profile(mode, &size, total_time TSRMLS_CC))) {
				goto err;
			}

			if (!html) {
				fprintf(data_output, "-----------------------------------------------------------------------------------\n");
				fprintf(data_output, "Total Time Taken    Avg. Time Taken    Number of Calls    Function Name\n");
				fprintf(data_output, "-----------------------------------------------------------------------------------\n");
			} else {
				php_printf("<tr><th bgcolor='#cccccc'>Total Time Taken</th><th bgcolor='#cccccc'>Avg. Time Taken</th><th bgcolor='#cccccc'>Number of Calls</th><th bgcolor='#cccccc'>Function Name</th></tr>\n");
			}

			for (i = 0; i < size; i++) {
				ent = list[i];

				if (ent->level == 2) {
					total_function_exec += ent->time_taken;
				}

				fetch_full_function_name(ent, buffer);

				if (html) {
					php_printf("<tr><td bgcolor='#ffffff'>%10.10f</td><td bgcolor='#ffffff'>%10.10f</td><td bgcolor='#ffffff'>%u</td><td bgcolor='#ffffff'>%s</td></tr>\n",
						ent->time_taken, (ent->time_taken / ent->f_calls), ent->f_calls, buffer);
				} else {
					fprintf(data_output, "%10.10f    %10.10f    %u    %s\n", ent->time_taken, (ent->time_taken / ent->f_calls), ent->f_calls, buffer);
				}
			}

			/* free memory allocated by fetch_simple_profile() */
			xdfree(list);
			break;
		}
		case XDEBUG_PROFILER_SD_LBL:
		case XDEBUG_PROFILER_SD_CPU:
		case XDEBUG_PROFILER_SD_NC: {
			int j;
		
			if (!(list = fetch_tree_profile(mode, &size, total_time TSRMLS_CC))) {
				goto err;
			}
			
			if (!html) {
				fprintf(data_output, "-----------------------------------------------------------------------------------\n");
				fprintf(data_output, "Time Taken    Number of Calls    Function Name    Location\n");
				fprintf(data_output, "-----------------------------------------------------------------------------------\n");
			} else {
				php_printf("<tr><th bgcolor='#cccccc'>Time Taken</th><th bgcolor='#cccccc'>Number of Calls</th><th bgcolor='#cccccc'>Function Name</th><th bgcolor='#cccccc'>Location</th></tr>\n");
			}
			
			for (i = 0; i < size; i++) {
				ent = list[i];

				if (ent->level == 2) {
					total_function_exec += ent->time_taken;
				}

				fetch_full_function_name(ent, buffer);

				if (html) {
					php_printf("<tr><td bgcolor='#ffffff'>");
					for (j = 2; j < ent->level; j++) {
						php_printf("&nbsp;&nbsp;");
					}
					php_printf("-&gt; %10.10f</td><td bgcolor='#ffffff'>%u</td><td bgcolor='#ffffff'>%s</td><td bgcolor='#ffffff'>%s:%d</td></tr>\n",
						ent->time_taken, ent->f_calls, buffer, ent->filename, ent->lineno);
				} else {
					for (j = 2; j < ent->level; j++) {
						fprintf(data_output, "  ");
					}
					fprintf(data_output, "-> %10.10f    %u    %s    %s:%d\n", ent->time_taken, ent->f_calls, buffer, ent->filename, ent->lineno);
				}
			}

			/* free memory allocated by fetch_tree_profile() */
			xdfree(list);
			break;
		}
		case XDEBUG_PROFILER_FC_SUM: {
			xdebug_fs **llist;
			int j;
		
			if (!(llist = fetch_fcall_summary(mode, &size, total_time TSRMLS_CC))) {
				goto err;
			}
			
			if (!html) {
				fprintf(data_output, "-----------------------------------------------------------------------------------\n");
				fprintf(data_output, "Function    Called function    Number of calls    Time Taken    Location\n");
				fprintf(data_output, "-----------------------------------------------------------------------------------\n");
			} else {
				php_printf("<tr><th bgcolor='#cccccc'>Function</th><th bgcolor='#cccccc'>Called function</th><th bgcolor='#cccccc'>Number of calls</th><th bgcolor='#cccccc'>Time Taken</th><th bgcolor='#cccccc'>Time Taken</th><th bgcolor='#cccccc'>Location</th></tr>\n");
			}
			
			for (i = 0; i < size; i++) {
				fetch_full_function_name(llist[i]->fse, buffer);
				if (llist[i]->fse->level == 2) {
					total_function_exec += llist[i]->fse->time_taken;
				}
				
				if (html) {
					php_printf("<tr><td bgcolor='#cccccc' colspan='2'>%s</td><td bgcolor='#cccccc'>%u</td><td bgcolor='#cccccc'>%10.10f (%2.0f%%)</td><td bgcolor='#cccccc'>%s:%d</td></tr>\n",
						buffer, llist[i]->fse->f_calls, llist[i]->fse->time_taken, (llist[i]->time / llist[i]->fse->time_taken * 100), llist[i]->fse->filename, llist[i]->fse->lineno);
									
					if (llist[i]->nelem_c) {
						php_printf("<tr><td bgcolor='#cccccc' colspan='5'>Children:</td></tr>");
					
						fprintf(data_output, "    Children:\n");
						for (j = 0; j < llist[i]->nelem_c; j++) {
							ent = llist[i]->children[j];
							fetch_full_function_name(ent, buffer);
							
							php_printf("<tr><td bgcolor='#cccccc'> </td><td bgcolor='#cccccc'>%s</td><td bgcolor='#cccccc'>%u</td><td bgcolor='#cccccc'>%10.10f (%2.0f%%)</td><td bgcolor='#cccccc'>%s:%d</td></tr>\n",
								buffer, llist[i]->fse->f_calls, llist[i]->fse->time_taken, (llist[i]->time / llist[i]->fse->time_taken * 100), llist[i]->fse->filename, llist[i]->fse->lineno);
							
							xdfree(llist[i]->children[j]);
						}
						xdfree(llist[i]->children);
					}
					
					if (llist[i]->nelem_p) {
						php_printf("<tr><td bgcolor='#cccccc' colspan='5'>Parents:</td></tr>");
						for (j = 0; j < llist[i]->nelem_p; j++) {
							ent = llist[i]->parents[j];
							fetch_full_function_name(ent, buffer);
							
							php_printf("<tr><td bgcolor='#cccccc'> </td><td bgcolor='#cccccc'>%s</td><td bgcolor='#cccccc'>%u</td><td bgcolor='#cccccc'>%2.0f%%</td><td bgcolor='#cccccc'>%s:%d</td></tr>\n",
								buffer, ent->f_calls, (llist[i]->fse->time_taken  /  ent->time_taken * 100), ent->filename, ent->lineno);

							xdfree(llist[i]->parents[j]);
						}
						xdfree(llist[i]->parents);
					}
				} else {
					fprintf(data_output, "%-30s    %u    %10.10f (%2.0f%%)  %s:%d\n",
						buffer, llist[i]->fse->f_calls, llist[i]->fse->time_taken, (llist[i]->time / llist[i]->fse->time_taken * 100), llist[i]->fse->filename, llist[i]->fse->lineno);
					if (llist[i]->nelem_c) {
						fprintf(data_output, "    Children:\n");
						for (j = 0; j < llist[i]->nelem_c; j++) {
							ent = llist[i]->children[j];
							fetch_full_function_name(ent, buffer);
							fprintf(data_output, "        %-22s    %u    %10.10f (%2.0f%%)  %s:%d\n", buffer, ent->f_calls, ent->time_taken, (ent->time_taken / llist[i]->fse->time_taken * 100), ent->filename, ent->lineno);
							xdfree(llist[i]->children[j]);
						}
						xdfree(llist[i]->children);
					}
					
					if (llist[i]->nelem_p) {
						fprintf(data_output, "    Parents:\n");
						for (j = 0; j < llist[i]->nelem_p; j++) {
							ent = llist[i]->parents[j];
							fetch_full_function_name(ent, buffer);
							fprintf(data_output, "        %-22s    %u    %2.0f%%  %s:%d\n", buffer, ent->f_calls, (llist[i]->fse->time_taken  /  ent->time_taken * 100), ent->filename, ent->lineno);
							xdfree(llist[i]->parents[j]);
						}
						xdfree(llist[i]->parents);
					}
					
					fprintf(data_output, "----------------------------------\n");
				}
				
				xdfree(llist[i]);
			}

			xdfree(llist);
			break;
		}	
	}

	if (html) {
		php_printf("</table>\n");
	} else {
		fprintf(data_output, "-----------------------------------------------------------------------------------\n");
		fprintf(data_output, "Opcode Compiling:                             %10.10f\n", XG(total_compiling_time));
		fprintf(data_output, "Function Execution:     %10.10f\n", total_function_exec);
		fprintf(data_output, "Ambient Code Execution: %10.10f\n", (total_time - total_function_exec));
		fprintf(data_output, "Total Execution:                              %10.10f\n", total_time);
		fprintf(data_output, "-----------------------------------------------------------------------------------\n");
		fprintf(data_output, "Total Processing:                             %10.10f\n", XG(total_compiling_time) + total_time);
		fprintf(data_output, "-----------------------------------------------------------------------------------\n");
	}
err:
	free_profile_modes(&mode_titles);
	return;
}

inline static void append_frame (zval **dest, function_stack_entry *ent)
{
	zval *frame = *dest;
	
	if (ent->function.function) {
		add_assoc_string_ex(frame, "function", sizeof("function"), ent->function.function, 1);
	} else {
		switch (ent->function.type) {
			case XFUNC_NEW:
				add_assoc_string_ex(frame, "function", sizeof("function"), "{new}", 1);
				break;
			case XFUNC_EVAL:
				add_assoc_string_ex(frame, "function", sizeof("function"), "{eval}", 1);
				break;
			case XFUNC_INCLUDE:
				add_assoc_string_ex(frame, "function", sizeof("function"), "{include}", 1);
				break;
			case XFUNC_INCLUDE_ONCE:
				add_assoc_string_ex(frame, "function", sizeof("function"), "{include_once}", 1);
				break;
			case XFUNC_REQUIRE:
				add_assoc_string_ex(frame, "function", sizeof("function"), "{require}", 1);
				break;
			case XFUNC_REQUIRE_ONCE:
				add_assoc_string_ex(frame, "function", sizeof("function"), "{require_once}", 1);
				break;
		}
	}
	
	if (ent->function.class) {
		add_assoc_string_ex(frame, "class", sizeof("class"), ent->function.class, 1);
		if (ent->function.type == XFUNC_MEMBER) {
			add_assoc_string_ex(frame, "method_type", sizeof("public"), "public", 1);
		} else {
			add_assoc_string_ex(frame, "method_type", sizeof("static"), "static", 1);
		}
	}
	
	add_assoc_long_ex(frame, "n_calls", sizeof("n_calls"), ent->f_calls);
	add_assoc_double_ex(frame, "ttl_time", sizeof("ttl_time"), ent->time_taken);
	
	if (ent->f_calls > 1) {
		add_assoc_double_ex(frame, "avg_time", sizeof("avg_time"), (ent->time_taken / ent->f_calls));
	}	
	
	if (ent->user_defined != XDEBUG_EXTERNAL) {
		add_assoc_string_ex(frame, "origin", sizeof("origin"), "php", 1);
	} else {
		add_assoc_string_ex(frame, "origin", sizeof("origin"), "user", 1);
	}
	
	add_assoc_long_ex(frame, "level", sizeof("level"), ent->level - 2);
}

PHP_FUNCTION(xdebug_get_function_profile)
{
	if (XG(do_profile) == 1) {
		long                  profile_flag = XDEBUG_PROFILER_LBL;
		function_stack_entry *ent, **list;
		double                total_time = get_mtimestamp() - XG(total_execution_time);
		double                total_function_exec = 0.0;
		zval                 *frame;
		int                   i, size = 0;

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &profile_flag) == FAILURE) {
			return;
		}

		switch (profile_flag) {
			case XDEBUG_PROFILER_LBL:
			case XDEBUG_PROFILER_CPU:
			case XDEBUG_PROFILER_NC:
			case XDEBUG_PROFILER_FS_AV:
			case XDEBUG_PROFILER_FS_SUM:
			case XDEBUG_PROFILER_FS_NC:
				if (!(list = fetch_simple_profile(profile_flag, &size, total_time TSRMLS_CC))) {
					goto err;
				}
				break;
			case XDEBUG_PROFILER_SD_LBL:
			case XDEBUG_PROFILER_SD_CPU:
			case XDEBUG_PROFILER_SD_NC:
				if (!(list = fetch_tree_profile(profile_flag, &size, total_time TSRMLS_CC))) {
					goto err;
				} 
				break;
			case XDEBUG_PROFILER_FC_SUM: {
				xdebug_fs **llist;
				zval       *children, *parents, *child, *parent;
				int         j;
			
				if (!(llist = fetch_fcall_summary(profile_flag, &size, total_time TSRMLS_CC))) {
					goto err;
				}
				
				array_init(return_value);
				
				for (i = 0; i < size; i++) {
					ent = llist[i]->fse;
					
					if (ent->level == 2) {
						total_function_exec += ent->time_taken;
					}

					MAKE_STD_ZVAL(frame);
					array_init(frame);
					append_frame(&frame, ent);
					
					if (llist[i]->nelem_c) {
						MAKE_STD_ZVAL(children);
						array_init(children);
						
						for (j = 0; j < llist[i]->nelem_c; j++) {
							MAKE_STD_ZVAL(child);
							array_init(child);
							append_frame(&child, llist[i]->children[j]);
							add_next_index_zval(children, child);
						}
						add_assoc_zval_ex(frame, "children", sizeof("children"), children);
					}
					if (llist[i]->nelem_p) {
						MAKE_STD_ZVAL(parents);
						array_init(parents);
						
						for (j = 0; j < llist[i]->nelem_p; j++) {
							MAKE_STD_ZVAL(parent);
							array_init(parent);
							append_frame(&parent, llist[i]->parents[j]);
							add_next_index_zval(parents, parent);
						}
						add_assoc_zval_ex(frame, "parents", sizeof("parents"), parents);
					}
					add_next_index_zval(return_value, frame);
				}
				goto finish;
				break;
			}
			default:
				php_error(E_WARNING, "'%l' is not a valid profiling flag\n", profile_flag);
err:
				RETURN_FALSE;
				break;
		}		

		array_init(return_value);

		for (i = 0; i < size; i++) {
			ent = list[i];

			if (ent->level == 2) {
				total_function_exec += ent->time_taken;
			}

			/* Initialize frame array */
			MAKE_STD_ZVAL(frame);
			array_init(frame);
		
			append_frame(&frame, ent);
			add_next_index_zval(return_value, frame);
		}

		xdfree(list);
finish:
		/* Add a general timing data */
		add_assoc_double_ex(return_value, "opcode_compile_time",     sizeof("opcode_compile_time"),     XG(total_compiling_time));
		add_assoc_double_ex(return_value, "function_execution",      sizeof("function_execution"),      total_function_exec);
		add_assoc_double_ex(return_value, "ambient_code_execution",  sizeof("ambient_code_execution"),  total_time - total_function_exec);
		add_assoc_double_ex(return_value, "total_execution",         sizeof("total_execution"),         total_time);
		add_assoc_double_ex(return_value, "total_script_processing", sizeof("total_script_processing"), XG(total_compiling_time) + total_time);
	} else {
		php_error(E_WARNING, "Function profiling was not started, use xdebug_start_profiling() before calling this function");
		RETURN_FALSE;
	}
}

PHP_FUNCTION(xdebug_start_profiling)
{
	if (XG(do_profile) == 0) {
		char *fname = NULL;
		int fname_len;
	
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &fname, &fname_len) == FAILURE) {
			return;
		}

		if (XG(do_trace) == 0) { /* we need the tracer to be on to work */
			xdebug_start_trace();
			XG(profiler_trace) = 1;
			XG(trace_file) = NULL;
		} else {
			XG(profiler_trace) = 0;
		}

		XG(do_profile) = 1;
		if (!XG(total_execution_time)) {
			XG(total_execution_time) = get_mtimestamp();
		}	

		if (fname) {
			XG(profile_file) = fopen(fname, "a");
			if (XG(profile_file)) {
				fprintf(XG(profile_file), "\nStart of function profiler\n");
			} else {
				php_error(E_NOTICE, "Could not open '%s', filesystem said: %s", fname, strerror(errno));
				XG(profile_file) = NULL;
			}
		} else {
			XG(profile_file) = NULL;
		}
	} else {
		php_error(E_NOTICE, "Function profiler already started");
	}
}

PHP_FUNCTION(xdebug_stop_profiling)
{
	if (XG(do_profile) == 1) {
		if (XG(do_trace) == 1) {
			XG(do_trace) = 0;
			xdebug_llist_destroy(XG(trace), NULL);
			XG(trace) = NULL;
			XG(profiler_trace) = 0;
		}
		XG(do_profile) = 0;
		if (XG(profile_file)) {
			fprintf(XG(profile_file), "End of function profiler\n");
			fclose(XG(profile_file));
		}
	} else {
		php_error(E_NOTICE, "Function profiling was not started");
	}
}

PHP_FUNCTION(xdebug_dump_function_profile)
{
	if (XG(do_profile) == 1) {
		long profile_flag = XDEBUG_PROFILER_LBL;
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &profile_flag) == FAILURE) {
			RETURN_FALSE;
		}
		if (profile_flag < 0 || profile_flag >= XDEBUG_PROFILER_MODES) {
			php_error(E_WARNING, "'%d' is not a valid profiling flag\n", profile_flag);
			RETURN_FALSE;
		}
		print_profile(PG(html_errors), profile_flag TSRMLS_CC);
		RETURN_TRUE;
	} else {
		php_error(E_WARNING, "Function profiling was not started, use xdebug_start_profiling() before calling this function");
		RETURN_FALSE;
	}
}
