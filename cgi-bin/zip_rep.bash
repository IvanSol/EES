#!/bin/sh
echo "Content-type: text/html"
echo ''
rm -f /var/www/Report$1.zip
echo "<h1> Archive is generating. Please wait... </h1>"
echo "<pre>"
zip -r /var/www/Report$1.zip Report$1/*
echo "</pre>"
echo ''
echo "<meta http-equiv=\"Refresh\" content=\"1; url=../Report$1.zip\">"
