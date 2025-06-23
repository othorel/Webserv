#!/usr/bin/env perl
use strict;
use warnings;
use CGI;

my $q = CGI->new;

print $q->header('text/html; charset=UTF-8');
print $q->start_html('Réponse du CGI Perl');
print $q->h1('Message reçu');

my $message = $q->param('speech') || '';
$message =~ s/</&lt;/g;
$message =~ s/>/&gt;/g;

print "<p>Vous avez écrit : <strong>$message</strong></p>";

print $q->end_html;
