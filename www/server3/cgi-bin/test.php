#!/usr/bin/php
<?php
setcookie('user_id', '1234');
echo "Content-Type: text/html\r\n\r\n";
?>
<!DOCTYPE html>
<html>
<head>
	<title>Test CGI PHP</title>
</head>
<body>
	<h1>Hello from PHP CGI!</h1>
	<p>Script has been successfuly executed.</p>
</body>
</html>
