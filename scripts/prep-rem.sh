set -x
COUNT=$1
REMOVE=$2
head -n $COUNT $3 > all.txt
head -n $REMOVE all.txt > to-remove.txt
tail -n `expr $COUNT - $REMOVE` all.txt > removed.txt
