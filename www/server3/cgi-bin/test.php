#!/usr/bin/php-cgi
<?php

$session_id = null;
$session_dir = __DIR__ . "/../../sessions";

// Lire HTTP_COOKIE
if (isset($_SERVER['HTTP_COOKIE'])) {
    $cookies_raw = $_SERVER['HTTP_COOKIE'];
    $cookies = explode(';', $cookies_raw);

    foreach ($cookies as $cookie) {
        $parts = explode('=', trim($cookie), 2);
        if (count($parts) === 2 && $parts[0] === 'session_id') {
            $session_id = $parts[1];
            break;
        }
    }
}

$session_file = $session_dir . '/' . $session_id;

if ($session_id && file_exists($session_file)) {
    $username = trim(file_get_contents($session_file));

    header("Content-Type: text/html");
    echo "<h1>Bienvenue $username</h1>";
    echo "<p>Session ID : $session_id</p>";
    exit;
}

if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['username'])) {
    $username = trim($_POST['username']);
    if (!is_dir($session_dir)) {
        mkdir($session_dir, 0777, true);
    }

    $session_id = bin2hex(random_bytes(16));
    $session_file = $session_dir . '/' . $session_id;
    file_put_contents($session_file, $username);

    header("Set-Cookie: session_id=$session_id; Path=/");
    header("Content-Type: text/html");
    echo "<h1>Bienvenue $username</h1>";
    echo "<p>Session créée avec succès !</p>";
    echo "<p>Session ID : $session_id</p>";
    exit;
}

// Formulaire initial
header("Content-Type: text/html");
echo "<h1>Créer une session</h1>";
echo "<form method='POST'>";
echo "<label>Entrez votre nom : <input type='text' name='username'></label>";
echo "<input type='submit' value='Envoyer'>";
echo "</form>";
exit;
