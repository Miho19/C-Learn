#!/bin/bash

# program to be tested
#TESTPROGS="selection-sort"
TESTPROGS="mergesort mmergesort"

# types of data sets to test
DATASETS="random sorted partially_sorted reversed"
#DATASETS="random"

# maximum number of items to test
MAXITEMS=2000000

# start with this many items, then increase by this
# many until you reach $MAXITEMS
INC=200000

# we can use system programs to get data into the order we wish to test:
#   sort -n  for numerically sorted data
#   sort -rn for reversed numerically sorted data
#   cat      for random data (has no effect on the order)
#   sort     for partially sorted data (sorts alphabetically)
WHICHSORT () {
   if [ $1 = "sorted" ]; then
      echo "sort -n"
   elif [ $1 = "partially_sorted" ]; then
      echo "sort"
   elif [ $1 = "random" ]; then
      echo "cat"
   elif [ $1 = "reversed" ]; then
      echo "sort -rn"
   else
      echo "cat"
   fi
}

if [ ! -x printrandom ]; then
   echo "Can't find executable 'printrandom'"
   exit 1
else
   echo -n "Making file of $MAXITEMS numbers ... "
   ./printrandom $MAXITEMS > randnums
   echo "done!"
fi

# create gnuplot script
rm -f plotscript
echo set key left >> plotscript
echo set term postscript color solid >> plotscript
echo set title \"Sort comparison - $(whoami)\" >> plotscript
echo set xlabel \"Number of items\" >> plotscript
echo set ylabel \"Time\" >> plotscript
printf "plot " >> plotscript

# run the tests
for PROG in $TESTPROGS; do
   # make sure the test program exists and produces sensible output
   if [ ! -x ./$PROG ]; then
      echo $PROG does not exist or is not executable
      exit 1
   elif [ `./$PROG < /dev/null 2>&1 | wc | awk '{print $1$2}'` -ne 12 ]; then
      echo $PROG does not produce 2 numbers on stderr
      exit 1
   fi
   # program does something vaguely sensible, so run the test
   echo creating output files for $PROG
   for s in $DATASETS; do
      echo "\"$PROG.$s\" with linespoints, \\" >> plotscript
      rm -f $PROG.$s
#      for NITEMS in `seq -f %.0f 0 $INC $MAXITEMS`; do
      for ((NITEMS = $INC; $NITEMS <= $MAXITEMS; NITEMS = $NITEMS + $INC)); do
         head -n $NITEMS randnums | `WHICHSORT $s` | \
         ./$PROG > /dev/null 2>> $PROG.$s
         printf "creating $PROG.$s $NITEMS\r"
      done
      echo
   done
done

# horrible hack to get rid of last ", \" in plotscript
sed '$ s/, \\//' < plotscript > plotscript.new
mv -f plotscript.new plotscript

# create and view a nice postscript graph
gnuplot plotscript > graph.ps && ps2pdf graph.ps graph.pdf && \
rm -f graph.ps

if [ -f graph.pdf ]; then
   if [ -x /bin/atril ]; then
      atril graph.pdf 2>/dev/null &
   else
      open graph.pdf
   fi
fi
