#his sample chooses random 'screen saver'
terminal_idle() {
	local cmds=(
		"cmatrix -b -u 2"
		"cbonsai -l -i"
		"asciiquarium"
	)
	printf '\e]11;#000000\a'
	local cmd=${cmds[RANDOM % ${#cmds[@]}]}
	eval "$cmd"
	printf '\e]111\a'
}
trap terminal_idle SIGALRM

#this sample uses chafa animation:
terminal_idle() {
	printf '\e]11;#000000\a\e[?1049h'
	chafa ~/Pictures/Wallpapers/terminal-idle.gif -w 9 --bg black
	printf '\e[?1049l\e]111\a'
}
trap terminal_idle SIGALRM
