#!/bin/sh
if [ $# -eq "2" ]
then
		R="r$2"
		RS="rs$2"
		echo "run cuda:"
		./run.sh $1 $2 > $R
		echo "run serial:"
		./run_serial.sh $1 $2 > $RS
		diff $R $RS
else
		echo "1: steps, 2: points"
fi
exit 0

