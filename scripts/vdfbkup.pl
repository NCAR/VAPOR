#!/usr/bin/perl
#
#      $Id$
#
#########################################################################
#									#
#			   Copyright (C)  2007                          #
#	     University Corporation for Atmospheric Research		#
#			   All Rights Reserved				#
#									#
#########################################################################
#
#	File:		vdfbkup.pl
#
#	Author:		John Clyne
#			National Center for Atmospheric Research
#			PO 3000, Boulder, Colorado
#
#	Date:		Fri Jan 13 18:00:32 MST 1995
#
#	Description:	
#
#	Usage:
#
#	Environment:
#
#	Files:
#
#
#	Options:

use English;
use POSIX;
use File::Basename;
use File::Spec;
use File::Copy;
use Cwd 'abs_path';
use Cwd;


$tmpdirdef = defined($ENV{'TMPDIR'}) ? $ENV{'TMPDIR'} : "/tmp";
#
#	Options is a table of *configurable* options supported by
#	vdfbkup.pl. The fields contained within are: option name, perl variable
#	name, default option values, number of option arguments (0 or 1), and
#	a description of the option.
#
@Options = (
"maxtarsize",	"MaxTarSize",	"5000",	'1', "Max size of tar file(MBytes)",
"maxsize",	"MaxSize",	"500",	'1', "Max size of file to tar (MBytes)",
"maxarg",	"MaxArg",	"70000",'1', "Max size of unix cmd line(bytes)",
"bs",	"BS",	"512",'1', "Tar blocking factor (bs*512 bytes)",
"nr",		"NotReally",	"0",	'0', "Echo, but do not execute cmds",
"quiet",	"Quiet",	"0",	'0', "Operate quitely",
"nolog",	"NoLog",	"0",	'0', "Do not create a log file",
"restart",	"Restart",	"0",	'0', "Restart using the ./vdfbkup_restart.txt restart file",
);


sub     usage {
	local($s) = @_;

	local($format) = "\t%-12.12s  %-12.12s  %-5.5s %s\n";

	if (defined ($s)) {
		print STDERR "$ProgName: $s\n";
	}

	print STDERR "Usage: $ProgName [options] vdffile (directory|command)\n";
	print STDERR "Usage: $ProgName -restart\n";
	print STDERR "\nWhere \"options\" are:\n\n";
	printf STDERR $format, "Option name", "Default", '#args', "Description";
	printf STDERR $format, "------ ----", "-------", '-----', "-----------";
	print STDERR "\n";

	for($i=0; $i<=$#Options; $i+=5) {
		printf STDERR $format,	"-$Options[$i]", $Options[$i+2], $Options[$i+3], $Options[$i+4];
	}
	exit(1);
}

#
#	Set a variable, whose name is given by `$name', to the value
#	given by `$value'
#
sub set_var_by_name {
	local($name, $value) = @_;

	eval 	'$' . $name . " = \'$value\'";
}

sub get_var_by_name {
	local($name) = @_;
	local($value);

	eval '$value = ' . '$' . "$name";

	return($value);
}

sub configure {
	my (@argv) = @_;
	my ($i);
	my ($match);

        #
        #       Set defaults
        #
        #       This nonsense creates a variable named by the second
        #       field of @Options (the option name) and assigns it
        #       the value of the third field of @Options (the default option
        #       argument). 
        #
        for($i=0; $i<=$#Options; $i+=5) {
                do set_var_by_name($Options[$i+1], $Options[$i+2]);
        }


	#
	#	Next, parse the command line, overriding anything
	#	set by default above.
	#
	while ($argv[0] =~ /^-/) {
		$opt = shift @argv;

		$match = -1;	# index of matching option

		#
		#	Look for an option name that matches the 
		#	option specifier given on the command line. Exact
		#	matches aren't required. The shortes unique string
		#	is sufficient
		#
		for($i=0; $i<=$#Options; $i+=5) {
			$_ = "-" . $Options[$i];	# option name


			if (/^$opt/) {

				#
				# Do we already have a match? If so the 
				# option specifier is ambiguous
				#
				if ($match >= 0) {
					do usage("Ambiguous option: \"$opt\"");
				}
				$match = $i;
			}

		}
		if ($match < 0) {
			do usage ("Unknown option: \"$opt\"");
		}

		#
		# does the option take an argument? If not the option's 
		# value is given by the table, "Options"
		#
		if ($Options[$match+3]) {
			defined($value = shift @argv) || do usage("Missing offset");
		}
		else {
			$value = ! $Options[$match+2];
		}

		do set_var_by_name($Options[$match+1], $value);

	}
	return(@argv);
}

