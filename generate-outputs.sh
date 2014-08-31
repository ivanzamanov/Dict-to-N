c=0
for ((i=0; i<1113056; i++))
do
	c=$((c+1))
	if [ 0 -eq `expr $c % 1000` ]; then
		echo $c
	fi
	echo `printf %05d $i` >> outputs.txt
done
