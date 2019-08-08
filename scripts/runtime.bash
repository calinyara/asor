: "${RUNTIME_arch_run?}"
: ${MAX_SMP:=$(getconf _NPROCESSORS_CONF)}
: ${TIMEOUT:=90s}

PASS() { echo -ne "\e[32mPASS\e[0m"; }
SKIP() { echo -ne "\e[33mSKIP\e[0m"; }
FAIL() { echo -ne "\e[31mFAIL\e[0m"; }

extract_summary()
{
    local cr=$'\r'
    tail -3 | grep '^SUMMARY: ' | sed 's/^SUMMARY: /(/;s/'"$cr"'\{0,1\}$/)/'
}

# We assume that QEMU is going to work if it tried to load the kernel
premature_failure()
{
    local log="$(eval $(get_cmdline _NO_FILE_4Uhere_) 2>&1)"

    echo "$log" | grep "_NO_FILE_4Uhere_" |
        grep -q -e "could not load kernel" -e "error loading" &&
        return 1

    RUNTIME_log_stderr <<< "$log"

    echo "$log"
    return 0
}

get_cmdline()
{
    local kernel=$1
    echo "TESTNAME=$testname TIMEOUT=$timeout ACCEL=$accel $RUNTIME_arch_run $kernel -smp $smp $opts"
}

skip_nodefault()
{
    [ "$run_all_tests" = "yes" ] && return 1
    [ "$STANDALONE" != "yes" ] && return 0

    while true; do
        read -r -p "Test marked not to be run by default, are you sure (y/N)? " yn
        case $yn in
            "Y" | "y" | "Yes" | "yes")
                return 1
                ;;
            "" | "N" | "n" | "No" | "no" | "q" | "quit" | "exit")
                return 0
                ;;
        esac
    done
}

function print_result()
{
    # output test results in a TAP format
    # https://testanything.org/tap-version-13-specification.html

    local status="$1"
    local testname="$2"
    local summary="$3"
    local reason="$4"

    if [ -z "$reason" ]; then
        echo "`$status` $testname $summary"
    else
        echo "`$status` $testname ($reason)"
    fi
}

function run()
{
    local testname="$1"
    local groups="$2"
    local smp="$3"
    local kernel="$4"
    local opts="$5"
    local arch="$6"
    local check="${CHECK:-$7}"
    local accel="${ACCEL:-$8}"
    local timeout="${9:-$TIMEOUT}" # unittests.cfg overrides the default

    if [ -z "$testname" ]; then
        return
    fi

    if [ -n "$only_tests" ] && ! grep -qw "$testname" <<<$only_tests; then
        return
    fi

    if [ -n "$only_group" ] && ! grep -qw "$only_group" <<<$groups; then
        return
    fi

    if [ -z "$only_group" ] && grep -qw "nodefault" <<<$groups &&
            skip_nodefault; then
        print_result "SKIP" $testname "" "test marked as manual run only"
        return;
    fi

    if [ -n "$arch" ] && [ "$arch" != "$ARCH" ]; then
        print_result "SKIP" $testname "" "$arch only"
        return 2
    fi

    # check a file for a particular value before running a test
    # the check line can contain multiple files to check separated by a space
    # but each check parameter needs to be of the form <path>=<value>
    for check_param in "${check[@]}"; do
        path=${check_param%%=*}
        value=${check_param#*=}
        if [ "$path" ] && [ "$(cat $path)" != "$value" ]; then
            print_result "SKIP" $testname "" "$path not equal to $value"
            return 2
        fi
    done

    last_line=$(premature_failure > >(tail -1)) && {
        print_result "SKIP" $testname "" "$last_line"
        return 77
    }

    cmdline=$(get_cmdline $kernel)
    if grep -qw "migration" <<<$groups ; then
        cmdline="MIGRATION=yes $cmdline"
    fi
    if [ "$verbose" = "yes" ]; then
        echo $cmdline
    fi

    # extra_params in the config file may contain backticks that need to be
    # expanded, so use eval to start qemu.  Use "> >(foo)" instead of a pipe to
    # preserve the exit status.
    summary=$(eval $cmdline 2> >(RUNTIME_log_stderr) \
                             > >(tee >(RUNTIME_log_stdout $kernel) | extract_summary))
    ret=$?
    [ "$STANDALONE" != "yes" ] && echo > >(RUNTIME_log_stdout $kernel)

    if [ $ret -eq 0 ]; then
        print_result "PASS" $testname "$summary"
    elif [ $ret -eq 77 ]; then
        print_result "SKIP" $testname "$summary"
    elif [ $ret -eq 124 ]; then
        print_result "FAIL" $testname "" "timeout; duration=$timeout"
    elif [ $ret -gt 127 ]; then
        print_result "FAIL" $testname "" "terminated on SIG$(kill -l $(($ret - 128)))"
    else
        print_result "FAIL" $testname "$summary"
    fi

    return $ret
}

#
# Probe for MAX_SMP, in case it's less than the number of host cpus.
#
# This probing currently only works for ARM, as x86 bails on another
# error first. Also, this probing isn't necessary for any ARM hosts
# running kernels later than v4.3, i.e. those including ef748917b52
# "arm/arm64: KVM: Remove 'config KVM_ARM_MAX_VCPUS'". So, at some
# point when maintaining the while loop gets too tiresome, we can
# just remove it...
while $RUNTIME_arch_run _NO_FILE_4Uhere_ -smp $MAX_SMP \
		|& grep -qi 'exceeds max CPUs'; do
	MAX_SMP=$((MAX_SMP >> 1))
done
