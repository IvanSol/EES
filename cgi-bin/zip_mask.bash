#!/bin/sh
echo "Content-type: text/html"
echo ''
rm -f /var/www/Mask$1.zip
echo "<h1> Archive is generating. Please wait... </h1>"
echo "<pre>"
zip -r9 /var/www/Mask$1.zip Mask$1/*
echo "</pre>"
echo ''
echo "<meta http-equiv=\"Refresh\" content=\"1; url=../Mask$1.zip\">"
