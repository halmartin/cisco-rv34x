#!/bin/sh

POWER_LED_GREEN=/sys/class/leds/rv160w::power-gled/brightness
DIAG_LED_RED=/sys/class/leds/rv160w::diag-rled/brightness
WLAN2_4G_LED_GREEN=/sys/class/leds/rv160w::wlan2_4-gled/brightness
WLAN5G_LED_GREEN=/sys/class/leds/rv160w::wlan5_0-gled/brightness
USB_LED_AMBER=/sys/class/leds/rv160w::usb2_2-aled/brightness
USB_LED_GREEN=/sys/class/leds/rv160w::usb2_2-gled/brightness
VPN_LED_AMBER=/sys/class/leds/rv160w::vpn-aled/brightness
VPN_LED_GREEN=/sys/class/leds/rv160w::vpn-gled/brightness
DMZ_LED_GREEN=/sys/class/leds/rv160w::dmz-gled/brightness

DELAY_SLOW=1
DELAY_FAST=0.333

LED_ON=1
LED_OFF=0

LED="$1"
STATE="$2"

case $LED in

  wlan5g)
    # WLAN 5G ON/OFF
    case $STATE in
      on)
        echo "WLAN 5G LED ON"
        echo ${LED_ON} > ${WLAN5G_LED_GREEN}
        ;;
      off)
        echo "WLAN 5G LED OFF"
        echo ${LED_OFF} > ${WLAN5G_LED_GREEN}
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
        while true; do
          echo ${LED_ON} > ${DIAG_LED_RED}
          sleep $DELAY_SLOW
          echo ${LED_OFF} > ${DIAG_LED_RED}
          sleep $DELAY_SLOW
        done
        ;;
      fast)
        echo "Diag LED fast blinking for Firmware Operation..."
        while true; do
          echo ${LED_ON} > ${DIAG_LED_RED}
          sleep $DELAY_FAST
          echo ${LED_OFF} > ${DIAG_LED_RED}
          sleep $DELAY_FAST
        done
        ;;
      on)
        echo "Diag LED ON"
        echo ${LED_ON} > ${DIAG_LED_RED}
        ;;
      off)
        echo "Diag LED OFF"
        echo ${LED_OFF} > ${DIAG_LED_RED}
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
        echo ${LED_ON} > ${WLAN2_4G_LED_GREEN}
        ;;
      off)
        echo "WLAN 2.4G LED OFF"
        echo ${LED_OFF} > ${WLAN2_4G_LED_GREEN}
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
        echo ${LED_ON} > ${POWER_LED_GREEN}
        ;;
      off)
        echo "Power LED OFF"
        echo ${LED_OFF} > ${POWER_LED_GREEN}
        ;;
      slow)
        echo "Power LED slow blinking for Bootup..."
        while true; do
          echo ${LED_ON} > ${POWER_LED_GREEN}
          sleep $DELAY_SLOW
          echo ${LED_OFF} > ${POWER_LED_GREEN}
          sleep $DELAY_SLOW
        done
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
        echo ${LED_OFF} > ${USB_LED_GREEN}
        echo ${LED_ON} > ${USB_LED_AMBER}
        ;;
      green)
        echo ${LED_OFF} > ${USB_LED_AMBER}
        echo ${LED_ON} > ${USB_LED_GREEN}
        ;;
      blink)
	ledam=`cat ${USB_LED_AMBER}`
	if [ "$ledam" = "0" ];then
	    echo ${LED_OFF} > ${USB_LED_GREEN}
	    sleep .5
	    echo ${LED_ON} > ${USB_LED_GREEN}
	    sleep .5
	fi
	;;
      off)
        echo ${LED_OFF} > ${USB_LED_AMBER}
        echo ${LED_OFF} > ${USB_LED_GREEN}
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
        echo ${LED_OFF} > ${USB_LED_GREEN}
        echo ${LED_ON} > ${USB_LED_AMBER}
        ;;
      green)
        echo ${LED_OFF} > ${USB_LED_AMBER}
        echo ${LED_ON} > ${USB_LED_GREEN}
        ;;
      blink)
	ledam=`cat ${USB_LED_AMBER}`
	if [ "$ledam" = "0" ];then
	    echo ${LED_OFF} > ${USB_LED_GREEN}
	    sleep .5
	    echo ${LED_ON} > ${USB_LED_GREEN}
	    sleep .5
	fi
	;;
      off)
        echo ${LED_OFF} > ${USB_LED_AMBER}
        echo ${LED_OFF} > ${USB_LED_GREEN}
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
        echo ${LED_ON} > ${DMZ_LED_GREEN}
        ;;
      off)
        echo "DMZ LED OFF"
        echo ${LED_OFF} > ${DMZ_LED_GREEN}
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
        echo ${LED_OFF} > ${VPN_LED_GREEN}
        echo ${LED_ON} > ${VPN_LED_AMBER}
        ;;
      green)
#        echo "VPN LED GREEN"
        echo ${LED_OFF} > ${VPN_LED_AMBER}
        echo ${LED_ON} > ${VPN_LED_GREEN}
        ;;
      fast)
#        echo "VPN  LED fast blinking for Tx/Rx Operation..."
        while true; do
          echo ${LED_OFF} > ${VPN_LED_AMBER}
          echo ${LED_ON} > ${VPN_LED_GREEN}
          sleep $DELAY_FAST
          echo ${LED_OFF} > ${VPN_LED_AMBER}
          echo ${LED_OFF} > ${VPN_LED_GREEN}
          sleep $DELAY_FAST
        done
        ;;
      off)
#        echo "VPN LED OFF"
        echo ${LED_OFF} > ${VPN_LED_AMBER}
        echo ${LED_OFF} > ${VPN_LED_GREEN}
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
