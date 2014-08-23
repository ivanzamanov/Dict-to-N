DOT_PATH="/cygdrive/d/Graphviz2.38/bin/dot.exe"

STAT="stat --format=%Y"
MTIME=0

while true
do
    NEW_MTIME=`$STAT $1`
    if [ $NEW_MTIME -gt $MTIME ]; then
	echo Changed
	MTIME=$NEW_MTIME
	$DOT_PATH -Tpng `cygpath -w $1` > `cygpath -w $2`
    fi
    sleep 1
done
