#!/bin/bash
#
cd `dirname $0`

source ../../run_header.sh

# Direct tests

$cctest "global csv_format $csv_format" "read direct.cct"

>&2 echo $0 complete

# EOF