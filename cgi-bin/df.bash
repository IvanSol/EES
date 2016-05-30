#!/bin/bash
echo "Content-type: text/html"
#POST=$(</dev/stdin)
#echo "$POST" > /images/file
rm -f ./Report$1/*
rm -f ./Mask$1/*
echo ''
echo "<h2> Отчёт сгенерирован. Вы можете: </h2>"
echo ''
echo "- <a href = \"../rep$1.html\"> Посмотреть отчёт в виде .html страницы </a> <br>"
echo ''
echo "- <a href = ./zip_all.bash $1> Скачать все сгенерированные данные в виде .zip архива </a> <br>"
echo ''
echo "- <a href = ./zip_rep.bash $1> Скачать отчёты в виде .zip архива </a> <br>"
echo ''
echo "- <a href = ./zip_mask.bash $1> Скачать маски в виде .zip архива </a> <br> <br>"
echo ''
echo "<h2> Вывод программы: </h2>"
echo ''
echo '<pre>'
echo ''
DIR="./Src_usr$1/"
BASENAME="User_image"
if [ `ls -1A $DIR | wc -l` -eq 0 ]; then
	DIR="./Src/"
	BASENAME="CASIA"
fi
cp -in ./Src/data ./Src_usr$1/

ls $DIR | grep .bmp | sort > process
mkdir Mask$1
mkdir Report$1
./runEES process $DIR $BASENAME $1
echo ''
echo '</pre>'
./genreport.bash $DIR $1> /var/www/rep$1.html
rm -f ./Src_usr$1/*
echo ''
echo ''

