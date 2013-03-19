set (MYSTR "PostgreSQL 9.13.14")
message (STATUS "MYSTR: ${MYSTR}" )

string(REGEX REPLACE "PostgreSQL[ \t]+([0-9]+.[0-9]+.[0-9]+).*$" "\\1" 
  VERSION ${MYSTR})
message (STATUS "VERSION: ${VERSION}" )