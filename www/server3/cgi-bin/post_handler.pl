#!/usr/bin/perl

use strict;
use warnings;
use CGI;

# Create a new CGI object to handle incoming parameters
my $cgi = CGI->new;

# Retrieve the 'speech' field from the POST form, or set a default message
my $speech = $cgi->param('speech') || "Nothing was said.";

# Send the HTTP header with content type text/html
print $cgi->header('text/html');

# Start of the HTML response with matching style and fonts
print <<HTML;
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Perl CGI Response</title>
  <!-- Google Fonts -->
  <link href="https://fonts.googleapis.com/css2?family=Creepster&display=swap" rel="stylesheet">
  <link href="https://fonts.googleapis.com/css2?family=IM+Fell+English+SC&display=swap" rel="stylesheet">

  <!-- Inline CSS to match the original form page -->
  <style>
    body {
      font-family: 'IM Fell English SC', sans-serif;
      background-image: url('/img/background.jpg');
      background-size: cover;
      background-position: center;
      background-repeat: no-repeat;
      text-align: center;
      padding: 40px;
      margin: 0;
      min-height: 100vh;
      color: white;
    }

    h2 {
      font-family: 'Creepster', cursive;
      color: darkred;
      font-size: 70px;
      text-shadow: 2px 2px 4px black, 0 10px 10px rgba(139, 0, 0, 0.6);
      margin-bottom: 30px;
    }

    p {
      font-size: 24px;
      background-color: rgba(0, 0, 0, 0.5);
      display: inline-block;
      padding: 20px;
      border-radius: 10px;
      box-shadow: 0 0 10px black;
      color: #ffdddd;
    }

    a.back-link img {
      width: 220px;
      border-radius: 8px;
      margin-top: 40px;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
      transition: transform 0.3s ease, box-shadow 0.3s ease;
    }

    a.back-link:hover img {
      transform: scale(1.1);
      box-shadow: 0 6px 12px rgba(0, 0, 0, 0.3);
    }
  </style>
</head>

<body>
  <!-- Title matching the style of the form -->
  <h2>Perl Script Result</h2>

  <!-- Display the user's input -->
  <p>You said: <strong>$speech</strong></p>

  <!-- Back button image link to return to the form -->
  <br>
  <a href="/" class="back-link">
    <img src="/img/back_index.png" alt="Return to form">
  </a>
</body>
</html>
HTML
