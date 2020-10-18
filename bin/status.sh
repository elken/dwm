#!/bin/bash

normal="\x01"
red="\x05"
yellow="\x06"
green="\x07"
blue="\x0A"

getBattery() {
    perc=$(acpi -b | awk '/Battery/ {print $4}' | cut -d% -f1)
    time=$(acpi -b | awk '/Battery/ {print " (" substr($5,1,5)")"}')
    is_charging=$(acpi -a | awk '/Adapter/ {print $3}')
    battery=""

    if [ "${is_charging}" != "on-line" ]; then
        if [ ${perc} -le "25" ]; then
            battery="${red}${time}"
        elif [ ${perc} -le "50" ]; then
            battery="${yellow}${time}"
        elif [ ${perc} -le "75" ]; then
            battery="${yellow}${time}"
        elif [ ${perc} -le "100" ]; then
            battery="${green}${time}"
        fi
    fi

    echo -ne "${battery}"
}

getCPU() {
    cpu=$(mpstat -P ALL 1 1 | awk '/Average:/ && $2 ~ /all/ {print $3}')
    if [ $(bc <<< "${cpu}<25") == 1 ]; then
        echo -ne "${green}"
    elif [ $(bc <<< "${cpu}<75") == 1 ]; then
        echo -ne "${yellow}"
    else
        echo -ne "${red}"
    fi
}

getMEM() {
    if [ "$(free -V | awk '{print $4}' | cut -d. -f3)" -le 9 ]; then
        mem="$(free -m | awk '/-\/+/ {print $3}')"
        total=$(free -m | awk '/Mem:/ {print $2}')
    else
        total=$(free -m | awk '/Mem:/ {print $2}')
        mem="$(bc <<< ${total}-$(free -m | awk '/Mem:/ {print $7}'))"
    fi
    if [ $(bc <<< "${mem}<$(echo ${total}* .25 | bc)") == 1 ]; then
       echo -ne "${green}"
    elif [ $(bc <<< "${mem}<$(echo ${total}* .5 | bc)") == 1 ]; then
       echo -ne "${yellow}"
    else
       echo -ne "${red}"
    fi
 }

getUpdates() {
    count="$(checkupdates | wc -l)"
    if [ $count -le 0 ]; then
        echo -ne ""
    else
        echo -ne "${blue}  ${normal}${count} "
    fi
}

getSound() {
    is_muted=$(amixer get Master | awk '/%/ {gsub(/[\[\]]/,""); print $6}' | tail -1)
    cur_device=$(pactl list sinks | awk '/Active Port:/ {print substr($3,15)}' | grep -v "^$")
    if [ ${cur_device} == "headphones" ]; then
        out_device=("  " " ﳌ ")
    else
        out_device=(" 蓼 " " 遼 ")
    fi

    if [ ${is_muted} == "on" ]; then
        echo -ne "${green}${out_device[0]}"
    else
        echo -ne "${red}${out_device[1]}"
    fi

}

getMusic() {
    music_str="" 
    eval $(sp eval)

    # if [ "$(mpc current)" != "" ]; then
    #     if [ $(mpc | awk '/^\[/ {print $1}') == "[playing]" ]; then
    music_str+="${blue}"
    #     else
    #         music_str+="${blue}"
    #     fi

    #     music_str+=" $(mpc current) ($(mpc | head -2 | tail -1 | awk '{print $3}'))"
    # elif [ "$($HOME/bin/clem-np.sh)" != "" ]; then
    #     local status="$($HOME/bin/clem-np.sh)"
    #     if [ "$(echo $status | cut -d '|' -f1)" == "Playing" ]; then
    #         music_str+="${blue}"
    #     else
    #         music_str+="${blue}"
    #     fi

    #     music_str+="$(echo $status | cut -d '|' -f2-)"
    # fi
    music_str+="${SPOTIFY_TITLE} - ${SPOTIFY_ARTIST}"
    echo -ne "${music_str} "
}

getTime() {
    tme="$(date '+%A %d/%m/%y %H:%M')"
    echo -ne "${blue}  ${normal}${tme} "
}

while true; do
    xsetroot -name "$(getUpdates) $(getBattery) $(getSound) $(getCPU)  $(getMEM)  $(getTime) "
done
