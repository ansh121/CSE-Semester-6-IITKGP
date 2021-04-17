mkdir "1.d.files.out"
for file in "1.d.files"/*
do
	sort -nr $file > "1.d.files.out/$(basename "$file")"
	echo "This File is sorted in reverse order $(basename "$file")"
done
cd "1.d.files.out"
sort -m -r *.txt > "../1.d.out.txt"