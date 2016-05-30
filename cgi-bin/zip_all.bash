#!/bin/sh
echo "Content-type: text/html"
echo ''
rm -f /var/www/All$1.zip
echo "<h1> Archive is generating. Please wait... </h1>"
echo "<pre>"
zip -r /var/www/All$1.zip [MR][ae][sp][ko]*$1/*
echo "</pre>"
echo ''
echo "<meta http-equiv=\"Refresh\" content=\"1; url=../All$1.zip\">"
