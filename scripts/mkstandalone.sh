#!/usr/bin/env bash

if [ ! -f config.mak ]; then
	echo "run ./configure && make first. See ./configure -h"
	exit 1
fi
source config.mak
source scripts/common.bash

temp_file ()
{
	local var="$1"
	local file="${2:--}"

	echo "$var=\`mktemp\`"
	echo "cleanup=\"\$$var \$cleanup\""
	echo "base64 -d << 'BIN_EOF' | zcat > \$$var || exit 2"

	gzip -c "$file" | base64

	echo "BIN_EOF"
	echo "chmod +x \$$var"
}

config_export ()
{
	echo "export $(grep ^${1}= config.mak)"
}

generate_test ()
{
	local args=()
	for arg in "${@}"; do
		args+=("$(printf "%q" "$arg")")
	done

	echo "#!/usr/bin/env bash"
	echo "export STANDALONE=yes"
	echo "export ENVIRON_DEFAULT=yes"
	echo "export HOST=\$(uname -m | sed -e 's/i.86/i386/;s/arm.*/arm/;s/ppc64.*/ppc64/')"
	echo "export PRETTY_PRINT_STACKS=no"

	config_export ARCH
	config_export ARCH_NAME
	config_export PROCESSOR

	echo "echo BUILD_HEAD=$(cat build-head)"

	if [ ! -f $kernel ]; then
		echo 'echo "skip '"$testname"' (test kernel not present)"'
		echo 'exit 2'
		return
	fi

	echo "trap 'rm -f \$cleanup' EXIT"

	if [ "$FIRMWARE" ]; then
		temp_file FIRMWARE "$FIRMWARE"
		echo 'export FIRMWARE'
	fi

	if [ "$ERRATATXT" ]; then
		temp_file ERRATATXT "$ERRATATXT"
		echo 'export ERRATATXT'
	fi

	temp_file bin "$kernel"
	args[3]='$bin'

	(echo "#!/usr/bin/env bash"
	 cat scripts/arch-run.bash "$TEST_DIR/run") | temp_file RUNTIME_arch_run

	echo "exec {stdout}>&1"
	echo "RUNTIME_log_stdout () { cat >&\$stdout; }"
	echo "RUNTIME_log_stderr () { cat >&2; }"

	cat scripts/runtime.bash

	echo "run ${args[@]}"
}

function mkstandalone()
{
	local testname="$1"

	if [ -z "$testname" ]; then
		return
	fi

	if [ -n "$one_testname" ] && [ "$testname" != "$one_testname" ]; then
		return
	fi

	standalone=tests/$testname

	generate_test "$@" > $standalone

	chmod +x $standalone
	echo Written $standalone.
}

trap 'rm -f $cfg' EXIT
cfg=$(mktemp)

unittests=$TEST_DIR/unittests.cfg
one_kernel="$1"

if [ "$one_kernel" ]; then
	[ ! -f $one_kernel ] && {
		echo "$one_kernel doesn't exist"
		exit 1
	}

	one_kernel_base=$(basename $one_kernel)
	one_testname="${2:-${one_kernel_base%.*}}"

	if grep -q "\[$one_testname\]" $unittests; then
		sed -n "/\\[$one_testname\\]/,/^\\[/p" $unittests \
			| awk '!/^\[/ || NR == 1' > $cfg
	else
		echo "[$one_testname]" > $cfg
		echo "file = $one_kernel_base" >> $cfg
	fi
else
	cp -f $unittests $cfg
fi

mkdir -p tests

for_each_unittest $cfg mkstandalone
