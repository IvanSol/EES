<?php
$time = time();
$target_dir = "/usr/lib/cgi-bin/Src_usr" . $time . "/";
if (!is_dir($target_dir)) {
	mkdir($target_dir);         
}
$uploadOk = 1;
array_map('unlink', glob($target_dir."/*"));
$total = count($_FILES['fileToUpload']['name']);
if($total>0 && $_FILES['fileToUpload']['name'][0] !="") {
for($i=0; $i<$total; $i++) {
$target_file = $target_dir . basename($_FILES["fileToUpload"]["name"][$i]);
$fileName = basename($_FILES["fileToUpload"]["name"][$i]);
// Check if $uploadOk is set to 0 by an error
if ($uploadOk == 0) {
	echo "File " . $fileName . " was not uploaded.<br/>";
	// if everything is ok, try to upload file
} else {
	if (move_uploaded_file($_FILES["fileToUpload"]["tmp_name"][$i], $target_file)) {
		echo "The file ". basename( $_FILES["fileToUpload"]["name"][$i]). " has been uploaded to" . $target_dir . "<br/>";
	} else {
		echo "Sorry, there was an error uploading your file " . $fileName . ".<br/>";
	}
}
}
}
header("Location: cgi-bin/df.bash?" . $time);
die();
?>
