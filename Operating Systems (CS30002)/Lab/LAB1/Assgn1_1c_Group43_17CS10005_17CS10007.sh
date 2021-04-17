function gcd(){
	if (( $1==0 ))
	then
		echo "$2"
	else
		temp=`expr $2 % $1`
		temp2=$(gcd $temp $1)
		echo "$temp2"
	fi
}

GCD=0

for NUM in $*
do
	temp3=$(gcd $GCD $NUM)
	GCD=$temp3
done

echo "GCD is $GCD"