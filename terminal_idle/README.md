# This is sample gif animation collection

## These are available for free download, I just included these here as a sample and a couple of my favorites!

**These make best use with following signal trap:**
```
terminal_idle() {
	local wallpaper_dir="~/Pictures/Wallpapers/terminal_idle"
	local image bg
	image=$(find "$wallpaper_dir" -maxdepth 1 -type f | shuf -n 1)
	bg=$(basename "$image" | grep -oP '#[0-9A-Fa-f]{6}')
	bg="${bg:-#FFFFFF}"

	printf '\e[?1049h\e]11;%s\a' "$bg"
	chafa "$image" -w 9 --bg "${bg#\#}"
	printf '\e]111\a\e[?1049l'
}
trap terminal_idle SIGALRM
```
