./main $1 > "$1.s" \
&& gcc "$1_test.c" "$1.s" -o $1 \
&& ./$1

rm "$1.s"
rm $1
