#!/bin/sh
#. /etc/boardInfo

TMP_PREINSTALLED_CERTFILE=/tmp/preinstalled_certificate
CONFIG_HEADER="config preinstalled_certificate"

PREINSTALLED_CACERTS_DIR=$1
#CONFIG=/tmp/config

DIRTEMP=`mktemp`
rm $DIRTEMP
#mkdir $DIRTEMP
CONFIG=$DIRTEMP

UCI_CERTFILE=preinstalled_certificate
CERTFILE=$CONFIG/$UCI_CERTFILE
TMP_TMPCERTSTARTTIME=`mktemp`
TMP_TMPCERTENDTIME=`mktemp`
TMP_TMPCERTOUTPUT=`mktemp`

add_duration() {
	DIR=$PREINSTALLED_CACERTS_DIR
	`openssl x509 -in $DIR/$@ -noout -startdate| cut -d= -f2 > $TMP_TMPCERTSTARTTIME`
	`openssl x509 -in $DIR/$@ -noout -enddate| cut -d= -f2 > $TMP_TMPCERTENDTIME`
	`sed -i "s/  */ /g" $TMP_TMPCERTSTARTTIME`
	`sed -i "s/  */ /g" $TMP_TMPCERTENDTIME`
	while read mon day time year                                                                               
	do                                                                                                     
		year=`echo $year | cut -d ' ' -f 1`                                                            
		case "$mon" in                                                                                      
			Jan) mon=01 ;;                                                                     
			Feb) mon=02 ;;                                                                         
                	Mar) mon=03 ;;                                                                         
			Apr) mon=04 ;;                                                                     
			May) mon=05 ;;             
			Jun) mon=06 ;;             
			Jul) mon=07 ;;                                            
			Aug) mon=08 ;;                                                                               
			Sep) mon=09 ;;                                                 
			Oct) mon=10 ;;                                                 
			Nov) mon=11 ;;                                                 
			Dec) mon=12 ;;                                                                         
		esac                                                                                           
		day_len=`echo $day| wc -c`                                                                     
		if [ "$day_len" = 2 ];then                                                                     
			day=0$day                                                                                    
		fi                                                 
		start_time="$year-$mon-$day"                            
		done < $TMP_TMPCERTSTARTTIME
                                                           
	while read mon day time year               
	do                                  
		year=`echo $year | cut -d ' ' -f 1`                                         
		case "$mon" in                                   
			Jan) mon=01 ;;                   
			Feb) mon=02 ;;                                                                          
			Mar) mon=03 ;;             
			Apr) mon=04 ;;                                                    
			May) mon=05 ;;                                              
			Jun) mon=06 ;;
			Jul) mon=07 ;;
			Aug) mon=08 ;;
			Sep) mon=09 ;;
			Oct) mon=10 ;;
			Nov) mon=11 ;;
			Dec) mon=12 ;;
		esac      
		day_len=`echo $day| wc -c`
		if [ "$day_len" = 2 ];then
			day=0$day                                             
		fi                                                                                             
		end_time="$year-$mon-$day"
		done < $TMP_TMPCERTENDTIME
		cert_name=`echo $@ | cut -d '.' -f 1`
		`uci set certificate.$cert_name.duration="From $start_time To $end_time" >/dev/null 2>&1`
		echo  '\t'"option duration 'From $start_time To $end_time'" >> $CERTFILE
		rm -rf $TMP_TMPCERTSTARTTIME
		rm -rf $TMP_TMPCERTENDTIME
}

