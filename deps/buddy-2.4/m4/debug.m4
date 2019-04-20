AC_DEFUN([buddy_DEBUG_FLAGS],
[AC_ARG_ENABLE([swap-count], 
  [AC_HELP_STRING([--enable-swap-count], 
     [Count number of fundamental variable swaps (for debugging)])])
case $enable_swap_count in
  yes)
    AC_DEFINE([SWAPCOUNT], 1, 
 	      [Define to 1 to count number of fundamental variable swaps 
	       (for debugging).])
    ;;
esac

AC_ARG_ENABLE([cache-stats], 
  [AC_HELP_STRING([--enable-cache-stats], 
     [Gather statistical information about operator and unique node caching (for debugging)])])
case $enable_cache_stats in
  yes)
    AC_DEFINE([CACHESTATS], 1, 
              [Defube to 1 to gather statistical information about operator and unique node caching (for debugging).])
    ;;
esac
])

     
  