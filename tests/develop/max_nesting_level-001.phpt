--TEST--
Test for xdebug.max_nesting_level (default) [1]
--INI--
xdebug.max_stack_frames=-1
xdebug.mode=develop
xdebug.show_local_vars=0
xdebug.max_nesting_level=128
--FILE--
<?php
function foo($a)
{
	foo($a+1);
}
foo(0);
?>
--EXPECTF--
Fatal error: Uncaught Error: Xdebug has detected a possible infinite loop, and aborted your script with a stack depth of '128' frames in %smax_nesting_level-001.php on line 2

Error: Xdebug has detected a possible infinite loop, and aborted your script with a stack depth of '128' frames in %smax_nesting_level-001.php on line 2

Call Stack:
%w%f %w%d   1. {main}() %smax_nesting_level-001.php:0
%w%f %w%d   2. foo($a = 0) %smax_nesting_level-001.php:6
%w%f %w%d   3. foo($a = 1) %smax_nesting_level-001.php:4
%w%f %w%d   4. foo($a = 2) %smax_nesting_level-001.php:4
%w%f %w%d   5. foo($a = 3) %smax_nesting_level-001.php:4
%w%f %w%d   6. foo($a = 4) %smax_nesting_level-001.php:4
%w%f %w%d   7. foo($a = 5) %smax_nesting_level-001.php:4
%w%f %w%d   8. foo($a = 6) %smax_nesting_level-001.php:4
%w%f %w%d   9. foo($a = 7) %smax_nesting_level-001.php:4
%w%f %w%d  10. foo($a = 8) %smax_nesting_level-001.php:4
%w%f %w%d  11. foo($a = 9) %smax_nesting_level-001.php:4
%w%f %w%d  12. foo($a = 10) %smax_nesting_level-001.php:4
%w%f %w%d  13. foo($a = 11) %smax_nesting_level-001.php:4
%w%f %w%d  14. foo($a = 12) %smax_nesting_level-001.php:4
%w%f %w%d  15. foo($a = 13) %smax_nesting_level-001.php:4
%w%f %w%d  16. foo($a = 14) %smax_nesting_level-001.php:4
%w%f %w%d  17. foo($a = 15) %smax_nesting_level-001.php:4
%w%f %w%d  18. foo($a = 16) %smax_nesting_level-001.php:4
%w%f %w%d  19. foo($a = 17) %smax_nesting_level-001.php:4
%w%f %w%d  20. foo($a = 18) %smax_nesting_level-001.php:4
%w%f %w%d  21. foo($a = 19) %smax_nesting_level-001.php:4
%w%f %w%d  22. foo($a = 20) %smax_nesting_level-001.php:4
%w%f %w%d  23. foo($a = 21) %smax_nesting_level-001.php:4
%w%f %w%d  24. foo($a = 22) %smax_nesting_level-001.php:4
%w%f %w%d  25. foo($a = 23) %smax_nesting_level-001.php:4
%w%f %w%d  26. foo($a = 24) %smax_nesting_level-001.php:4
%w%f %w%d  27. foo($a = 25) %smax_nesting_level-001.php:4
%w%f %w%d  28. foo($a = 26) %smax_nesting_level-001.php:4
%w%f %w%d  29. foo($a = 27) %smax_nesting_level-001.php:4
%w%f %w%d  30. foo($a = 28) %smax_nesting_level-001.php:4
%w%f %w%d  31. foo($a = 29) %smax_nesting_level-001.php:4
%w%f %w%d  32. foo($a = 30) %smax_nesting_level-001.php:4
%w%f %w%d  33. foo($a = 31) %smax_nesting_level-001.php:4
%w%f %w%d  34. foo($a = 32) %smax_nesting_level-001.php:4
%w%f %w%d  35. foo($a = 33) %smax_nesting_level-001.php:4
%w%f %w%d  36. foo($a = 34) %smax_nesting_level-001.php:4
%w%f %w%d  37. foo($a = 35) %smax_nesting_level-001.php:4
%w%f %w%d  38. foo($a = 36) %smax_nesting_level-001.php:4
%w%f %w%d  39. foo($a = 37) %smax_nesting_level-001.php:4
%w%f %w%d  40. foo($a = 38) %smax_nesting_level-001.php:4
%w%f %w%d  41. foo($a = 39) %smax_nesting_level-001.php:4
%w%f %w%d  42. foo($a = 40) %smax_nesting_level-001.php:4
%w%f %w%d  43. foo($a = 41) %smax_nesting_level-001.php:4
%w%f %w%d  44. foo($a = 42) %smax_nesting_level-001.php:4
%w%f %w%d  45. foo($a = 43) %smax_nesting_level-001.php:4
%w%f %w%d  46. foo($a = 44) %smax_nesting_level-001.php:4
%w%f %w%d  47. foo($a = 45) %smax_nesting_level-001.php:4
%w%f %w%d  48. foo($a = 46) %smax_nesting_level-001.php:4
%w%f %w%d  49. foo($a = 47) %smax_nesting_level-001.php:4
%w%f %w%d  50. foo($a = 48) %smax_nesting_level-001.php:4
%w%f %w%d  51. foo($a = 49) %smax_nesting_level-001.php:4
%w%f %w%d  52. foo($a = 50) %smax_nesting_level-001.php:4
%w%f %w%d  53. foo($a = 51) %smax_nesting_level-001.php:4
%w%f %w%d  54. foo($a = 52) %smax_nesting_level-001.php:4
%w%f %w%d  55. foo($a = 53) %smax_nesting_level-001.php:4
%w%f %w%d  56. foo($a = 54) %smax_nesting_level-001.php:4
%w%f %w%d  57. foo($a = 55) %smax_nesting_level-001.php:4
%w%f %w%d  58. foo($a = 56) %smax_nesting_level-001.php:4
%w%f %w%d  59. foo($a = 57) %smax_nesting_level-001.php:4
%w%f %w%d  60. foo($a = 58) %smax_nesting_level-001.php:4
%w%f %w%d  61. foo($a = 59) %smax_nesting_level-001.php:4
%w%f %w%d  62. foo($a = 60) %smax_nesting_level-001.php:4
%w%f %w%d  63. foo($a = 61) %smax_nesting_level-001.php:4
%w%f %w%d  64. foo($a = 62) %smax_nesting_level-001.php:4
%w%f %w%d  65. foo($a = 63) %smax_nesting_level-001.php:4
%w%f %w%d  66. foo($a = 64) %smax_nesting_level-001.php:4
%w%f %w%d  67. foo($a = 65) %smax_nesting_level-001.php:4
%w%f %w%d  68. foo($a = 66) %smax_nesting_level-001.php:4
%w%f %w%d  69. foo($a = 67) %smax_nesting_level-001.php:4
%w%f %w%d  70. foo($a = 68) %smax_nesting_level-001.php:4
%w%f %w%d  71. foo($a = 69) %smax_nesting_level-001.php:4
%w%f %w%d  72. foo($a = 70) %smax_nesting_level-001.php:4
%w%f %w%d  73. foo($a = 71) %smax_nesting_level-001.php:4
%w%f %w%d  74. foo($a = 72) %smax_nesting_level-001.php:4
%w%f %w%d  75. foo($a = 73) %smax_nesting_level-001.php:4
%w%f %w%d  76. foo($a = 74) %smax_nesting_level-001.php:4
%w%f %w%d  77. foo($a = 75) %smax_nesting_level-001.php:4
%w%f %w%d  78. foo($a = 76) %smax_nesting_level-001.php:4
%w%f %w%d  79. foo($a = 77) %smax_nesting_level-001.php:4
%w%f %w%d  80. foo($a = 78) %smax_nesting_level-001.php:4
%w%f %w%d  81. foo($a = 79) %smax_nesting_level-001.php:4
%w%f %w%d  82. foo($a = 80) %smax_nesting_level-001.php:4
%w%f %w%d  83. foo($a = 81) %smax_nesting_level-001.php:4
%w%f %w%d  84. foo($a = 82) %smax_nesting_level-001.php:4
%w%f %w%d  85. foo($a = 83) %smax_nesting_level-001.php:4
%w%f %w%d  86. foo($a = 84) %smax_nesting_level-001.php:4
%w%f %w%d  87. foo($a = 85) %smax_nesting_level-001.php:4
%w%f %w%d  88. foo($a = 86) %smax_nesting_level-001.php:4
%w%f %w%d  89. foo($a = 87) %smax_nesting_level-001.php:4
%w%f %w%d  90. foo($a = 88) %smax_nesting_level-001.php:4
%w%f %w%d  91. foo($a = 89) %smax_nesting_level-001.php:4
%w%f %w%d  92. foo($a = 90) %smax_nesting_level-001.php:4
%w%f %w%d  93. foo($a = 91) %smax_nesting_level-001.php:4
%w%f %w%d  94. foo($a = 92) %smax_nesting_level-001.php:4
%w%f %w%d  95. foo($a = 93) %smax_nesting_level-001.php:4
%w%f %w%d  96. foo($a = 94) %smax_nesting_level-001.php:4
%w%f %w%d  97. foo($a = 95) %smax_nesting_level-001.php:4
%w%f %w%d  98. foo($a = 96) %smax_nesting_level-001.php:4
%w%f %w%d  99. foo($a = 97) %smax_nesting_level-001.php:4
%w%f %w%d 100. foo($a = 98) %smax_nesting_level-001.php:4
%w%f %w%d 101. foo($a = 99) %smax_nesting_level-001.php:4
%w%f %w%d 102. foo($a = 100) %smax_nesting_level-001.php:4
%w%f %w%d 103. foo($a = 101) %smax_nesting_level-001.php:4
%w%f %w%d 104. foo($a = 102) %smax_nesting_level-001.php:4
%w%f %w%d 105. foo($a = 103) %smax_nesting_level-001.php:4
%w%f %w%d 106. foo($a = 104) %smax_nesting_level-001.php:4
%w%f %w%d 107. foo($a = 105) %smax_nesting_level-001.php:4
%w%f %w%d 108. foo($a = 106) %smax_nesting_level-001.php:4
%w%f %w%d 109. foo($a = 107) %smax_nesting_level-001.php:4
%w%f %w%d 110. foo($a = 108) %smax_nesting_level-001.php:4
%w%f %w%d 111. foo($a = 109) %smax_nesting_level-001.php:4
%w%f %w%d 112. foo($a = 110) %smax_nesting_level-001.php:4
%w%f %w%d 113. foo($a = 111) %smax_nesting_level-001.php:4
%w%f %w%d 114. foo($a = 112) %smax_nesting_level-001.php:4
%w%f %w%d 115. foo($a = 113) %smax_nesting_level-001.php:4
%w%f %w%d 116. foo($a = 114) %smax_nesting_level-001.php:4
%w%f %w%d 117. foo($a = 115) %smax_nesting_level-001.php:4
%w%f %w%d 118. foo($a = 116) %smax_nesting_level-001.php:4
%w%f %w%d 119. foo($a = 117) %smax_nesting_level-001.php:4
%w%f %w%d 120. foo($a = 118) %smax_nesting_level-001.php:4
%w%f %w%d 121. foo($a = 119) %smax_nesting_level-001.php:4
%w%f %w%d 122. foo($a = 120) %smax_nesting_level-001.php:4
%w%f %w%d 123. foo($a = 121) %smax_nesting_level-001.php:4
%w%f %w%d 124. foo($a = 122) %smax_nesting_level-001.php:4
%w%f %w%d 125. foo($a = 123) %smax_nesting_level-001.php:4
%w%f %w%d 126. foo($a = 124) %smax_nesting_level-001.php:4
%w%f %w%d 127. foo($a = 125) %smax_nesting_level-001.php:4
%w%f %w%d 128. foo($a = 126) %smax_nesting_level-001.php:4
