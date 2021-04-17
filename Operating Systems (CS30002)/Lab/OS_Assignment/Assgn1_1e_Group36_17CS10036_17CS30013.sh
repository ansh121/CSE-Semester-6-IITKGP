index=$2
index=$((index - 1))
while read -a line; do (echo -e "${line[$index]}" | tr '[:upper:]' '[:lower:]'); done < $1 > "go.txt"
(sort -r | uniq -c) < "go.txt"  > "with_frequency.txt"
awk '{print $2,$1}' "with_frequency.txt" > "result.freq"
(sort -k 2nr result.freq) > "1e_output_$index.freq"
rm "result.freq"
rm "with_frequency.txt"
rm "go.txt"