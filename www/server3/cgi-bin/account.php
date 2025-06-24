#!/usr/bin/php-cgi
<?php
session_start();

$username = $_SESSION['username'] ?? null;

?>
<!DOCTYPE html>
<html lang="fr">
<head>
	<meta charset="UTF-8" />
	<meta name="viewport" content="width=device-width, initial-scale=1" />
	<title>Mon Compte 🍪</title>
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
		.info-box {
			background-color: rgba(0,0,0,0.5);
			padding: 30px;
			border-radius: 15px;
			display: inline-block;
			font-size: 24px;
			max-width: 400px;
			color: #ffd700;
		}
		a.button {
			display: inline-block;
			margin-top: 30px;
			padding: 10px 20px;
			background-color: chocolate;
			color: white;
			text-decoration: none;
			border-radius: 8px;
			font-size: 18px;
			transition: background-color 0.3s;
		}
		a.button:hover {
			background-color: #a0522d;
		}
	</style>
</head>
<body>
	<h1>Mon Compte 🍪</h1>
	<?php if ($username): ?>
		<div class="info-box">
			<p>Bienvenue, <strong><?= $username ?></strong> !</p>
			<p>Votre session est active et votre nom est en mémoire.</p>
		</div>
	<?php else: ?>
		<div class="info-box" style="color:#f88;">
			<p>Vous n'êtes pas connecté.</p>
			<p><a href="/cgi-bin/login.php" class="button">Se connecter</a></p>
		</div>
	<?php endif; ?>
</body>
</html>
