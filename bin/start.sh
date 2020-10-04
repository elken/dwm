exec > ~/.logs/xsession 2>&1
export LANG="en_GB.UTF-8"
export LANGUAGE="en_GB.UTF-8"
setxkbmap -option terminate:ctrl_alt_bksp
setxkbmap -option ctrl:nocaps
setxkbmap gb
xset -dpms
xset s off
xsetroot -cursor_name left_ptr
xset +fp ~/.local/share/fonts
xset fp rehash
xrdb ~/.Xresources
[ ! -e /tmp/dwm.fifo ] && mkfifo /tmp/dwm.fifo

nitrogen --restore &
nm-applet &
dunst &
picom &
gnome-screensaver &

~/.dwm/bin/status.sh 2> ~/.logs/status &
~/.dwm/dwm 2> ~/.logs/dwm
