echo Generating vapor installer
echo
echo NOTE: Due to the oddities of conda, this should not be run from the directory from which you also build.
echo NOTE: If you do, make sure to remove any generated files before calling conda build again.
echo

CHANNEL=__vapor_install_temp_vapor-channel
mkdir -p $CHANNEL/linux-64/
cp `find $CONDA_PREFIX -name 'vapor*.bz2'` $CHANNEL/linux-64/
conda index $CHANNEL

cat > install_vapor.sh << EOF
[ "" = "\$CONDA_PREFIX" ] && echo This installer requires conda && exit 1
[ "" != "\`conda list | grep -v '^#'\`" ] && echo Please run in empty conda environment && exit 1
tail -n +___START_LINE___ "\$0" | tar -xf -
conda install -c conda-forge -c file:/\`pwd\`/$CHANNEL vapor
rm -rf $CHANNEL
echo
echo Vapor installed
echo Examples can be found at \$CONDA_PREFIX/lib/python3.9/site-packages/vapor/examples
echo Examples jupyter notebooks can be found at \$CONDA_PREFIX/lib/python3.9/site-packages/vapor/example_notebooks
echo
exit

___START_EMBEDDED_DATA___
EOF

LINE=`grep -n ___START_EMBEDDED_DATA___ install_vapor.sh | tail -n1 | cut -f1 -d:`
LINE=$(($LINE+1))
sed -i "s/___START_LINE___/$LINE/g" install_vapor.sh

tar -cf - $CHANNEL >> install_vapor.sh

rm -rf $CHANNEL

echo Done
