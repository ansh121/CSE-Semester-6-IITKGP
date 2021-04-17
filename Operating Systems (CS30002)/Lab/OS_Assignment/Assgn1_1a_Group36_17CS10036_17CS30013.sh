num1=$1
op=$2
num2=$3
ans=1
case "$op" in
"+")ans=$((num1 + num2))
;;
"-")ans=$((num1 - num2))
;;
"*")ans=$((num1 * num2))
;;
"/")ans=$((num1 / num2))
;;
"%")ans=$((num1 % num2))
;;
esac
echo "$ans"