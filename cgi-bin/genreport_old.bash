cp -r ./Src /var/www
cp -r ./Report /var/www
cp -r ./Mask /var/www

echo "<html> <body>" 
echo "<style type="text/css">" 
echo "  <h1>Отчёт: <h1>" 
echo "<table class="brd">" 
echo "  <tr>" 
echo "    <th>Исходный файл</th>" 
echo "    <th>Маска</th>" 
echo "    <th>Отчёт</th>" 
echo "  </tr>" 

for filename in `ls ./Src` ; do
		echo "  <tr>" 

		echo -n "    <td> <img src=\"./Src/"
		echo -n $filename
		echo  "\"> </td>" 

	    echo -n "    <td> <img src=\"./Mask/" 
		echo -n $filename | awk -F . '{ORS=""} {print $1}' 
	    echo         "_occlusion.bmp\"> </td>"   

	    echo -n "    <td> <img src=\"./Report/" 
	    echo -n $filename | awk -F . '{ORS=""} {print $1}' 
	    echo           "_Rep.bmp\"> </td>"   

		echo "  </tr>" 
done

echo "</table>" 
echo "</html> </body>" 
