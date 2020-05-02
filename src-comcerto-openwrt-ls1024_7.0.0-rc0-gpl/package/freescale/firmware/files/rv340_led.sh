#!/bin/sh

## LED <> SYSFS Mappings
WLAN5G_LED_GREEN="c2krv340:green:wlan5g"
DIAG_LED_RED="c2krv340:red:diag"
WLAN2_4G_LED_GREEN="c2krv340:green:wlan2g"
POWER_LED_GREEN="c2krv340:green:power"
USB2_LED_AMBER="c2krv340:amber:usb2"
USB2_LED_GREEN="c2krv340:green:usb2"
USB1_LED_AMBER="c2krv340:amber:usb1"
USB1_LED_GREEN="c2krv340:green:usb1"
DMZ_LED_GREEN="c2krv340:green:dmz"
VPN_LED_AMBER="c2krv340:amber:vpn"
VPN_LED_GREEN="c2krv340:green:vpn"

LED_SYSFS="/sys/class/leds"

DELAY_SLOW=1000
DELAY_FAST=333

LED_ON=255
LED_OFF=0

LED="$1"
STATE="$2"

#pgrep -f rv340_led.sh | xargs kill -9 > /dev/null  2>&1

case $LED in

  wlan5g)
    # WLAN 5G ON/OFF
    case $STATE in
      on)
        echo "WLAN 5G LED ON"
        echo "default-on" >${LED_SYSFS}/${WLAN5G_LED_GREEN}/trigger
        echo "${LED_ON}" >${LED_SYSFS}/${WLAN5G_LED_GREEN}/brightness
        for INDEX in 0 1 2 3; do
          echo "netdev" >${LED_SYSFS}/${WLAN5G_LED_GREEN}_${INDEX}/trigger
          echo "wl1$([ ${INDEX} -gt 0 ] && echo .${INDEX})" >${LED_SYSFS}/${WLAN5G_LED_GREEN}_${INDEX}/device_name
          echo "50" >${LED_SYSFS}/${WLAN5G_LED_GREEN}_${INDEX}/interval
          echo "tx rx" >${LED_SYSFS}/${WLAN5G_LED_GREEN}_${INDEX}/mode
        done
        ;;
      off)
        echo "WLAN 5G LED OFF"
        for INDEX in 0 1 2 3; do
          echo "none" >${LED_SYSFS}/${WLAN5G_LED_GREEN}_${INDEX}/trigger
        done
        echo "none" >${LED_SYSFS}/${WLAN5G_LED_GREEN}/trigger
        ;;
      *)
        echo "Incorrect Option $STATE for $LED"
        ;;
    esac
    echo "WLAN 5G done."
    ;;

  diag)
    # Diag for Firmware Operation
    case $STATE in
      slow)
        echo "Diag LED slow blinking for Firmware Operation..."
        echo "timer" >${LED_SYSFS}/${DIAG_LED_RED}/trigger
        echo "${DELAY_SLOW}" >${LED_SYSFS}/${DIAG_LED_RED}/delay_on
        echo "${DELAY_SLOW}" >${LED_SYSFS}/${DIAG_LED_RED}/delay_off
        ;;
      fast)
        echo "Diag LED fast blinking for Firmware Operation..."
        echo "timer" >${LED_SYSFS}/${DIAG_LED_RED}/trigger
        echo "${DELAY_FAST}" >${LED_SYSFS}/${DIAG_LED_RED}/delay_on
        echo "${DELAY_FAST}" >${LED_SYSFS}/${DIAG_LED_RED}/delay_off
        ;;
      on)
        echo "Diag LED ON"
        echo "default-on" >${LED_SYSFS}/${DIAG_LED_RED}/trigger
        echo "${LED_ON}" >${LED_SYSFS}/${DIAG_LED_RED}/brightness
        ;;
      off)
        echo "Diag LED OFF"
        echo "none" >${LED_SYSFS}/${DIAG_LED_RED}/trigger
        ;;
      *)
        echo "Incorrect Option STATE: $STATE for LED: $LED"
        ;;
    esac
    echo "diag done."
    ;;

  wlan2.4)
    # WLAN 2.4G ON/OFF
    case $STATE in
      on)
        echo "WLAN 2.4G LED ON"
        echo "default-on" >${LED_SYSFS}/${WLAN2_4G_LED_GREEN}/trigger
        echo "${LED_ON}" >${LED_SYSFS}/${WLAN2_4G_LED_GREEN}/brightness
        for INDEX in 0 1 2 3; do
          echo "netdev" >${LED_SYSFS}/${WLAN2_4G_LED_GREEN}_${INDEX}/trigger
          echo "wl0$([ ${INDEX} -gt 0 ] && echo .${INDEX})" >${LED_SYSFS}/${WLAN2_4G_LED_GREEN}_${INDEX}/device_name
          echo "50" >${LED_SYSFS}/${WLAN2_4G_LED_GREEN}_${INDEX}/interval
          echo "tx rx" >${LED_SYSFS}/${WLAN2_4G_LED_GREEN}_${INDEX}/mode
        done
        ;;
      off)
        echo "WLAN 2.4G LED OFF"
        for INDEX in 0 1 2 3; do
          echo "none" >${LED_SYSFS}/${WLAN2_4G_LED_GREEN}_${INDEX}/trigger
        done
        echo "none" >${LED_SYSFS}/${WLAN2_4G_LED_GREEN}/trigger
        ;;
      *)
        echo "Incorrect Option $STATE for $LED"
        ;;
    esac
    echo "WLAN 2.4G done."
    ;;

  power)
    # Power ON/OFF
    case $STATE in
      on)
        echo "Power LED ON"
        echo "default-on" >${LED_SYSFS}/${POWER_LED_GREEN}/trigger
        echo "${LED_ON}" >${LED_SYSFS}/${POWER_LED_GREEN}/brightness
        ;;
      off)
        echo "Power LED OFF"
        echo "none" >${LED_SYSFS}/${POWER_LED_GREEN}/trigger
        ;;
      slow)
        echo "Power LED slow blinking for Bootup..."
        echo "timer" >${LED_SYSFS}/${POWER_LED_GREEN}/trigger
        echo "${DELAY_SLOW}" >${LED_SYSFS}/${POWER_LED_GREEN}/delay_on
        echo "${DELAY_SLOW}" >${LED_SYSFS}/${POWER_LED_GREEN}/delay_off
        ;;
      *)
        echo "Incorrect Option $STATE for $LED"
        ;;
    esac
    echo "power done."
    ;;

  usb2)
    # USB2 LED Control
    case $STATE in
      amber)
        echo "none" >${LED_SYSFS}/${USB2_LED_GREEN}/trigger
        echo "default-on" >${LED_SYSFS}/${USB2_LED_AMBER}/trigger
        echo "${LED_ON}" >${LED_SYSFS}/${USB2_LED_AMBER}/brightness
        ;;
      green|blink)
        echo "none" >${LED_SYSFS}/${USB2_LED_AMBER}/trigger
        echo "usbdev" >${LED_SYSFS}/${USB2_LED_GREEN}/trigger
        echo "50" >${LED_SYSFS}/${USB2_LED_GREEN}/activity_interval
        echo "1-1" >${LED_SYSFS}/${USB2_LED_GREEN}/device_name
        ;;
      off)
        echo "none" >${LED_SYSFS}/${USB2_LED_AMBER}/trigger
        echo "none" >${LED_SYSFS}/${USB2_LED_GREEN}/trigger
        ;;
      *)
        echo "Incorrect Option $STATE for $LED"
        ;;
    esac
    ;;

  usb1)
    # USB1 LED Control
    case $STATE in
      amber)
        echo "none" >${LED_SYSFS}/${USB1_LED_GREEN}/trigger
        echo "default-on" >${LED_SYSFS}/${USB1_LED_AMBER}/trigger
        echo "${LED_ON}" >${LED_SYSFS}/${USB1_LED_AMBER}/brightness
        ;;
      green|blink)
        echo "none" >${LED_SYSFS}/${USB1_LED_AMBER}/trigger
        echo "usbdev" >${LED_SYSFS}/${USB1_LED_GREEN}/trigger
        echo "50" >${LED_SYSFS}/${USB1_LED_GREEN}/activity_interval
        echo "3-1" >${LED_SYSFS}/${USB1_LED_GREEN}/device_name
        ;;
      off)
        echo "none" >${LED_SYSFS}/${USB1_LED_AMBER}/trigger
        echo "none" >${LED_SYSFS}/${USB1_LED_GREEN}/trigger
        ;;
      *)
        echo "Incorrect Option $STATE for $LED"
        ;;
    esac
    ;;

  dmz)
    # DMZ ON/OFF
    case $STATE in
      on)
        echo "DMZ LED ON"
        echo "default-on" >${LED_SYSFS}/${DMZ_LED_GREEN}/trigger
        echo "${LED_ON}" >${LED_SYSFS}/${DMZ_LED_GREEN}/brightness
        ;;
      off)
        echo "DMZ LED OFF"
        echo "none" >${LED_SYSFS}/${DMZ_LED_GREEN}/trigger
        ;;
      *)
        echo "Incorrect Option $STATE for $LED"
        ;;
    esac
    echo "DMZ done."
    ;;

  vpn)
    # VPN LED Control
    case $STATE in
      amber)
