#!/bin/sh
# mkinstalldirs --- make directory hierarchy
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-05-16
# Last modified: 1994-03-25
# Public domain
#
# Header: /usr2/foxharp/src/pgf/vile/RCS/mkdirs.sh,v 1.2 1994/07/11 22:56:20 pgf Exp
#

errstatus=0

for file in ${1+"$@"} ; do 
   set fnord `echo ":$file" | sed -ne 's/^:\//#/;s/^://;s/\// /g;s/^#/\//;p'`
   shift

   pathcomp=
   for d in ${1+"$@"} ; do
     pathcomp="$pathcomp$d"
     case "$pathcomp" in
       -* ) pathcomp=./$pathcomp ;;
     esac

     if test ! -d "$pathcomp"; then
        echo "mkdir $pathcomp" 1>&2
        mkdir "$pathcomp" || errstatus=$?
     fi

     pathcomp="$pathcomp/"
   done
done

exit $errstatus

# mkinstalldirs ends here
