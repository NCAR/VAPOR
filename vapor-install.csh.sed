#!/bin/csh -f

set arch = "ARCH"
set version = "VERSION"

unset directory
unset vapor_root
unset lib_search_dirs
unset bindir
unset mandir
set nocopy = 0
while ($#argv && ! $?directory)
	if ("$argv[1]" == "-nocopy") then
		set nocopy = 1
	else if ($#argv && "$argv[1]" == "-root") then
		shift
		set vapor_root = $argv[1]
	else if ($#argv && "$argv[1]" == "-libdir") then
		shift
		set lib_search_dirs = $argv[1]
	else if ($#argv && "$argv[1]" == "-bindir") then
		shift
		set bindir = $argv[1]
	else if ($#argv && "$argv[1]" == "-mandir") then
		shift
		set mandir = $argv[1]
	else
		set directory = $argv[1]
		endif
	shift
end

if ($#argv || ! $?directory) then
	echo "Usage: $0 [-nocopy] [-root <root>] directory"
	exit (1)
endif

if ("$directory" !~ /*) then
	echo "Installatin directory ($directory) must specify an absolute path"
	exit (1)
endif

if (! $nocopy) then
	set directory = "${directory}/vapor-${version}"
endif


if (! $?vapor_root) set vapor_root = $directory
if (! $?lib_search_dirs) set lib_search_dirs = $vapor_root/lib
if (! $?bindir) set bindir = $vapor_root/bin
if (! $?mandir) set mandir = $vapor_root/share/man
set sharedir = $vapor_root/share


echo directory = $directory


if (! $nocopy) then
	#
	# If the installation directory doesn't exist, create it.
	#
	if (! -e $directory) mkdir $directory

	if (! -d $directory) then
		echo "$0 : $directory is not a directory"
		exit 1
	endif

	echo "Installing VAPOR to $directory"

	#
	# Copy the distribution to the target directory
	#
	tar cf - bin include lib plugins share | (cd $directory; tar xf -)

endif

if (-e /bin/sed) set sedcmd = /bin/sed
if (-e /usr/bin/sed) set sedcmd = /usr/bin/sed
if ($?SEDCMD) then
	set sedcmd = $SEDCMD
endif
if (! $?sedcmd) then 
	echo "********WARNING***********"
	echo "The sed command was not found on this system and the installation"
	echo "was not completed. To complete the installation, perform either "
	echo "one of the following:"
	echo ""
	echo "1. Set the SEDCMD environment variable to point to the sed "
	echo "stream editor path and rerun this script."
	echo ""
	echo "or"
	echo "" 
	echo "2. Edit the scripts "
	echo "" 
	echo "$directory/bin/vapor-setup.csh"
	echo "$directory/bin/vapor-setup.sh"
	echo ""
	echo "changing the values of the root and lib_search_dirs  "
	echo "variables to the values below:"
	echo ""
	echo "root : $directory"
	echo ""
	echo "lib_search_dirs : $directory/lib"
	echo ""
	echo "********WARNING***********"
	exit 1
endif


if (-e $directory/bin/vapor-setup.csh) then
	set dir = $directory/bin
else
	set dir = $directory
endif

#
# Edit the user environment setup scripts
#
set old0 = 'set[ 	][ 	]*root[ 	][ 	]*=.*$'
set new0 = "set root = $vapor_root"
set old1 = 'set[ 	][ 	]*lib_search_dirs[ 	][ 	]*=.*$'
set new1 = "set lib_search_dirs = $lib_search_dirs"
set old2 = 'set[ 	][ 	]*bindir[ 	][ 	]*=.*$'
set new2 = "set bindir = $bindir"
set old3 = 'set[ 	][ 	]*mandir[ 	][ 	]*=.*$'
set new3 = "set mandir = $mandir"
set old4 = 'set[ 	][ 	]*sharedir[ 	][ 	]*=.*$'
set new4 = "set sharedir = $sharedir"
$sedcmd -e "s#$old0#$new0#" -e "s#$old1#$new1#" -e "s#$old2#$new2#" -e "s#$old3#$new3#" -e "s#$old4#$new4#" < $dir/vapor-setup.csh >! $dir/vapor-setup.tmp
/bin/mv $dir/vapor-setup.tmp $dir/vapor-setup.csh


set old0 = 'root=.*$'
set new0 = "root=$vapor_root"
set old1 = 'lib_search_dirs=.*$'
set new1 = "lib_search_dirs=$lib_search_dirs"
set old2 = 'bindir=.*$'
set new2 = "bindir=$bindir"
set old3 = 'mandir=.*$'
set new3 = "mandir=$mandir"
set old4 = 'sharedir=.*$'
set new4 = "sharedir=$sharedir"
$sedcmd -e "s#$old0#$new0#" -e "s#$old1#$new1#" -e "s#$old2#$new2#" -e "s#$old3#$new3#" -e "s#$old4#$new4#"< $dir/vapor-setup.sh >! $dir/vapor-setup.tmp
/bin/mv $dir/vapor-setup.tmp $dir/vapor-setup.sh


#
# Set DT_RUNPATH for all binary executables and shared libs on Linux
# platforms
#
# N.B. patchelf program seems to break stripped libraries. So we skip
# them. Fortunately, only need to set RUNPATH for libs that depend
# on other libs, and none of the stripped libs have recursive
# dependencies. N.B. patchelf doesn't work with symlinks either.
#
if ("$arch" == "Linux") then
    foreach f ($directory/bin/*)
        file -L $f | grep -q ELF 
        if ( $status == 0 && ! -l $f && $f != $directory/bin/patchelf) then
            $directory/bin/patchelf --set-rpath $directory/lib $f
        endif
    end

	#
	# now do libraries  and plugins
	# N.B. Apparent bug in patchelf prevents editing of libexpat
	# and libpng. Fortunately, these libraries do not have 
	# dependencies in vapor installation tree
	#
    foreach f (`find $directory/lib $directory/plugins -name \*.so -o -name \*.so.\*`)
        file -L $f | grep -q ELF 
        if ( $status == 0 && ! -l $f && $f !~ *libexpat.so* && $f !~ *libpng*) then
            $directory/bin/patchelf --set-rpath $directory/lib $f
        endif
    end
endif


exit 0
