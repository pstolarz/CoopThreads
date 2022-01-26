#!/bin/bash

err_file=$(mktemp)

clean_up() {
  rm -f $err_file
  exit 1
}
trap clean_up SIGHUP SIGINT SIGTERM

rc=0
for test_exe in $*; do
    test_out=$(echo $test_exe | sed -E 's/^(t[0-9]+)_.*/\1.out/')

    if [[ -e $test_out ]]; then
        i=0
        while : ; do
            rc=0
            ./$test_exe | ./test_chk.pl $test_out 2>$err_file && break
            rc=$?
            [[ $(( i++ )) -ge 5 ]] && break
        done
    else \
        ./$test_exe 1>/dev/null 2>$err_file
        rc=$?
    fi

    if [[ $rc -eq 0 ]]; then
        echo "TEST $test_exe: OK";
    else
        echo "TEST $test_exe: FAILED"
        cat $err_file
        break
    fi
done

rm $err_file
exit $rc
