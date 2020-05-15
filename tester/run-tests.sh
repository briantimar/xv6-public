#! /usr/bin/env bash 

# run_test testdir testnumber
run_test () {
    local testdir=$1
    local outputdir=$2
    local testnum=$3
    local verbose=$4

    # pre: execute this after before the test is done, to set up
    local prefile=$testdir/$testnum.pre
    if [[ -f $prefile ]]; then
    
	eval $(cat $prefile)
	if (( $verbose == 1 )); then
	    echo -n "pre-test:  "
	    cat $prefile
	fi
    fi
    local testfile=$testdir/$testnum.run
    # if (( $verbose == 1 )); then
	# echo -n "test:      "
	# cat $testfile
    # fi
    eval $(cat $testfile) > $outputdir/$testnum.out 2> $outputdir/$testnum.err
    echo $? > $outputdir/$testnum.rc

    # post: execute this after the test is done, to clean up
    local postfile=$testdir/$testnum.post
    if [[ -f $postfile ]]; then
	eval $(cat $postfile)
	if (( $verbose == 1 )); then
	    echo -n "post-test: "
	    cat $postfile
	fi
    fi
    return 
}

print_error_message () {
    local testnum=$1
    local contrunning=$2
    local filetype=$3
    builtin echo -e "\e[31mtest $testnum: $filetype incorrect\e[0m"
    echo "  what results should be found in file: $testdir/$testnum.$filetype"
    echo "  what results produced by your program: tests-out/$testnum.$filetype"
    echo "  compare the two using diff, cmp, or related tools to debug, e.g.:"
    echo "  prompt> diff $testdir/$testnum.$filetype tests-out/$testnum.$filetype"
    if (( $contrunning == 0 )); then
	exit 1
    fi
}

# check_test testdir testnumber contrunning out/err
check_test () {
    local testdir=$1
    local outputdir=$2
    local testnum=$3
    local contrunning=$4
    local filetype=$5

    # option to use cmp instead?
    returnval=$(diff $testdir/$testnum.$filetype $outputdir/$testnum.$filetype)
    if (( $? == 0 )); then
	echo 0
    else
	echo 1
    fi
}

# run_and_check testdir testnumber contrunning verbose printerror
#   testnumber: the test to run and check
#   printerrer: if 1, print an error if test does not exist
run_and_check () {
    local testdir=$1
    local outputdir=$2
    local testnum=$3
    local contrunning=$4
    local verbose=$5
    local failmode=$6


    if [[ ! -f $testdir/$testnum.run ]]; then
	if (( $failmode == 1 )); then
	    echo "test $testnum does not exist" >&2; exit 1
	fi
	exit 0
    
    fi
    if (( $verbose == 1 )); then
	echo "running test $testdir/$testnum with output $outputdir:"
	cat $testdir/$testnum.desc
    fi
    run_test $testdir $outputdir $testnum $verbose
    rccheck=$(check_test $testdir $outputdir $testnum $contrunning rc)
    outcheck=$(check_test $testdir $outputdir $testnum $contrunning out)
    errcheck=$(check_test $testdir $outputdir $testnum $contrunning err)
    othercheck=0
    if [[ -f $testdir/$testnum.other ]]; then
	othercheck=$(check_test $testdir $testnum $contrunning other)
    fi
    # echo "results: outcheck:$outcheck errcheck:$errcheck"
    if (( $rccheck == 0 )) && (( $outcheck == 0 )) && (( $errcheck == 0 )) && (( $othercheck == 0 )); then
	echo "test $testnum: passed"
	if (( $verbose == 1 )); then
	    echo ""
	fi
    else
	if (( $rccheck == 1 )); then
	    print_error_message $testnum $contrunning rc
	fi
	if (( $outcheck == 1 )); then
	    print_error_message $testnum $contrunning out
	fi
	if (( $errcheck == 1 )); then
	    print_error_message $testnum $contrunning err
	fi
	if (( $othercheck == 1 )); then
	    print_error_message $testnum $contrunning other
	fi
    fi
}

# usage: call when args not parsed, or when help needed
usage () {
    echo "usage: run-tests.sh [-h] [-v] [-t test] [-c] [-s] [-d testdir]"
    echo "  -h                help message"
    echo "  -v                verbose"
    echo "  -t n              run only test n"
    echo "  -c                continue even after failure"
    echo "  -s                skip pre-test initialization"
    echo "  -d testdir        run tests from testdir"
    echo "  -o outputdir      output test results to outputdir"
    return 0
}

#
# main program
#
verbose=0
testdir="tests"
outputdir="tests-out"
contrunning=0
skippre=0
specific=""

args=`getopt hvsct:d:o: $*`
if [[ $? != 0 ]]; then
    usage; exit 1
fi

set -- $args
for i; do
    case "$i" in
    -h)
	usage
	exit 0
        shift;;
    -v)
        verbose=1
        shift;;
    -c)
        contrunning=1
        shift;;
    -s)
        skippre=1
        shift;;
    -t)
        specific=$2
	shift
	number='^[0-9]+$'
	if ! [[ $specific =~ $number ]]; then
	    usage
	    echo "-t must be followed by a number" >&2; exit 1
	fi
        shift;;
    -d)
        testdir=$2
	shift
        shift;;
    -o)
        outputdir=$2
        shift
        shift;;
    --)
        shift; break;;
    esac
done

echo "Loading tests from directory $testdir"
# need a test directory; must be named "tests-out"
if [[ ! -d $outputdir ]]; then
    mkdir -p $outputdir
fi

# do a one-time setup step
if (( $skippre == 0 )); then
    if [[ -f $testdir/pre ]]; then
	echo "doing one-time pre-test (use -s to suppress)"
	source $testdir/pre
	if (( $? != 0 )); then
	    echo "pre-test: failed"
	    exit 1
	fi
	echo ""
    fi
fi

# run just one test
if [[ $specific != "" ]]; then
    run_and_check $testdir $outputdir $specific $contrunning $verbose 1
    exit 0
fi

# run all tests
(( testnum = 1 ))
while true; do
    run_and_check $testdir $outputdir $testnum $contrunning $verbose 0
    (( testnum = $testnum + 1 ))
done

exit 0
