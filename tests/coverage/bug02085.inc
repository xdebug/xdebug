<?php

// To get this test to fail before the fix, this file must be
// encoded using the Source Guardian encoder and source guardian
// loaders must be installed. https://www.sourceguardian.com/loaders.html
//
// That extension somehow sets the zend_op_array->filename member for encoded files to NULL
// So, when xdebug filtering is enabled, the xdebug_filter_match_path_include function
// will try to see if the zend_op_array->filename matches a filter
// But since zend_op_array->filename is NULL, we get a segmentation fault
//
// This fix will guard against the possibility of other extensions that may set
// the zend_op_array->filename to NULL since it is absolutely possible that
// the zend_op_array->filename could be NULL when checking against filter paths

echo "I'm encoded";