sub mysystem {
    my(@cmd) = @_;

	if (! $Quiet) {
		smsg(*STDOUT, "Executing -> @cmd");
	}
	
	if (! $NotReally) {
		system(@cmd);
		if ($? != 0) {
			smsg(*STDERR, "Command \"@cmd\" exited with error");
			exit(1);
		}
	}
}

sub	Cleanup {
	my($sig) = @_;

	close RESTART;
	print STDERR "$ProgName: Caught a SIG$sig -- Shutting down\n";
	exit(0);
}


sub	timestamp {
	my($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime();

	$year -= 100;	# y2k
	return(sprintf(
		"%2.2d%2.2d%2.2d%2.2d%2.2d%2.2d",
		$year, $mon, $mday, $hour, $min, $sec)
	);
}
	


sub 	smsg {
	my($fh, $msg) = @_;
	my($str);

	my($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime();
	$timestr = POSIX::strftime(
		"%a, %b %d %H:%M:%S", $sec, $min, $hour, $mday, $mon, $year, $wday, 
		$yday, $isdst
	);

	$msg = sprintf("$ProgName: ($timestr): %s\n", $msg);

	if (length($msg) < $MaxMsg) {
		printf $fh "%s\n", "$msg";
	}
	else {
		my($maxmsg);
		my($elipses) = "...";

		$maxmsg = $MaxMsg - length($elipses);
		printf $fh "%-${MaxMsg}.${MaxMsg}s", "$msg";
		printf $fh "%s\n", $elipses;
	}
}

sub	copyit {
	local($src, $dst) = @_;

	#if (-d $dst) {
	#	$dst = File::Spec->catfile($dst, $src);
	#}

	if (! $Quiet) {
		printf STDOUT "Copying file $src to $dst\n";
	}

	if (! $NotReally) {
		if (! copy($src, $dst)) {
			smsg (*STDERR, "Can't copy $src to $dst");
			exit(1);
		}
	}

   if (! $NoLog) {
		if (-f $src) {
			($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size, $atime,$mtime,$ctime,$blksize,$blocks) = stat($src);

			print FILE "$src   -   $size\n";
		}
	}
}

sub vdfsplit {
	my($file) = @_;

	my($dummy1, $varname) = File::Spec->splitdir($file);
	my($dummy1, $dummy2, $filebase) = File::Spec->splitpath($file);

	my($dummy, $ts, $level) = split(/\./, $filebase);
	$level =~s/nc//;

	return($varname, $ts, $level);
}

sub	tarit {
	local($tarfile, @files) = @_;

	my(@cmd) = ("tar", "-c", "-b", $BS, "-f", $tarfile, @files);

	mysystem(@cmd);

   if (! $NoLog) {
			print FILE "\n------------------\n$tarfile contents\n------------------\n";
		foreach $file (@files) {
			if (-f $file) {
				($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size, $atime,$mtime,$ctime,$blksize,$blocks) = stat($file);

				print FILE "$file   -   $size\n";
			}
		}
	}
}

#
# Save state of @CmdLineArgs, $VDFFile, @FilesToTar, @TarNames, and 
# @FilesToCopy variables
#
sub save_state {

	if (! open (RESTART, "> $RestartFile")) { 
		print STDERR "Can't open state file $RestartFile\n";
		return;
	}

	foreach $arg (@CmdLineArgs) {
		print RESTART "$arg\n";
	}
	print RESTART "END_CMD_LINE_ARGS\n";

	$vdffile = abs_path($VDFFile);
	print RESTART "$vdffile\n";

	foreach $file (@FilesToCopy) {
		print RESTART "$file\n";
	}

	my(@tar_names) = @TarNames;
	foreach $listref (@FilesToTar) {
		@a = @$listref;
		$tarfile = shift @tar_names;
		print RESTART "$tarfile\n";
		foreach $file (@a) {
			print RESTART "$file\n";
		}
	}
	close RESTART;
}

#
# Restore state of @CmdLineArgs, $VDFFile, @FilesToTar, @TarNames, and 
# @FilesToCopy variables
#
sub restore_state {

	if (! open (RESTART, "< $RestartFile")) { 
		print STDERR "Can't open state file $RestartFile\n";
		return ();
	}

	my($done) = 0;
	while ((!$done) && ($_ = <RESTART>)) {
		chop $_;

		if ($_ =~ /^END_CMD_LINE_ARGS$/) {
			$done = 1;
			next;
		}
		push @CmdLineArgs, $_;
	}

	if (! defined($VDFFile = <RESTART>)) {
		print STDERR "Bogus restart file\n";
		return();
	}
	chop $VDFFile;

	@FilesToCopy = ();
	@TarNames = ();
	my($done) = 0;
	while ((!$done) && ($_ = <RESTART>)) {
		chop $_;

		if ($_ =~ /\.tar$/) {
			push @TarNames, $_;
			$done = 1;
			next;
		}
		push @FilesToCopy, $_;
	}
	@FilesToTar = ();

	@list = ();
	while ($_ = <RESTART>) {
		chop $_;

		if ($_ =~ /\.tar$/) {
			push @TarNames, $_;
			push @FilesToTar, [@list];
			@list = ();
			next;
		}
		push @list, $_;
	}

	if (@list) {
		push @FilesToTar, [@list];
	}

	close RESTART;

}

sub tar_create {
	my($vdfbase) = @_;

	if (! $Quiet) {
		printf "%-30.30s: %s\n", "Max tar file size (MB's)", $MaxTarSize;
		printf "%-30.30s: %s\n", "Max tar file element size (MB's)", $MaxSize;
		printf "%-30.30s: %s\n", "Max arg length (bytes)", $MaxArg;
		printf "\n";
	}

	$MaxTarSize *= 0.95;	# allow for tar overhead.



	if (! $Restart) {

		$data_dir = $vdfbase;
		$data_dir =~ s/\.vdf/_data/;

		$cmd = join(' ', @VDFLSCmd, $vdfbase);
		if (! $Quiet) {
			smsg(*STDOUT, "Executing -> $cmd");
		}
		$_ = `$cmd`;
		if ($?>>8) { 
			printf STDERR "$ProgName: Command \"$cmd\" failed\n";
			exit(1);
		}

		my(@files) = ();
		@lines = split /\n/, $_;
		foreach $_ (@lines) {
			push @files, $data_dir . "/" . $_;
		}


		@FilesToTar = ();	# list of lists of files to tar
		@FilesToCopy = ();# list of files to copy without tarring
		$targlen = 0;
		$tsize = $tsize_all = 0;
		@list = ();
		@TarNames = ();
		foreach $file (@files) {
			if (-f $file) {
				($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size, $atime,$mtime,$ctime,$blksize,$blocks) = stat($file);
			}
			else {
				next;
			}
			$size = $blocks / 2048;	# convert from 512-byte blocks to mbytes

			$arglen = length("$file ");

			if ($size > $MaxSize) {
				push @FilesToCopy, $file;
				$tsize_all += $size;
				next;
			}

			if (! @list) {
				($varname0, $ts0, $level0) = vdfsplit($file);
				($varname1, $ts1, $level1) = vdfsplit($file);
			}
			($varname, $ts, $level) = vdfsplit($file);
			
			if (
				(($targlen + $arglen) >= $MaxArg) || 
				(($tsize + $size) >= $MaxTarSize) ||
				($varname0 ne $varname) ||
				($level0 != $level)) {

				push @FilesToTar, [@list];
				push @TarNames, sprintf("%s_%4.4d-%4.4d.nc%d.tar", $varname0, $ts0, $ts1, $level0);
				@list = ();
				($varname0, $ts0, $level0) = vdfsplit($file);
				($varname1, $ts1, $level1) = vdfsplit($file);
				$targlen = 0;
				$tsize = 0;
			}

			($varname1, $ts1, $level1) = vdfsplit($file);

			push @list, $file;
			$targlen += $arglen;
			$tsize += $size;
			$tsize_all += $size
		}

		if (@list) {
			push @FilesToTar, [@list];
			push @TarNames, sprintf("%s_%4.4d-%4.4d.nc%d.tar", $varname0, $ts0, $ts1, $level0);
		}
	}

	#
	# back up the vdf file
	#
	if (defined ($TargetDirectory)) {
		copyit($vdfbase, $TargetDirectory);
	}
	else {
		my(@cmd) = (@BackupCmd);
		foreach $_ (@cmd) {
			my($dummy1, $dummy2, $filebase) = File::Spec->splitpath($vdfbase);
			$_ =~ s/%s/$vdfbase/;
			$_ =~ s/%b/$filebase/;
		}
		mysystem(@cmd);
	}


	save_state();

	while ($file = shift(@FilesToCopy)) {
		if (defined ($TargetDirectory)) {
			copyit($file, $TargetDirectory);
		}
		else {
			my(@cmd) = (@BackupCmd);
			foreach $_ (@cmd) {
				my($dummy1, $dummy2, $filebase) = File::Spec->splitpath($file);
				$_ =~ s/%s/$file/;
				$_ =~ s/%b/$filebase/;
			}
			mysystem(@cmd);
		}
		save_state();
	}

	while ($listref = shift(@FilesToTar)) {
		$tarbase = shift(@TarNames);
		if (defined ($TargetDirectory)) {
			$tarfile = File::Spec->catfile($TargetDirectory, $tarbase);
		}
		else {
			$tarfile = $tarbase;
		}
		@files = @$listref;
		tarit($tarfile, @files);
		if (defined (@BackupCmd)) {
			my(@cmd) = (@BackupCmd);
			foreach $_ (@cmd) {
				my($dummy1, $dummy2, $filebase) = File::Spec->splitpath($tarfile);
				$_ =~ s/%s/$tarfile/;
				$_ =~ s/%b/$filebase/;
			}
			mysystem(@cmd);

			unlink $tarfile;
		}
		save_state();
	}
}

#################################################################
##
##	M A I N   P R O G R A M
##
#################################################################


$0              =~ s/.*\///;
$ProgName       = $0;
$MBYTE		= 1024 * 1024;
$MaxMsg		= "256";
@VDFLSCmd	= ("vdfls", "-sort", "varname");
$RestartFile = File::Spec->catfile(cwd(), ".vdfbkup_restart.txt");

@CmdLineArgs = @ARGV;
@ARGV = configure(@ARGV);


if ($Restart) {
	if (@ARGV != 0) {
		usage("Wrong # of arguments");
	}
	restore_state();
	@ARGV = @CmdLineArgs;
	@ARGV = configure(@ARGV);

} else {
	if (-f $RestartFile) {
		print STDERR "A restart file, $RestartFile,\n"; 
		print STDERR "was found from a previous\n";
		print STDERR "session, but the -restart option was not present. You \n";
		print STDERR "must remove the restart file or use the -restart option.\n";
		exit (1);
	}
}

if (@ARGV < 2) {
	usage("Wrong # of arguments - no files specified");
}

$VDFFile = shift @ARGV;

if (defined $TargetDirectory) {undef $TargetDirectory;}
if (defined @BackupCmd) {undef @BackupCmd;}

if (-d $ARGV[0]) {
	$TargetDirectory = abs_path(shift @ARGV);
} else {
	@BackupCmd = @ARGV;
}


$SIG{INT} = 'Cleanup';

if (defined $TargetDirectory) {
	$LogFile = File::Spec->catfile($TargetDirectory, "vdfbkup.log");
}
else  {
	$LogFile = "vdfbkup.log";
}
	

my($volume, $directory, $vdfbase) = File::Spec->splitpath($VDFFile);
if ($directory ne "") {
	chdir $directory or die "$ProgName: Can't cd to $directory: $!\n";
}

if (! $NoLog) {
	open (FILE, "> $LogFile") || die "Can't open log file $LogFile for writing!";
}


tar_create($vdfbase);

if (! $NoLog) {
	close FILE;
}

if (defined @BackupCmd && ! $NoLog) {
	my(@cmd) = (@BackupCmd);
	foreach $_ (@cmd) {
		my($dummy1, $dummy2, $filebase) = File::Spec->splitpath($LogFile);
		$_ =~ s/%s/$LogFile/;
		$_ =~ s/%b/$filebase/;
	}
	mysystem(@cmd);
	unlink $LogFile if (! $NotReally);
}

unlink $RestartFile;
