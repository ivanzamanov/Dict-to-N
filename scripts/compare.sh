sort < $1 > /tmp/input1
sort < $2 > /tmp/input2
diff /tmp/input1 /tmp/input2

rm /tmp/input1
rm /tmp/input2
