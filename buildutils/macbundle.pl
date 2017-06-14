#!/usr/bin/perl
#
#      $Id$
#
#########################################################################
#									#
#			   Copyright (C)  2006				7
#	     University Corporation for Atmospheric Research		#
#			   All Rights Reserved				#
#									#
#########################################################################
#
#	File:		macbundle.pl
#
#	Author:		John Clyne
#			National Center for Atmospheric Research
#			PO 3000, Boulder, Colorado
#
#	Date:		Thu Feb  1 15:18:09 MST 2007
#
#	Description:	Bundle up vapor for mac platform
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
use File::Spec;
use File::Glob ':globally';	# what the hell is this?

my ($prog_path) = $0;
$prog_path =~ s/.*\///;
$ProgName = $prog_path;


sub usage {
	my($msg) = @_;

	my($format) = "\t%-12.12s  %-16.16s  %s\n";

	if ($msg) {
		printf STDERR "$ProgName: $msg\n";
	}

	printf STDERR "Usage: %s install_path template_path dst_dir version\n", $ProgName;

	exit(1);
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



sub copy_dir_contents{
	my($srcdir, $destdir) = @_;

	$tmpfile =  "/tmp/ProgName.$$.tar";

	$cwd = getcwd();
	chdir $srcdir or die "$ProgName: Can't cd to $srcdir: $!\n";
	@cmd = ("/usr/bin/tar", "-cf", $tmpfile, ".");
	mysystem(@cmd);

	if (! -d $destdir) {
		@cmd = ("/bin/mkdir", "-p", $destdir);
		mysystem(@cmd);
	}

	chdir $cwd or die "$ProgName: Can't cd to $cwd: $!\n";
	chdir $destdir or die "$ProgName: Can't cd to $destdir: $!\n";

	@cmd = ("/usr/bin/tar", "-xf", $tmpfile);
	mysystem(@cmd);

	chdir $cwd or die "$ProgName: Can't cd to $cwd: $!\n";

	unlink $tmpfile;
}

sub copy_template {
	($template_path, $bundle_path) = @_;

	if (-e $bundle_path) {
		@cmd = ("/bin/rm", "-fr", $bundle_path);
		mysystem(@cmd);
	}

	mkdir $bundle_path or die "$ProgName: Can't mkdir $bundle_path: $!\n";
	copy_dir_contents($template_path, $bundle_path);
}

sub copy_install_targets {
	my($install_path, $bundle_path) = @_;

	$srcdir = File::Spec->catdir($install_path, "bin");
	$dstdir = File::Spec->catdir($bundle_path, "Contents", "MacOS");
	copy_dir_contents($srcdir, $dstdir);

	$srcdir = File::Spec->catdir($install_path, "lib");
	$dstdir = File::Spec->catdir($bundle_path, "Contents", "MacOS");
	copy_dir_contents($srcdir, $dstdir);

	$srcdir = File::Spec->catdir($install_path, "include");
	$dstdir = File::Spec->catdir($bundle_path, "Contents", "FrameWorks", "Headers");
	copy_dir_contents($srcdir, $dstdir);

	$srcdir = File::Spec->catdir($install_path, "share");
	$dstdir = File::Spec->catdir($bundle_path, "Contents", "SharedSupport");
	copy_dir_contents($srcdir, $dstdir);

	$srcdir = File::Spec->catdir($install_path, "plugins");
	$dstdir = File::Spec->catdir($bundle_path, "Contents", "Plugins");
	copy_dir_contents($srcdir, $dstdir);

	$srcdir = File::Spec->catdir("..", "Frameworks", "Headers");
	$dstdir = File::Spec->catdir($bundle_path, "Contents", "MacOS","include");
	@cmd = ("/bin/ln", "-s", $srcdir, $dstdir);
	mysystem(@cmd);

}

sub edit_template {
	my($templatepath, $bundlepath, $version) = @_;


	$file = File::Spec->catfile($templatepath, "Contents", "Info.plist");
	if (! open(IFH, "<$file")) {
		printf STDERR "$ProgName: Can't open \"$file\": $!\n";
		exit(1);
	}

	$file = File::Spec->catfile($bundlepath, "Contents", "Info.plist");
	if (! open(OFH, ">$file")) {
		printf STDERR "$ProgName: Can't open \"$file\": $!\n";
		exit(1);
	}

	while (<IFH>) {
		s/VERSION/$version/;
		printf OFH "%s", $_;
	}

	close (OFH);
	close (IFH);
}

if ($#ARGV == 2) {
	$TemplatePath = File::Spec->canonpath(shift(@ARGV));
	$DstDir = File::Spec->canonpath(shift(@ARGV));
}
elsif ($#ARGV == 3) {
	$InstallPath = File::Spec->canonpath(shift(@ARGV));
	$TemplatePath = File::Spec->canonpath(shift(@ARGV));
	$DstDir = File::Spec->canonpath(shift(@ARGV));
}
else {
	usage("Wrong # of arguments");
}
$VersionString = shift(@ARGV);

@dirs = File::Spec->splitdir($TemplatePath);
$BundleName = pop(@dirs);
$SrcDir = File::Spec->catdir(@dirs);
$BundlePath = File::Spec->catdir($DstDir, $BundleName);


if (defined(shift @ARGV)) {
	usage("Wrong # of arguments");
}

copy_template($TemplatePath, $BundlePath);

edit_template($TemplatePath, $BundlePath, $VersionString);

if (! (defined($InstallPath))) {
	# we're done.
	exit(0);
}  

copy_install_targets($InstallPath, $BundlePath);


$vapor_install = File::Spec->catfile($InstallPath, "vapor-install.csh");
$macos_dir = File::Spec->catdir($BundlePath, "Contents", "MacOS");
$plugins_dir = File::Spec->catdir($BundlePath, "Contents", "Plugins");

my ($vol, $dir, $file) = File::Spec->splitpath($0);
print "dir = $dir\n";
my ($install_name) = File::Spec->catpath($vol, $dir, "install_name.pl");
my (@cmd) = (
	$install_name, "-exec_path", "$InstallPath/bin", $macos_dir, $plugins_dir, $macos_dir
);
mysystem(@cmd);

$mandir = File::Spec->catdir($bundle_path, "Contents", "SharedSupport", "man");

my (@cmd) = (
	$vapor_install, "-nocopy", "-root", $macos_dir, 
	"-libdir", $macos_dir, "-bindir", $macos_dir, "-mandir", $mandir,
	$macos_dir
);
mysystem(@cmd);

exit(0);
