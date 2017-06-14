#!/usr/bin/perl

use English;
use POSIX;
use Cwd;
use Cwd 'abs_path';
use File::Basename;
use File::Copy;
use File::Spec;

$0 =~ s/.*\///;
$ProgName = $0;
$Debug = 0;

sub usage {
	my($msg) = @_;

	if ($msg) {
		printf STDERR "$ProgName: $msg\n";
	}

    my($format) = "\t%-12.12s  %-12.12s  %s\n";
        

	print STDERR "Usage: $ProgName [options] targetlist... libdir";
	print STDERR "\nWhere \"options\" are:\n\n";
	printf STDERR $format, "Option name", "Default", "Description";
	printf STDERR $format, "------ ----", "-------", "-----------";
	printf STDERR $format, "-exclude", "expression", "Paths to exclude";
	printf STDERR $format, "-include", "expression", "Paths to include (override -exclude)";
	printf STDERR $format, "-ldlibpath", "expression", "library search path";
	print STDERR "\n";

    
    exit(1)


}


sub mysystem {
	my(@cmd) = @_;

	print "cmd=@cmd\n";

	system(@cmd);
	if ($? != 0) {
		print STDERR "$ProgName: \"@cmd\" exited with error\n";
		exit(1);
	}
}

sub copy_dir{
	my($srcdir, $destdir) = @_;

	my($volume, $parentdir, $file) = File::Spec->splitpath($srcdir);

	$tmpfile =  "/tmp/$ProgName.$$.tar";

	$cwd = getcwd();
	chdir $parentdir or die "$ProgName: Can't cd to $parentdir: $!\n";
	@cmd = ("tar", "-cf", $tmpfile, "$file");
	mysystem(@cmd);

	chdir $cwd or die "$ProgName: Can't cd to $cwd: $!\n";
	chdir $destdir or die "$ProgName: Can't cd to $destdir: $!\n";

	@cmd = ("tar", "-xf", $tmpfile);
	mysystem(@cmd);

	chdir $cwd or die "$ProgName: Can't cd to $cwd: $!\n";

	unlink $tmpfile;
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

	
sub get_deps {
	my($target) = @_;

	#
	# ldd generates error message on 32bit version of libQtCore 
	# "Accessing a corrupted shared library"
	#
	if (($Arch eq "Linux") && ( ($target =~ /libQtCore/))) {
		printf STDERR "$ProgName: SKIPPING $target\n";
		return();
	}

	my(@Deps) = ();
	my($target_is_lib) = 1;
	my(@lddcmd);

	my($cmd) = "/usr/bin/file $target";
	$_ = `$cmd`;
	if ($?>>8) {
		printf STDERR "$ProgName: Command \"$cmd\" failed\n";
		exit(1);
	}
	if ($_ =~ "executable") {
		$target_is_lib = 0;
	}

	if ($Arch eq "Darwin") {
		# Mac system
		@lddcmd = ("/usr/bin/otool", "-L"); 

	} else {
		@lddcmd = ("/usr/bin/ldd");
	}
	my ($cmd) = join(' ', @lddcmd, $target);
	$_ = `$cmd`;
	if ($?>>8) {
		printf STDERR "$ProgName: Command \"$cmd\" failed\n";
		exit(1);
	}
	my(@lines) = split /\n/, $_;

	if (($Arch eq "Darwin") || ($Arch eq "AIX")) {
		shift @lines;	# discard first line
	}

	# clyne - Fri Feb  5 12:20:50 MST 2016
	#
	# for .so files the second line is *not* the library name
	#

	#if (($Arch eq "Darwin") && $target_is_lib) {
	#shift @lines;	# discard second line - the library name
	#}




	# Get base name of target without any extensions
	#
	my ($tfname, $tdirs, $text) = fileparse($target, qr/\..*/);

LINE:	foreach $line (@lines) {
		my($lib);
		$line =~ s/^\s+//;
		if ($Arch eq "Darwin") {
			($lib) = split(/\s+/, $line);
		}
		elsif ($Arch eq "AIX") {
			($lib) = split(/\s+/, $line);
			$lib =~ s/\(.*\)//;
			next LINE if ($lib eq "/unix");
		}
		else {
			($junk1, $junk2, $lib) = split(/\s+/, $line);
		}

		next LINE if (! defined($lib));

		if ($lib =~ "not found") {
			printf STDERR "$ProgName: Command \"$cmd\" failed - library $lib not found\n";
			exit(1);
		}

		next LINE if (($Arch eq "Darwin") && !( ($lib =~ /dylib/) || $lib =~ /framework/) );
		next LINE if (($Arch eq "Linux") && !( ($lib =~ /\.so/)));

		# Handle case where target name appears as its own dependency
		#
		my ($fname, $dirs, $ext) = fileparse($lib, qr/\..*/);
		next LINE if (($Arch eq "Darwin") && ($tfname eq $fname));

		my ($toss) = 0;
		foreach $exclude (@ExcludePaths) {
			if ($lib =~ m!$exclude!) {
				$toss = 1;
			}
		}
		if ($toss) {
			foreach $include (@IncludePaths) {
				if ($lib =~ m!$include!) {
					$toss = 0;
				}
			}
		}

		if (! $toss) {
			push @Deps, $lib;
		}
	}
	return(@Deps);
}

@IncludePaths = ();
@ExcludePaths = ();
while ($ARGV[0] =~ /^-/) {
    $_ = shift @ARGV;

    if (/^-exclude$/) {
        defined($_ = shift @ARGV) || die "Missing argument";
		push(@ExcludePaths, $_);
    }
    elsif (/^-arch$/) {
        defined($Arch = shift @ARGV) || die "Missing argument";
    }
    elsif (/^-include$/) {
        defined($_ = shift @ARGV) || die "Missing argument";
		push(@IncludePaths, $_);
    }
    elsif (/^-ldlibpath$/) {
        defined($_ = shift @ARGV) || die "Missing argument";
		$LD_LIBRARY_PATH = defined($LD_LIBRARY_PATH) ? "$LD_LIBRARY_PATH:$_" : $_;
    }
    else {
        usage("Invalid option: $_");
    }
}


if (! (defined($Libdir = pop @ARGV))) {
	usage("Wrong # of arguments");
}
$Libdir = abs_path($Libdir);

if (! -d $Libdir) {
	print STDERR "$ProgName: Library directory $Libdir does not exist\n";
	exit(1);
}

@Targets = @ARGV;

#
# Set LD environment variables for ld search path.
# Needed for the ldd command only (Mac 'otool' ignores these)
#
$ENV{"LD_LIBRARY_PATH"} = $LD_LIBRARY_PATH;
$ENV{"LD_LIBRARYN32_PATH"} = $LD_LIBRARY_PATH;
$ENV{"LD_LIBRARY64_PATH"} = $LD_LIBRARY_PATH;

#
# Recursively look for library dependencies
#
@cpfiles = ();
while (defined($target = shift(@Targets))) {

	if ($Debug) {print "Target = $target\n";}

	@_ = get_deps($target);

	foreach $dep (@_) {
		if ($Debug) {print "Dep = $dep\n";}

		if (! -f $dep) {
			printf STDERR "$ProgName: Library dependency $dep not found\n";
			exit(1);
		}

		$match = 0;
		foreach $target (@Targets) {
			$match = 1 if ($dep eq $target);
		}
		if ($Arch ne "AIX") {
			push @Targets, $dep if (! $match);
		}

		#
		# add library dependency to copy list if not already there
		#
		$match = 0;
		foreach $cpfile (@cpfiles) {
			$match = 1 if ($dep eq $cpfile);
		}
		push @cpfiles, $dep if (! $match);
	}
}

foreach $_ (@cpfiles) {
	$dirname = abs_path(dirname($_));
	if ($dirname ne $Libdir) {

		if (! (($Arch eq "Darwin") && $_ =~ /framework/)) {

			my($volume, $dir, $file) = File::Spec->splitpath($_);

			my ($target) = File::Spec->catpath("", $Libdir, $file);

			print "Copying $_ to $target\n";
			copy($_,$target) || die "$ProgName: file copy failed - $!\n";
			my(@cmd) = ("/bin/chmod", "+x", $target);
			mysystem(@cmd);

			my ($baselib) = $target;
			if ($Arch eq "Darwin") {
				if ($baselib =~ /(\.[\d+|.]+dylib)/) {
					$baselib =~ s/$1/.dylib/;
				}
			} 
			else {
				if ($baselib =~ /(\.so\.[\d+|.]+)/) {
					$baselib =~ s/$1/.so/;
				}
			}
			if (($baselib ne $target) && ! -e $baselib) {
				my(@cmd) = ("/bin/ln", "-s", "$file", "$baselib");
				mysystem(@cmd);
			}
		}
		else {
			$dirname = $_;
			$dirname =~ s/(.*framework).*/$1/;
			copy_dir($dirname, $Libdir);
		}
	}
}



exit 0;
