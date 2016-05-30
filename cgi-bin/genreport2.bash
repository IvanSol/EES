cp -r ./Src /var/www
cp -r ./Report /var/www
cp -r ./Mask /var/www

cat repsamp.html


echo "  <h1>Отчёт: <h1>" 
echo "<body>"

for filename in `ls ./Src` ; do
echo "		<div id=\"wrap\" style=\"padding:20px 40px;width:80%;margin:0 auto;background-color:#fff;height:80px\">"

echo "			<div class=\"spoil\">"
echo "				<div class=\"smallfont\"><input type=\"button\" value=\"Спойлер\"; class=\"input-button\" onclick=\"if (this.parentNode.parentNode.getElementsByTagName('div')[1].getElementsByTagName('div')[0].style.display != '') { this.parentNode.parentNode.getElementsByTagName('div')[1].getElementsByTagName('div')[0].style.display = ''; this.innerText = ''; this.value = 'Свернуть'; } else { this.parentNode.parentNode.getElementsByTagName('div')[1].getElementsByTagName('div')[0].style.display = 'none'; this.innerText = ''; this.value = 'Спойлер'; }\">"
echo "					</div><div class=\"alt2\"><div style=\"display: none; text-align:center;\">"

echo "					<blockquote class=\"Untext\">"
echo "							<table class="brd">" 
echo " 							<tr>" 
echo "						    	<th>Исходный файл</th>" 
echo "						    	<th>Маска</th>" 
echo "						    	<th>Отчёт</th>" 
echo "							</tr>" 
echo "							<tr>" 
echo -n "					    	   <td> <img src=\"./Src/"
echo -n								   $filename
echo 									"\" width = 30%> </td>" 
echo -n "							   <td> <img src=\"./Mask/" 
echo -n 								$filename | awk -F . '{ORS=""} {print $1}' 
echo        							"_occlusion.bmp\" width = 30%> </td>"   
echo -n " 							   <td> <img src=\"./Report/" 
echo -n $filename | awk -F . '{ORS=""} {print $1}' 
echo    						       "_Rep.bmp\" width = 40% </td>"   
echo "							</tr>" 
echo "							</table>" 
echo "					</blockquote>"

echo "					</div>"
echo "				</div>"
echo "			</div> "
echo "		</div> "
done

echo "</body>"
echo "</html>"
