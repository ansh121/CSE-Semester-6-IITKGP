SENT="SERIAL	RANDOM_STRING"
echo "$SENT" > "output.txt"
COUNT=1

while IFS= read -r line
do
	SENT="$COUNT	$line"
	echo "$SENT" >> "output.txt"
	TEMP=`expr $COUNT + 1`
	COUNT=$TEMP
done < 1b_input.txt