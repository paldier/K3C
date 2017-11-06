#!/usr/bin/perl -w

use warnings;
use strict;
my $owrt = '.';

my @feeds=();

open FEEDS, "$owrt/scripts/feeds list -s |" or die "Unable to get list of feeds!";

while (<FEEDS>) {
	chomp;
	s/#.+$//;
	next unless /\S/;
	my @line = split /\s+/, $_, 2;
	print("found feed: $line[0]\n");
	push @feeds, $line[0];
	system("$owrt/scripts/feeds update $line[0]") and die "ERROR: can't update $line[0]";
}

close FEEDS;

foreach my $feed (@feeds) {
	system("$owrt/scripts/feeds install -a -p $feed") and die "ERROR: can't install $feed";
}
