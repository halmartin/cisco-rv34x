#!/bin/sh
DEBUG=false
# Number of days to warn about soon-to-expire certs
warning_days=100

certs_to_check='members.dyndns.org:443
dynupdate.no-ip.com:443
nic.changeip.com:443'

certs='ChangeIP.pem
DynDNS.pem
No-ip.pem'

for CERT in $certs_to_check
do
	$DEBUG && echo "Checking cert: [$CERT]"

	for CERTFILE in $certs
	do
		provider=$(echo $CERT | awk -F '.' '{print $2}')
		if [ "$(echo $CERTFILE | grep -i $provider)" != "" ];then
			if [ -f "/etc/ssl/certs/$CERTFILE" ];then
				end_date=$(openssl x509 -noout -enddate -in /etc/ssl/certs/$CERTFILE | awk -F ' ' '{ print $4"-01-" $2" "$3}')
			else
				openssl s_client -connect $CERT < /dev/null > /tmp/temporary.out
				openssl x509 -outform PEM -in /tmp/temporary.out -out /etc/ssl/certs/$CERTFILE
				end_date=$(openssl x509 -noout -enddate -in /etc/ssl/certs/$CERTFILE | awk -F ' ' '{ print $4"-01-" $2" "$3}')
			fi
			break
		fi
	done

	end_epoch=$(date +%s -d "$end_date")
	epoch_now=$(date +%s)
	
	if [ "$end_epoch" = "" ];then
		end_epoch=$(($epoch_now + 1))
	fi

	seconds_to_expire=$(($end_epoch - $epoch_now))
	days_to_expire=$(($seconds_to_expire / 86400))

	$DEBUG && echo "Days to expiry: ($days_to_expire)"

	warning_seconds=$((86400 * $warning_days))

	if [ "$seconds_to_expire" -lt "$warning_seconds" ]; then
		$DEBUG && echo "Cert [$CERT] is soon to expire ($seconds_to_expire seconds)"
		#logger -t ddns -p local0.info "cert [$CERT] is soon to expire in $days_to_expire days "
		openssl s_client -connect $CERT < /dev/null > /tmp/temporary.out
		openssl x509 -outform PEM -in /tmp/temporary.out -out /etc/ssl/certs/$CERTFILE
	fi
done
