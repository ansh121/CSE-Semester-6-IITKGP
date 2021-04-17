array=0
gcd=0
for array in $*
do
	while [ $array -ne 0 ]
	do
		remainder=$array
		array=$(( gcd % array ))
		gcd=$remainder
	done
done
echo "$gcd"
