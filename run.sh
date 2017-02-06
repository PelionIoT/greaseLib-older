#!/bin/bash 

SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink                                                                                             
  SELF="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located              
done

THIS_DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"


LD_LIBRARY_PATH=${THIS_DIR}/deps/build/lib "$@"
