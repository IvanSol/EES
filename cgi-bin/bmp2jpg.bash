for i in $1/*.bmp; do
	convert "$i" "$2/`basename $i .bmp`.jpg";
done
