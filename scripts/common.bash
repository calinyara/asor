
function for_each_unittest()
{
	local unittests="$1"
	local cmd="$2"
	local testname
	local smp
	local kernel
	local opts
	local groups
	local arch
	local check
	local accel
	local timeout

	exec {fd}<"$unittests"

	while read -r -u $fd line; do
		if [[ "$line" =~ ^\[(.*)\]$ ]]; then
			"$cmd" "$testname" "$groups" "$smp" "$kernel" "$opts" "$arch" "$check" "$accel" "$timeout"
			testname=${BASH_REMATCH[1]}
			smp=1
			kernel=""
			opts=""
			groups=""
			arch=""
			check=""
			accel=""
			timeout=""
		elif [[ $line =~ ^file\ *=\ *(.*)$ ]]; then
			kernel=$TEST_DIR/${BASH_REMATCH[1]}
		elif [[ $line =~ ^smp\ *=\ *(.*)$ ]]; then
			smp=${BASH_REMATCH[1]}
		elif [[ $line =~ ^extra_params\ *=\ *(.*)$ ]]; then
			opts=${BASH_REMATCH[1]}
		elif [[ $line =~ ^groups\ *=\ *(.*)$ ]]; then
			groups=${BASH_REMATCH[1]}
		elif [[ $line =~ ^arch\ *=\ *(.*)$ ]]; then
			arch=${BASH_REMATCH[1]}
		elif [[ $line =~ ^check\ *=\ *(.*)$ ]]; then
			check=${BASH_REMATCH[1]}
		elif [[ $line =~ ^accel\ *=\ *(.*)$ ]]; then
			accel=${BASH_REMATCH[1]}
		elif [[ $line =~ ^timeout\ *=\ *(.*)$ ]]; then
			timeout=${BASH_REMATCH[1]}
		fi
	done
	"$cmd" "$testname" "$groups" "$smp" "$kernel" "$opts" "$arch" "$check" "$accel" "$timeout"
	exec {fd}<&-
}
