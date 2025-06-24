#!/usr/bin/php-cgi
<?php
session_start();

$username = 'webserv';

$logLine = date('Y-m-d H:i:s') . " - User '$username' accept cookies\n";

$logFile = __DIR__ . '/../Cookies/access.log';

file_put_contents($logFile, $logLine, FILE_APPEND | LOCK_EX);

$cookieName = 'user_cookie_accepted_' . $username;
setcookie($cookieName, 'yes', time() + (30 * 24 * 60 * 60), "/");
header("Location: /Cookies/cookies.html");
exit;
?>
