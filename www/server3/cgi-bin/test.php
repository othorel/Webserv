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

function html_header($title = "Session") {
    echo <<<HTML
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <title>$title</title>
    <link href="https://fonts.googleapis.com/css2?family=Creepster&display=swap" rel="stylesheet" />
    <link href="https://fonts.googleapis.com/css2?family=IM+Fell+English+SC&display=swap" rel="stylesheet" />
    <style>
        body {
            font-family: 'IM Fell English SC', serif;
            background-image: url('/img/background.jpg');
            background-size: cover;
            background-position: center;
            background-repeat: no-repeat;
            text-align: center;
            padding: 40px;
            margin: 0;
            min-height: 100vh;
            color: #fff;
        }
        h1 {
            font-family: 'Creepster', cursive;
            color: chocolate;
            font-size: 80px;
            text-shadow: 2px 2px 6px #000, 0 10px 10px rgba(210, 105, 30, 0.5);
            margin-bottom: 30px;
        }
        p, label {
            font-size: 20px;
            background-color: rgba(0,0,0,0.5);
            padding: 15px 20px;
            display: inline-block;
            border-radius: 10px;
            margin-bottom: 20px;
        }
        input[type="text"] {
            font-size: 18px;
            padding: 8px;
            border-radius: 6px;
            border: none;
            margin-left: 10px;
        }
        input[type="submit"] {
            background-color: chocolate;
            border: none;
            color: white;
            padding: 10px 20px;
            font-size: 18px;
            border-radius: 8px;
            cursor: pointer;
            transition: background-color 0.3s;
        }
        input[type="submit"]:hover {
            background-color: #a0522d;
        }
        .soft-frame {
            border-radius: 15px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.4);
            display: block;
            margin: 30px auto;
            max-width: 400px;
            width: 90%;
            transition: transform 0.3s ease;
        }
        .soft-frame:hover {
            transform: scale(1.05);
        }
        .homepage-button img {
            width: 180px;
            border-radius: 8px;
            margin-top: 40px;
            transition: transform 0.3s ease, box-shadow 0.3s ease;
        }
        .homepage-button:hover img {
            transform: scale(1.1);
        }
    </style>
</head>
<body>
    <h1>Cookies 🍪</h1>
HTML;
}

function html_footer() {
    echo <<<HTML
    <a href="/" class="homepage-button">
        <img src="/img/back_home.png" alt="Home" />
    </a>
</body>
</html>
HTML;
}

// Case 1: existing session
if ($session_id && file_exists($session_file)) {
    $username = trim(file_get_contents($session_file));

    header("Content-Type: text/html");
    html_header("Bienvenue");
    echo "<p>Welcome <strong>$username</strong></p>";
    echo "<br />\n";
    echo "<p>You are already logged in.</p>";
    echo "<br />\n";
    echo "<p>Session ID : $session_id</p>";
    echo "<br />\n";
    html_footer();
    exit;
}

// Case 2: sunmitted form
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
    html_header("Session créée");
    echo "<p>Welcome <strong>$username</strong> !</p>";
    echo "<br />\n";
    echo "<p>Session ID : $session_id</p>";
    echo "<br />\n";
    html_footer();
    exit;
}

// Case 3: form
header("Content-Type: text/html");
html_header("Créer une session");
echo <<<HTML
    <form method="POST">
        <label>Enter your name :
            <input type="text" name="username" required />
        </label><br><br>
        <input type="submit" value="Send">
    </form>
HTML;
html_footer();
exit;
