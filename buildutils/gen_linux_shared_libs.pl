#!/usr/bin/env perl

use strict;
use warnings;
use File::Copy qw(copy);

my $tmpDir = "/tmp/vapor_install_libs";

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

my $ldd = `ldd "$ARGV[0]"`;
my @libs;

foreach (split(/\n/, $ldd)) {
	chomp;
	push @libs, $1 if m/=> (\/glade\S+\.so\S*)/;

	foreach my $e (@extras) {
		push @libs, $1 if m/.*$e.*=> (\/.+.so\S*)/;
	}
}

mkdir $tmpDir if (not -e $tmpDir);

foreach my $l (@libs) {
	my $real = $l;
	while (-l $real) {
		$real = readlink $real;
	}
	if (not $real =~ /^\//) {
		$l =~ m/^.*\//;
		$real = $&.$real;
	}
	$l =~ m/[^\/]+$/;
	my $baseName = $&;
	copy "$real", "$tmpDir/$baseName";
	print "$tmpDir/$baseName\n";
}

# use Data::Dumper qw(Dumper);
# print Dumper \@libs;
