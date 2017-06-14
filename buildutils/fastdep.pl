# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software

sub dir {

  my ( $file ) = @_;

  my ( $dir );
  if ( $file =~ /\// ) {
    $file =~ /^(.*)\/([^\/])+$/;
    $dir = $1;
  } else {
    $dir = ".";
  }

  $dir .= "/";
}

sub includes {

  my ( $file ) = @_;

  return @{$inc{$file}} if ( $inc{$file} );

  my ( @angles, @quotes );
  return () unless open(SCAN, $file);
  while (<SCAN>) {

    next unless /^\s*\#/;
    if ( /^\s*\#\s*include\s*([<\"])(.*)[>\"]/ ) {
      if ( $1 eq "<" ) {
	push @angles, $2;
      } else {
	push @quotes, $2;
      }
    }
  }

  close(SCAN);

  my ( $dir ) = dir($file);
  my ( @files, $f, $name );

  while ( $name = pop @quotes ) {
    $f = $dir . $name;
    if ( -f $f ) {
      push @files, $f;
    } else {
      push @angles, $name;
    }
  }

  foreach $name ( @angles ) {
    foreach $dir ( @incpath ) {
      $f = $dir . $name;
      if ( -f $f ) {
	push @files, $f;
	last;
      }
    }
  }

  $inc{$file} = \@files;
  @files;
}

sub depends {

  my ( $file ) = @_;

  my ( @files ) = ( @_ );
  my ( %files );
  while ( $f = pop @files ) {

    next if exists $files{$f};
    $files{$f} = 1;

    push @files, includes($f);
  }

  keys %files;
}

$obj_prefix = "";
$obj_suffix = ( $^O eq "MSWin32" ) ? ".obj" : ".o";
@extra_targets = ();

foreach $arg ( @ARGV ) {

  if ( $arg =~ /^-I(.+)\/$/ ) {
    push @incpath, $1;
  }
  elsif ( $arg =~ /^-I(.+)$/ ) {
    push @incpath, $1 . "/";
  }
  elsif ( $arg =~ /^--obj-prefix=(.*)$/ ) {
    $obj_prefix = $1;
  }
  elsif ( $arg =~ /^--obj-suffix=(.*)$/ ) {
    $obj_suffix = $1;
  }
  elsif ( $arg =~ /^--extra-target=(.*)$/ ) {
    push @extra_targets, $1;
  }
  elsif ( $arg =~ /^-/ ) {
    # skip it
  }
  else {
    push @files, $arg;
  }
}

foreach $file ( @files ) {

  my ( $obj );

  $file =~ /^(.*)\.\w+$/;
  $obj  = $obj_prefix . $1 . $obj_suffix;
  foreach $t ( @extra_targets ) {
    $obj .= " " . $t;
  }
  foreach $file ( depends($file) ) {
    print "$obj: $file\n";
  }
}
