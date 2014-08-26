DOT_PATH=dot

STAT="stat --format=%Y"
MTIME=0

while true
do
    NEW_MTIME=`$STAT $1`
    if [ $NEW_MTIME -gt $MTIME ]; then
	echo Changed
	MTIME=$NEW_MTIME
	$DOT_PATH -Tpng "$1" > "$2"
    fi
    sleep 1
done