add_subject_issuer() {
	DIR=$PREINSTALLED_CACERTS_DIR
	SUBJECT=`openssl x509 -in $DIR/$@ -noout -subject`
	SUBJECT="${SUBJECT#*/}"
	SUBJECT="/$SUBJECT"
	`openssl x509 -in $DIR/$@  -noout -text > $TMP_TMPCERTOUTPUT 2>&1`
	COMMA_SUBJECT=`openssl x509 -in $DIR/$@ -noout -subject -nameopt -sep_comma_plus | cut -d " " -f 2-`
	ISSUER=`openssl x509 -in $DIR/$@ -noout -issuer`
	ISSUER="${ISSUER#*/}"
	ISSUER="/$ISSUER"                                      
	country=`echo $SUBJECT | awk -F 'C=' '{print $2}' | cut -d / -f 1`
	state_name=`echo $SUBJECT | awk -F 'ST=' '{print $2}' | cut -d / -f 1`
	localityname=`echo $SUBJECT | awk -F 'L=' '{print $2}' | cut -d / -f 1`
	organization_name=`echo $SUBJECT | awk -F 'O=' '{print $2}' | cut -d / -f 1`
	organization_unit_name=`echo $SUBJECT | awk -F 'OU=' '{print $2}' | cut -d / -f 1`
	common_name=`echo $SUBJECT | awk -F 'CN=' '{print $2}' | cut -d / -f 1`                
	emailAddress=`echo $SUBJECT | awk -F 'emailAddress=' '{print $2}' | cut -d / -f 1`

	issuer_country=`echo $ISSUER | awk -F 'C=' '{print $2}' | cut -d / -f 1`
	issuer_state_name=`echo $ISSUER | awk -F 'ST=' '{print $2}' | cut -d / -f 1`
	issuer_localityname=`echo $ISSUER | awk -F 'L=' '{print $2}' | cut -d / -f 1`
	issuer_organization_name=`echo $ISSUER | awk -F 'O=' '{print $2}' | cut -d / -f 1`
	issuer_organization_unit_name=`echo $ISSUER | awk -F 'OU=' '{print $2}' | cut -d / -f 1`
	issuer_common_name=`echo $ISSUER | awk -F 'CN=' '{print $2}' | cut -d / -f 1`         
	issuer_emailAddress=`echo $ISSUER | awk -F 'emailAddress=' '{print $2}' | cut -d / -f 1`

	cert_name=`echo $@ | cut -d '.' -f 1`
	cert_name=`echo $cert_name | cut -d '-' -f 1`

	if [ -n "$COMMA_SUBJECT" ];then
		#`uci set certificate.$cert_name.subject="$COMMA_SUBJECT"`
		echo  '\t'"option subject '$COMMA_SUBJECT'" >> $CERTFILE		
	else
		#`uci set certificate.$cert_name.subject=-`
		echo  '\t'"option subject '-'" >> $CERTFILE		
	fi
                                                                                  
	if [ "$SUBJECT" = "$ISSUER" ];then
		#`uci set certificate.$cert_name.signed_by_str="Self Signed"`
		#`uci set certificate.$cert_name.signed_by=1`
		echo  '\t'"option signed_by_str 'Self Signed'" >> $CERTFILE
		echo  '\t'"option signed_by '1'" >> $CERTFILE
	else        
		# Get issue's organization name as signed_by
		sign_by=`echo $ISSUER | awk -F 'CN=' '{print $2}' | cut -d / -f 1`
		#`uci set certificate.$cert_name.signed_by=2`
                echo  '\t'"option signed_by '2'" >> $CERTFILE      
		if [ -n "$sign_by" ];then
			#`uci set certificate.$cert_name.signed_by_str="$sign_by"`
                	echo  '\t'"option signed_by_str '$sign_by'" >> $CERTFILE      
		else
                	echo  '\t'"option signed_by_str '-'" >> $CERTFILE      
		fi
	fi                                                                            
                                                                                                                            
	if [ -n "$country" ];then
		#`uci set certificate.$cert_name.sub_country_name="$country"`
		echo  '\t'"option sub_country_name '$country'" >> $CERTFILE	
		#`uci set certificate.$cert_name.issuer_country_name="$issuer_country"`
		echo  '\t'"option issuer_country_name '$issuer_country'" >> $CERTFILE

	else
		#`uci set certificate.$cert_name.sub_country_name="-"`
		echo  '\t'"option sub_country_name '-'" >> $CERTFILE	
		#`uci set certificate.$cert_name.issuer_country_name="-"`
		echo  '\t'"option issuer_country_name '-'" >> $CERTFILE
	fi                  
	if [ -n "$state_name" ];then
		#`uci set certificate.$cert_name.sub_state="$state_name"`
		echo  '\t'"option sub_state '$state_name'" >> $CERTFILE
		#`uci set certificate.$cert_name.issuer_state="$issuer_state_name"`
		echo  '\t'"option issuer_state '$issuer_state_name'" >> $CERTFILE
	else
		#`uci set certificate.$cert_name.sub_state="-"`
		echo  '\t'"option sub_state '-'" >> $CERTFILE
		echo  '\t'"option issuer_state '-'" >> $CERTFILE
		#`uci set certificate.$cert_name.issuer_state="-"`
	fi                      
	if [ -n "$localityname" ];then
		#`uci set certificate.$cert_name.sub_locality_name="$localityname"`
		#`uci set certificate.$cert_name.issuer_locality_name="$sub_localityname"`
		echo  '\t'"option sub_locality_name '$localityname'" >> $CERTFILE
		echo  '\t'"option issuer_locality_name '$sub_localityname'" >> $CERTFILE	
	else
		echo  '\t'"option sub_locality_name '-'" >> $CERTFILE
		echo  '\t'"option issuer_locality_name '-'" >> $CERTFILE	
		#`uci set certificate.$cert_name.sub_locality_name="-"`
		#`uci set certificate.$cert_name.issuer_locality_name="-"`
	fi

	if [ -n "$organization_name" ];then
		#`uci set certificate.$cert_name.sub_organization_name="$organization_name"`
		#`uci set certificate.$cert_name.issuer_organization_name="$issuer_organization_name"`
		echo  '\t'"option sub_organization_name '$organization_name'" >> $CERTFILE
		echo  '\t'"option issuer_organization_name '$issuer_organization_name'" >> $CERTFILE
	else
		#`uci set certificate.$cert_name.sub_organization_name="-"`
		#`uci set certificate.$cert_name.issuer_organization_name="-"`
		echo  '\t'"option sub_organization_name '-'" >> $CERTFILE
		echo  '\t'"option issuer_organization_name '-'" >> $CERTFILE
	 fi
	if [ -n "$organization_unit_name" ];then
		#`uci set certificate.$cert_name.sub_organization_unit_name="$organization_unit_name"`            
		#`uci set certificate.$cert_name.issuer_organization_unit_name="$organization_unit_name"`
		echo  '\t'"option sub_organization_unit_name '$organization_unit_name'" >> $CERTFILE
		echo  '\t'"option issuer_organization_unit_name '$issuer_organization_unit_name'" >> $CERTFILE
	else                                      
		#`uci set certificate.$cert_name.sub_organization_unit_name="-"`                                  
		#`uci set certificate.$cert_name.issuer_organization_unit_name="-"`
		echo  '\t'"option sub_organization_unit_name '-'" >> $CERTFILE
		echo  '\t'"option issuer_organization_unit_name '-'" >> $CERTFILE
	fi                                                                               
	if [ -n "$common_name" ];then                                     
		#`uci set certificate.$cert_name.sub_common_name="$common_name"`                           
		#`uci set certificate.$cert_name.issuer_common_name="$issuer_common_name"`
		 echo  '\t'"option sub_common_name '$common_name'" >> $CERTFILE
		 echo  '\t'"option issuer_common_name '$issuer_common_name'" >> $CERTFILE
	else                                                
		#`uci set certificate.$cert_name.sub_common_name="-"`                                             
		#`uci set certificate.$cert_name.issuer_common_name="-"`

		 echo  '\t'"option sub_common_name '-'" >> $CERTFILE
		 echo  '\t'"option issuer_common_name '-'" >> $CERTFILE
	fi                                                                              
	if [ -n "$emailAddress" ];then
		#`uci set certificate.$cert_name.sub_email_address="$emailAddress"`                                  
		#`uci set certificate.$cert_name.issuer_email_address="$emailAddress"` 
		echo  '\t'"option sub_email_address '$emailAddress'" >> $CERTFILE
		echo  '\t'"option issuer_email_address '$issuer_emailAddress'" >> $CERTFILE
	else                               
		#`uci set certificate.$cert_name.sub_email_address="-"`                                           
		#`uci set certificate.$cert_name.issuer_email_address="-"`
		echo  '\t'"option sub_email_address '-'" >> $CERTFILE
		echo  '\t'"option issuer_email_address '-'" >> $CERTFILE
	fi                                                                                                          
                                                                                                                             
	# add used by field                                                                
	#`uci set certificate.$cert_name.useby="-"`
	echo  '\t'"option useby '-'" >> $CERTFILE

	# Get subject Alt name and type                                                                             
	subjectAltnum=`cat $TMP_TMPCERTOUTPUT | grep -n "Subject Alternative Name"`
	if [ -n "$subjectAltnum" ];then                                                  
		subjectAltnum=`echo $subjectAltnum | cut -d : -f1`
		req_line=`expr $subjectAltnum + 1`
		sed_ext=p
		subNameType=`sed -n $req_line$sed_ext $TMP_TMPCERTOUTPUT | cut -d : -f 1 | cut -d " " -f 2`
		subNameValue=`sed -n $req_line$sed_ext $TMP_TMPCERTOUTPUT | cut -d : -f 2`
		if [ "$subNameType" = "IP" ];then                              
			#`uci set certificate.$cert_name.sub_alt_name_type=$SUB_ALT_IP`
			echo  '\t'"option sub_alt_name_type '$SUB_ALT_IP'" >> $CERTFILE
		elif [ "$subNameType" = "DNS" ];then                              
			#`uci set certificate.$cert_name.sub_alt_name_type=$SUB_ALT_DNS`
			echo  '\t'"option sub_alt_name_type '$SUB_ALT_DNS'" >> $CERTFILE
		elif [ "$subNameType" = "email" ];then                            
			#`uci set certificate.$cert_name.sub_alt_name_type=$SUB_ALT_EMAIL`
			echo  '\t'"option sub_alt_name_type '$SUB_ALT_EMAIL'" >> $CERTFILE
		fi                                       
		#`uci set certificate.$cert_name.sub_alt_name="$subNameValue"`
		echo  '\t'"option sub_alt_name '$subNameValue'" >> $CERTFILE
	else                                                   
		#`uci set certificate.$cert_name.sub_alt_name="-"`                                              
		#`uci set certificate.$cert_name.sub_alt_name_type="-"`
		echo  '\t'"option sub_alt_name '-'" >> $CERTFILE
		echo  '\t'"option sub_alt_name_type '-'" >> $CERTFILE
	fi
                                                                                                                            
	key_enc_length=`cat $TMP_TMPCERTOUTPUT | grep Public-Key: | cut -d : -f 2 | cut -d "(" -f 2 | cut -d " " -f 1`
                                                                                                                            
	if [ -n "$key_enc_length" ];then                                    
		#`uci set certificate.$cert_name.key_encryption="$key_enc_length"`
		echo  '\t'"option key_encryption '$key_enc_length'" >> $CERTFILE
	
	else                                                                                                    
		#`uci set certificate.$cert_name.key_encryption="-"`                                                 
		echo  '\t'"option key_encryption '-'" >> $CERTFILE
	fi                                                                                    
	rm -rf $TMP_TMPCERTOUTPUT

}

