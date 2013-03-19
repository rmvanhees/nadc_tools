#!/bin/bash
#
rm -rf build

files=$(find . -name '*~')
[ ${#files} -gt 0 ] && rm $files

files=$(find . -name '*.o')
[ ${#files} -gt 0 ] && rm $files

exit 0
