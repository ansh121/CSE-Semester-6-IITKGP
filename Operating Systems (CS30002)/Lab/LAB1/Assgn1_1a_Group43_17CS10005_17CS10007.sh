if [ $2 == "+" ]
then
	ANS=`expr $1 + $3`
elif [ $2 == "-" ]
then
	ANS=`expr $1 - $3`	
elif [ $2 == "/" ]
then
	if [ $3 == 0 ]
	then
		ANS="Error!! Divide by Zero"
	else
		ANS=`expr $1 / $3`
	fi
elif [ $2 == "%" ]
then
	ANS=`expr $1 % $3`
else
	ANS=`expr $1 \* $3`
fi

echo "$1 $2 $3 = $ANS"