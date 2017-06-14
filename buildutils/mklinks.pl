#!/usr/bin/perl

use English;
use POSIX;
use Cwd;
use File::Basename;
#use File::Glob;
use File::Copy;
use File::Spec;

$0 =~ s/.*\///;
$ProgName = $0;

sub usage {
	my($msg) = @_;

	if ($msg) {
		printf STDERR "$ProgName: $msg\n";
	}

	printf STDERR "Usage: %s libname application_path directory\n", $ProgName;
	exit(1);
}


sub chaselink {
    my($path) = @_;

    if (defined($link = readlink($path))) {
		if (! File::Spec->file_name_is_absolute($link)) {
			my($name0,$dir0) = fileparse($path);
			my($name1,$dir1) = fileparse($link);

			$link = "$dir0" . "$name1"; 
		}

        return(chaselink($link));
    }
    else {
        return($path);
    }   
}

if (! (defined($Arch = shift @ARGV))) {
	usage("Wrong # of arguments");
}
if (! (defined($Libname = shift @ARGV))) {
	usage("Wrong # of arguments");
}
if (! (defined($Application = shift @ARGV))) {
	usage("Wrong # of arguments");
}
if (! (defined($TargetDir = shift @ARGV))) {
	usage("Wrong # of arguments");
}

if (defined(shift @ARGV)) {
	usage("Wrong # of arguments");
}

if ($Arch eq "Darwin") {
	$cmd = "/usr/bin/otool -L $Application";
} else {
	$cmd = "/usr/bin/ldd $Application";
}

$_ = `$cmd`;
if ($?>>8) {
	printf STDERR "$ProgName: Command \"$cmd\" failed\n";
	exit(1);
}

@matchs = ();
@lines = split /\n/, $_;
foreach $line (@lines) {
	if ($line =~ $Libname) {
		push @matchs, $line;
	}
}

if (@matchs < 1) {
	printf STDERR "$ProgName: WARNING : Library $Libname not found in application $Application\n";
	exit(0);
} 

if (@matchs > 1) {
	printf STDERR "$ProgName: Multiple instances of library $Libname found in application $Application\n";
	exit(1);
} 

# remove leading white space. Don't know why this is neccessary - split
# should take care of it.
#
$matchs[0] =~ s/^\s+//;

@_ = split /\s+/, $matchs[0];
if ($Arch eq "Darwin") {
	$path = $_[0];
}
else {
	$path = $_[2];
}


my($name,$dir,$suffix) = fileparse($path);

@libs = glob("$dir" . $Libname . "*");

@cpfiles = ();

foreach $lib (@libs) {

	if (defined(readlink($lib))) {
		$link = chaselink($lib);

		my($oldname,$olddir,) = fileparse($link);
		my($newname,$newdir) = fileparse($lib);

		if ($oldname ne $newname) {

			$newname = "$TargetDir" . "/" . $newname;

			if (! eval { symlink("$oldname","$newname"); 1 }) {
				printf STDERR "$ProgName: symlink($oldname, $newname) : failed\n";
				exit(1);
			}
		}
		else {
			push @cpfiles, $link 
		}
	}
	else {
		push @cpfiles, $lib 
	}
}

foreach $lib (@cpfiles) {
	copy($lib, $TargetDir) or die "Copy failed: $!";
	my($name,$dir,) = fileparse($lib);
	chmod 0755, $TargetDir . "/" . $name;
}

exit 0;
