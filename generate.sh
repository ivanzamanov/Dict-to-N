c=0
for ((i=0; i<100000000; i++))
do
	c=$((c+1))
	if [ 0 -eq `expr $c % 1000` ]; then
		echo $c
	fi
	echo `uuidgen` " " $RANDOM >> data.txt
done