#        echo "VPN LED AMBER"
        echo "none" >${LED_SYSFS}/${VPN_LED_GREEN}/trigger
        echo "default-on" >${LED_SYSFS}/${VPN_LED_AMBER}/trigger
        echo "${LED_ON}" >${LED_SYSFS}/${VPN_LED_AMBER}/brightness
        ;;
      green)
#        echo "VPN LED GREEN"
        echo "none" >${LED_SYSFS}/${VPN_LED_AMBER}/trigger
        echo "default-on" >${LED_SYSFS}/${VPN_LED_GREEN}/trigger
        echo "${LED_ON}" >${LED_SYSFS}/${VPN_LED_GREEN}/brightness
        ;;
      fast)
#        echo "VPN  LED fast blinking for Tx/Rx Operation..."
        echo "none" >${LED_SYSFS}/${VPN_LED_AMBER}/trigger
        echo "timer" >${LED_SYSFS}/${VPN_LED_GREEN}/trigger
        echo "${DELAY_FAST}" >${LED_SYSFS}/${VPN_LED_GREEN}/delay_on
        echo "${DELAY_FAST}" >${LED_SYSFS}/${VPN_LED_GREEN}/delay_off
        ;;
      off)
#        echo "VPN LED OFF"
        echo "none" >${LED_SYSFS}/${VPN_LED_AMBER}/trigger
        echo "none" >${LED_SYSFS}/${VPN_LED_GREEN}/trigger
        ;;
      *)
        echo "Incorrect Option $STATE for $LED"
        ;;
    esac
#    echo "VPN done."
    ;;

  *)
    echo "Incorrect LED Option $LED"
    ;;
esac