mkdir -p $CONFIG
#rm $CERTFILE
touch $CONFIG/$UCI_CERTFILE

for cname in `ls "$PREINSTALLED_CACERTS_DIR"/*.crt`
do
	#name=`echo $cname | cut -d '/' -f 5`
	name=`echo $cname | sed 's/.*\///'`
	new_name=`echo -n $name | perl -p -e '$_ =~ s/[^a-zA-Z0-9_.]/_/g;$_ =~ s/__*/_/g;'` 
	#echo $new_name

	if [ "$name" != "$new_name" ]; then
		mv $cname $PREINSTALLED_CACERTS_DIR/$new_name
	fi

	cert_name=`echo $new_name | cut -d '.' -f 1`
	#echo $cert_name
	#Create a UCI entry for each preinstalled certificate

	echo "$CONFIG_HEADER '$cert_name'" >> $CERTFILE 	
	echo  '\t'"option cert_name '$cert_name'" >> $CERTFILE
	echo  '\t'"option allow_export '1'" >> $CERTFILE
	echo  '\t'"option source '0'" >> $CERTFILE
	echo  '\t'"option type '1'" >> $CERTFILE
	echo  '\t'"option is_ca_enabled '1'" >> $CERTFILE

	add_duration $new_name	
	add_subject_issuer  $new_name
	echo "" >> $CERTFILE
done
cp $CONFIG/$UCI_CERTFILE $1

rm -rf $CONFIG
#Update the certificate entries
#/usr/bin/certscript

