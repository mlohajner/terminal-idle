#!/usr/bin/env bash
# ~/.local/bin/terminal-idle.sh

# Correlates with RestartSec in terminal-idle.service
INACTIVITY_PERIOD=300
NOW=$(date +%s)

for pts in /dev/pts/[0-9]*; do
	n=$(basename "$pts")
	atime=$(stat -c %X "$pts" 2>/dev/null) || continue
	idle=$(( NOW - atime ))
	(( idle < INACTIVITY_PERIOD )) && continue   # recent activity -> skip
	# tpgid = PID foreground process
	tpgid=$(ps -o tpgid= -t "pts/$n" 2>/dev/null | head -1 | tr -d ' ')
	[[ -z "$tpgid" || "$tpgid" == "-1" ]] && continue
	comm=$(ps -o comm= -p "$tpgid" 2>/dev/null)
	case "$comm" in
		bash|-bash|zsh|-zsh|sh|-sh)
			sigcgt=$(grep '^SigCgt:' "/proc/$tpgid/status" 2>/dev/null | awk '{print $2}')
			[[ -z "$sigcgt" ]] && continue
			# no trap -> no signal
			if (( ( 0x$sigcgt >> 13 ) & 1 )); then
				kill -SIGALRM "$tpgid" 2>/dev/null
			fi
			;;
	esac
done
