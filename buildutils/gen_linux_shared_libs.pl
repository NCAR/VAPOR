#!/usr/bin/env perl

use strict;
use warnings;

if (scalar @ARGV < 1) {
	print "Usage $0 input_bin [extra_lib [...]]\n";
	exit;
}

if (not -B $ARGV[0]) {
	print STDERR "Error: File \"$ARGV[0]\" is not a binary file\n";
	exit 1;
}

my @extras;
for (my $i = 1; $i < scalar @ARGV; $i++) {
	push @extras, $ARGV[$i];
}

my $ldd = `ldd $ARGV[0]`;

foreach (split(/\n/, $ldd)) {
	print "$1*\n" if m/=> (\/glade\S+\.so)/;

	foreach my $e (@extras) {
		print "$1*\n" if m/.*$e.*=> (\/.+.so)/;
	}
}
