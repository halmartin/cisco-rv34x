#/bin/sh
FirstVers=$1
SeconVers=$2

echo "$FirstVers $SeconVers" | /usr/bin/awk '#--re-interval is used to activate regexec POSIX quantifiers

### AWK Program

{

  # Chech args
  if ( $1 !~ /^([[:digit:]]{1,4}\.){1,3}[[:digit:]]{1,4}$/ ) {
    print "ERR: First arg "$1" is not a valid version number"
    exit 1
  }
  if ( $2 !~ /^([[:digit:]]{1,4}\.){1,3}[[:digit:]]{1,4}$/ ) {
    print "ERR: Second arg "$2" is not a valid version number"
    exit 1
  }

  # Split each arg in array
  split($1, FirstVersArr, ".")
  split($2, SeconVersArr, ".")

  for(a = 1; a <=4 ; a++) {
      #print FirstVersArr[a] " " SeconVersArr[a]
      if( FirstVersArr[a] == SeconVersArr[a])
        continue;
      if( FirstVersArr[a] > SeconVersArr[a]) {
        print $1" is greater than "$2
        exit 0
      } else {
        print $2" is greater than "$1
        exit 2
      }
  }
  #print $1" is same as "$2

 exit 1

}'
