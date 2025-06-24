#!/usr/bin/php-cgi
<?php
session_start();

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Récupère le nom utilisateur envoyé par formulaire
    $username = trim($_POST['username'] ?? '');

    if ($username !== '') {
        $_SESSION['username'] = htmlspecialchars($username, ENT_QUOTES, 'UTF-8');
        error_log("Session ID: " . session_id());
        // Log de la connexion
        $logLine = date('Y-m-d H:i:s') . " - User '{$_SESSION['username']}' logged in\n";
        $logFile = __DIR__ . '/../Cookies/access.log';
        file_put_contents($logFile, $logLine, FILE_APPEND | LOCK_EX);

        header("Location: /cgi-bin/account.php");
        exit;
    } else {
        $error = "Le nom d'utilisateur ne peut pas être vide.";
    }
}
?>
<!DOCTYPE html>
<html lang="fr">
<head>
	<meta charset="UTF-8" />
	<meta name="viewport" content="width=device-width, initial-scale=1" />
	<title>Connexion 🍪</title>
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
		form {
			background-color: rgba(0,0,0,0.5);
			padding: 20px;
			border-radius: 10px;
			display: inline-block;
		}
		input[type="text"] {
			font-size: 18px;
			padding: 8px;
			border-radius: 5px;
			border: none;
			margin-bottom: 10px;
			width: 250px;
		}
		button {
			background-color: chocolate;
			border: none;
			color: white;
			padding: 10px 20px;
			font-size: 16px;
			border-radius: 8px;
			cursor: pointer;
			transition: background-color 0.3s;
		}
		button:hover {
			background-color: #a0522d;
		}
		.error {
			color: #f88;
			margin-bottom: 10px;
		}
	</style>
</head>
<body>
	<h1>Connexion 🍪</h1>
	<?php if (!empty($error)): ?>
		<p class="error"><?= $error ?></p>
	<?php endif; ?>
	<form method="post" action="/cgi-bin/login.php">
		<input type="text" name="username" placeholder="Votre nom d'utilisateur" required autofocus />
		<br />
		<button type="submit">Se connecter</button>
	</form>
</body>
</html>
