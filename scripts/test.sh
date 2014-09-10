head -n $1 genom1-numbers.txt > small.txt 
#tail -n $1 genom1-numbers.txt > small.txt 
./dict small.txt 1>output
bash compare.sh small.txt output 
