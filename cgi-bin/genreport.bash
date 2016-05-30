rm -f /var/www/Src$2/*
mkdir /var/www/Src$2
./bmp2jpg.bash $1 /var/www/Src$2/

rm -f /var/www/Report$2/*
mkdir /var/www/Report$2
./bmp2jpg.bash ./Report$2/ /var/www/Report$2/

rm -f /var/www/Mask$2/*
mkdir /var/www/Mask$2
./bmp2jpg.bash ./Mask$2/ /var/www/Mask$2/


cat repsamp.html


echo "  <h1>Отчёт: <h1>" 
echo "<body>"

echo "<h3> Вы так же можете: </h3>"
echo "- <a href = ./cgi-bin/zip_all.bash?$2> Скачать все сгенерированные данные в виде .zip архива </a> <br>"
echo ''
echo "- <a href = ./cgi-bin/zip_rep.bash?$2> Скачать отчёты в виде .zip архива </a> <br>"
echo ''
echo "- <a href = ./cgi-bin/zip_mask.bash?$2> Скачать маски в виде .zip архива </a> <br> <br>"


for filename in `ls $1 | grep bmp` ; do
	echo "<body><div class=\"spoil\">"
	echo -n "<div class=\"smallfont\"><input type=\"button\" value=\""
	echo -n $filename
	echo -n "\" ;=\"\" class=\"input-button\" width = 100px onclick=\"if (this.parentNode.parentNode.getElementsByTagName('div')[1].getElementsByTagName('div')[0].style.display != '') { this.parentNode.parentNode.getElementsByTagName('div')[1].getElementsByTagName('div')[0].style.display = ''; this.innerText = ''; this.value = '"
	echo -n $filename
	echo -n "'; } else { this.parentNode.parentNode.getElementsByTagName('div')[1].getElementsByTagName('div')[0].style.display = 'none'; this.innerText = ''; this.value = '"
	echo -n $filename
	echo "'; }\">"
	echo "</div><div class=\"alt2\"><div style=\"display: none; text-align:center;\">"
	echo "						    	<big> Исходный файл: </big> <br> " 
	echo -n "					    	   <img src=\"./Src$2/"
	echo -n 								$filename | awk -F . '{ORS=""} {print $1}' 
	echo 									".jpg\" width = 50%> <br>" 
	echo "						    	<big> Маска: </big> <br>" 
	echo -n "							    <img src=\"./Mask$2/" 
	echo -n 								$filename | awk -F . '{ORS=""} {print $1}' 
	echo        							"_occlusion.jpg\" width = 50%> <br>"   
	echo "						   	    <big> Отчёт: </big> <br>" 
	echo -n " 							    <img src=\"./Report$2/" 
	echo -n									$filename | awk -F . '{ORS=""} {print $1}' 
	echo    						        "_Rep.jpg\" width = 70% <br>"   
	echo "					</div>"
	echo "				</div>"
	echo "			</div>"
	echo "		</div> <br>"
	done

	echo "</body>"
	echo "</html>"

