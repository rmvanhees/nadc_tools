#!/bin/bash
#
array=(${1//" "/ })
if [ ${#array[@]} -lt 3 ]; then
    echo "Usage: $0 OutFile Dir_idl_progs Dir_DataDir <list of idl-progs>"
    exit 1
fi

for indx in "${!array[@]}"
do
    if [ $indx -eq 0 ]
    then
	OutFile=${array[indx]}
    elif  [ $indx -eq 1 ]
    then
	Dir_IDL_NADC=${array[indx]}
    elif  [ $indx -eq 2 ]
    then
	Dir_DataDir=${array[indx]}
	echo "IF FILE_WHICH( 'setup_nadc.idl' ) EQ '' THEN BEGIN &\$" > $OutFile
	echo "  nadc_dir = '$PWD' &\$" >> $OutFile
	echo "  IF FILE_WHICH( nadc_dir, 'setup_nadc.idl' ) EQ '' THEN BEGIN &\$" >> $OutFile
	echo "     nadc_dir = '$Dir_IDL_NADC' &\$" >> $OutFile
	echo "  ENDIF &\$" >> $OutFile
	echo "  !path = !path + ':' + EXPAND_PATH( '+' + nadc_dir ) &\$" >> $OutFile
	echo "ENDIF" >> $OutFile
	echo "" >> $OutFile
    else
	file=${array[indx]}
	if [ ${file%.pro} != $file ]; then
	    echo "resolve_routine, \"${file%.pro}\", /EITHER" >> $OutFile
	fi
    fi
done

echo "" >> $OutFile
echo "DEFS_NADC, \"$Dir_DataDir\"" >> $OutFile
exit 0
