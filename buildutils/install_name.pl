#!/usr/bin/perl

use English;
use POSIX;
use Cwd;
use File::Basename;
#use File::Glob;
use File::Copy;
use File::Spec;
use File::Find;

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
	printf STDERR $format, "-exec_path", "", "Path to executables";
	printf STDERR $format, "-ldlibpath", "", "library search path";
	print STDERR "\n";

    
    exit(1)


}

sub mysystem {
	my(@cmd) = @_;

	if ($Debug) {print "cmd=@cmd\n";}

	system(@cmd);
	if ($? != 0) {
		print STDERR "$ProgName: \"@cmd\" exited with error\n";
		exit(1);
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

sub want_libraries {
    if (-f $File::Find::name) {

		my ($cmd) = "/usr/bin/file $File::Find::name";
		$_ = `$cmd`;
		if ($?>>8) {
			#printf STDERR "$ProgName: Command \"$cmd\" failed\n";
			#exit(1);
			return;
		}
		if ($_ =~ "Mach-O" && $_ =~ "library") {
			push (@WantedFiles, $File::Find::name);
		}
    }
} 

sub want_executables {
    if (-f $File::Find::name) {

		# clyne - Tue Feb  2 20:17:17 MST 2016
		#
		# The /usr/bin/file command returns "Linux/i386 core file". Sigh.
		#
		#my ($cmd) = "/usr/bin/file $File::Find::name";
		#$_ = `$cmd`;
		#if ($?>>8) {
			#printf STDERR "$ProgName: Command \"$cmd\" failed\n";
			#exit(1);
		#	return;
		#}
		#if ($_ =~ "Mach-O" && $_ =~ "executable") {
		#	push (@WantedFiles, $File::Find::name);
		#}

		my ($cmd) = "/usr/bin/otool -h $File::Find::name";
		$_ = `$cmd`;
		if ($?>>8) {
			#printf STDERR "$ProgName: Command \"$cmd\" failed\n";
			#exit(1);
			return;
		}
		if ($_ =~ "Mach header") {
			push (@WantedFiles, $File::Find::name);
		}
    }
} 

sub find_mach_o {
	my($do_lib, @paths) = @_;

	@WantedFiles = ();

	if ($do_lib) {
		find (\&want_libraries, @paths);
	}
	else {
		find (\&want_executables, @paths);
	}

	return(@WantedFiles);
}

sub get_relpath_to_libname {
	my($libname, $exec_path, @libs) = @_;

	foreach $lib (@libs) {
		my($vol_new, $dir_new, $file_new) = File::Spec->splitpath($lib);

		if ($libname eq $file_new) {
			return (File::Spec->abs2rel($lib, $ExecPath));
		}
	}
	return;
}

sub get_dep_libname {
	my($dep) = @_;

	#
	# remove Mac OS macros
	#
	$dep =~ s/\@executable_path//;
	$dep =~ s/\@loader_path//;
	$dep =~ s/\@rpath//;

	my($vol, $dir, $libname) = File::Spec->splitpath($dep);

	return ($libname);
}
		
			
	

#
# Install path is hardwired :-(
#
$InstallExecPath = "/Applications/VAPOR3/VAPOR.app/Contents/MacOS";

$ExecPath = "";
@IncludePaths = ();
@ExcludePaths = ("^/System", "^/usr/lib");	# exclude everything by default
@ExecutablePaths = ();
$Arch = "Darwin";		# only works on Mac OS
while ($ARGV[0] =~ /^-/) {
    $_ = shift @ARGV;

    if (/^-include$/) {
        defined($_ = shift @ARGV) || die "Missing argument";
		push(@IncludePaths, $_);
    }
    elsif (/^-exec_path$/) {
        defined($_ = shift @ARGV) || die "Missing argument";
		push(@ExecutablePaths, $_);
    }
    else {
        usage("Invalid option: $_");
    }
}


if (! (defined($ExecPath = pop @ARGV))) {
	usage("Wrong # of arguments");
}

if (! -d $ExecPath) {
	print STDERR "$ProgName: Executable path $ExecPath does not exist\n";
	exit(1);
}

push(@ExecutablePaths, $ExecPath);

@LibSearchPaths = @ARGV;

@Executables = find_mach_o(0, $ExecPath);
@Libraries = find_mach_o(1, @LibSearchPaths);

push(@IncludePaths, $ExecPath);

foreach $target (@Executables) {

	if ($Debug) {print "Target = $target\n";}

	my(@Deps) = get_deps($target, @ExecutablePaths);

	foreach $dep (@Deps) {

		if ($Debug) {print "dep = $dep\n";}

		my ($libname) = get_dep_libname($dep);
		if (! defined($libname)) {
			print STDERR "$ProgName: Couldn't resolve name for $dep\n";
			exit(1);
		}

		my($rel_path) = get_relpath_to_libname($libname, $ExecPath, @Libraries);
		if (! defined($rel_path)) {
			print STDERR "$ProgName: Dependent library $dep not found\n";
			print STDERR "Looked for $libname in:\n";
			foreach $lib (@Libraries) {
				print STDERR "	$lib\n";
			}
			exit(1);
		}

		#
		# Wed May  6 12:18:32 MDT 2015
		# Hard code name to full application installation path. This makes
		# it easier for 3rd party applications to link to VAPOR libraries. 
		#
		my (@cmd) = (
			"/usr/bin/install_name_tool", "-change", $dep, 
			"$InstallExecPath/$rel_path", $target
		);
#		my (@cmd) = (
#			"/usr/bin/install_name_tool", "-change", $dep, 
#			"\@executable_path/$rel_path", $target
#		);
		mysystem(@cmd);
	}
}

foreach $target (@Libraries) {

	if ($Debug) {print "Target = $target\n";}

	my($rel_path) = File::Spec->abs2rel($target, $ExecPath);
	if (! defined($rel_path)) {
		print STDERR "$ProgName: No path from $target to $ExecPath\n";
		exit(1);
	}

	#
	# Mon Nov 26 19:31:36 MST 2012 - clyne
	# Hard code name to full application installation path. This makes
	# it easier for 3rd party applications to link to VAPOR libraries. 
	#
	my($vol, $dir, $libname) = File::Spec->splitpath($target);
	my (@cmd) = (
		"/usr/bin/install_name_tool", "-id", 
		"$InstallExecPath/$rel_path", $target
	);
#	my (@cmd) = (
#		"/usr/bin/install_name_tool", "-id", 
#		"\@executable_path/$rel_path", $target
#	);
	mysystem(@cmd);

	my(@Deps) = get_deps($target, @ExecutablePaths);

	foreach $dep (@Deps) {

		if ($Debug) {print "dep = $dep\n";}

		my ($libname) = get_dep_libname($dep);
		if (! defined($libname)) {
			print STDERR "$ProgName: Couldn't resolve name for $dep\n";
			exit(1);
		}

		my($rel_path) = get_relpath_to_libname($libname, $ExecPath, @Libraries);
		if (! defined($rel_path)) {
			print STDERR "$ProgName: Dependent library $dep not found\n";
			exit(1);
		}

		my (@cmd) = (
			"/usr/bin/install_name_tool", "-change", $dep, 
			"$InstallExecPath/$rel_path", $target
		);
#		my (@cmd) = (
#			"/usr/bin/install_name_tool", "-change", $dep, 
#			"\@executable_path/$rel_path", $target
#		);
		mysystem(@cmd);

	}
}


exit 0;
