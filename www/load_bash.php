<?php
#shell_exec("sudo /usr/lib/cgi-bin/df.bash" . " " . $time);
#shell_exec("sudo touch abcd");
#"./cgi-bin/df.bash?
header("Location: cgi-bin/df.bash?" . $time);
die();
?>
