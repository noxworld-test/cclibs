#!/bin/bash
#
cd `dirname $0`

source ../../run_header.sh

# PREFUNC tests

$cctest "global csv_format $csv_format" "read prefunc.cct"

>&2 echo $0 complete

# EOF