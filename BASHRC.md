# Example signal handlers

## This sample chooses random 'screen saver'
```
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
```
---

## this sample uses chafa animation:
```
terminal_idle() {
	printf '\e[?1049h\e]11;#000000\a'
	chafa ~/Pictures/Wallpapers/terminal-idle.gif -w 9 --bg black
	printf '\e]111\a\e[?1049l'
}
trap terminal_idle SIGALRM
```
---

## This sample uses terminal_idle collection of images in your Pictures/Wallpaper directory
** NOTE: flexible bacground support: <filename>.#FFCC00.gif  
script extracts configurable background from filename and uses this as terminal and chafa background **
```
terminal_idle() {
	local wallpaper_dir="/home/marek/Pictures/Wallpapers/terminal_idle"
	local image
	image=$(find "$wallpaper_dir" -maxdepth 1 -type f | shuf -n 1)

	local bg
	bg=$(basename "$image" | grep -oP '#[0-9A-Fa-f]{6}')
	bg="${bg:-#FFFFFF}"

	printf '\e[?1049h\e]11;%s\a' "$bg"
	chafa "$image" -w 9 --bg "${bg#\#}"
	printf '\e]111\a\e[?1049l'
}
trap terminal_idle SIGALRM
```
