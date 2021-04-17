awk '{if ($1 == $2){}else if ($1 < $2){print $1, $2} else{print $2,$1}}' < 1f.graph.edgelist > 1f.graph.no_slf.edgelist
(sort 1f.graph.no_slf.edgelist | uniq -u) > 1f.graph.output.edgelist
awk ' { t = $1; $1 = $2; $2 = t; print; } ' 1f.graph.output.edgelist > x.txt    
cat 1f.graph.output.edgelist >> x.txt
awk -F" " '{print $1}' x.txt > y.txt
tr ' ' '\12' <y.txt| sort | uniq -c | sort -nr > x.txt          
awk -F $' ' ' { t = $1; $1 = $2; $2 = t; print; } ' OFS=$' ' x.txt >> ANS.txt    # Node # Degree
head --lines=5 ANS.txt > temp.txt 
(echo "Top 5 nodes in desc. order of degree (Node Degree)" && cat temp.txt) > 1f.graph.nodes.degree.txt
rm x.txt y.txt temp.txt ANS.txt 1f.graph.no_slf.edgelist    