#!/bin/sh
. /lib/functions.sh
. /lib/functions/network.sh

OLD_CA_DIR="/mnt/webrootdb/certificates/cacerts"
NEW_CA_DIR="/mnt/webrootdb/certificates/certs"
OLD_CERT_DIR="/mnt/webrootdb/certificates/certs"
NEW_CERT_DIR="/mnt/webrootdb/certificates/certs"
TEMP_UCI_FILE="updated_certificate"


upgrade_ImportedCertsToLatest()
{
	local cfg="$1"
	local cert_name duration type signed_by signed_by_str country_name state locality_name organization_name organization_unit_name common_name email_address sub_alt_name_type sub_alt_name key_encryption subject

	config_get cert_name "$cfg" cert_name
	config_get type "$cfg" type
	config_get duration "$cfg" duration
	config_get subject "$cfg" subject
	config_get signed_by "$cfg" signed_by
	config_get signed_by_str "$cfg" signed_by_str
	config_get country_name "$cfg" country_name
	config_get state "$cfg" state
	config_get locality_name "$cfg" locality_name
	config_get organization_name "$cfg" organization_name
	config_get organization_unit_name "$cfg" organization_unit_name
	config_get common_name "$cfg" common_name
	config_get email_address "$cfg" email_address
	config_get sub_alt_name_type "$cfg" sub_alt_name_type
	config_get sub_alt_name "$cfg" sub_alt_name
	config_get key_encryption "$cfg" key_encryption
	config_get useby "$cfg" useby "-"

	if [ "$type" = "0" ]
	then
	#{
		#Imported NON CA certificate
		#ISSUER=`openssl x509 -in $TMP_CERTS_IN/$cert_name.$PEM_EXT -noout -issuer`
		#ISSUER=`openssl x509 -in /etc/ssl/certs/"$cert_name".pem -noout -issuer`
		ISSUER=`openssl x509 -in "$OLD_CERT_DIR"/"$cert_name".pem -noout -issuer`
		ISSUER="${ISSUER#*/}"
		ISSUER="/$ISSUER"

		issuer_country=`echo $ISSUER | awk -F 'C=' '{print $2}' | cut -d / -f 1`
		issuer_state_name=`echo $ISSUER | awk -F 'ST=' '{print $2}' | cut -d / -f 1`
		issuer_localityname=`echo $ISSUER | awk -F 'L=' '{print $2}' | cut -d / -f 1`
		issuer_organization_name=`echo $ISSUER | awk -F 'O=' '{print $2}' | cut -d / -f 1`
		issuer_organization_unit_name=`echo $ISSUER | awk -F 'OU=' '{print $2}' | cut -d / -f 1`
		issuer_common_name=`echo $ISSUER | awk -F 'CN=' '{print $2}' | cut -d / -f 1`
		issuer_emailAddress=`echo $ISSUER | awk -F 'emailAddress=' '{print $2}' | cut -d / -f 1`

		[ -n "$issuer_country" ] || issuer_country="-"
		[ -n "$issuer_state_name" ] || issuer_state_name="-"
		[ -n "$issuer_localityname" ] || issuer_localityname="-"
		[ -n "$issuer_organization_name" ] || issuer_organization_name="-"
		[ -n "$issuer_organization_unit_name" ] || issuer_organization_unit_name="-"
		[ -n "$issuer_common_name" ] || issuer_common_name="-"
		[ -n "$issuer_emailAddress" ] || issuer_emailAddress="-"

		uci set "$TEMP_UCI_FILE".$cert_name=imported_certificate
		uci set "$TEMP_UCI_FILE"."$cert_name".cert_name="$cert_name"
		uci set "$TEMP_UCI_FILE"."$cert_name".is_imported=1
		uci set "$TEMP_UCI_FILE"."$cert_name".duration="$duration"
		uci set "$TEMP_UCI_FILE"."$cert_name".type="$type"
		uci set "$TEMP_UCI_FILE"."$cert_name".CertType="$type"
		uci set "$TEMP_UCI_FILE"."$cert_name".subject="$subject"
		uci set "$TEMP_UCI_FILE"."$cert_name".signed_by="$signed_by"
		uci set "$TEMP_UCI_FILE"."$cert_name".signed_by_str="$signed_by_str"
		uci set "$TEMP_UCI_FILE"."$cert_name".sub_country_name="$country_name"
		uci set "$TEMP_UCI_FILE"."$cert_name".issuer_country_name="$issuer_country"
		uci set "$TEMP_UCI_FILE"."$cert_name".sub_state="$state"
		uci set "$TEMP_UCI_FILE"."$cert_name".issuer_state="$issuer_state_name"
		uci set "$TEMP_UCI_FILE"."$cert_name".sub_locality_name="$locality_name"
		uci set "$TEMP_UCI_FILE"."$cert_name".issuer_locality_name="$issuer_localityname"
		uci set "$TEMP_UCI_FILE"."$cert_name".sub_organization_name="$organization_name"
		uci set "$TEMP_UCI_FILE"."$cert_name".issuer_organization_name="$issuer_organization_name"
		uci set "$TEMP_UCI_FILE"."$cert_name".sub_organization_unit_name="$organization_unit_name"
		uci set "$TEMP_UCI_FILE"."$cert_name".issuer_organization_unit_name="$issuer_organization_unit_name"
		uci set "$TEMP_UCI_FILE"."$cert_name".sub_common_name="$common_name"
		uci set "$TEMP_UCI_FILE"."$cert_name".issuer_common_name="$issuer_common_name"
		uci set "$TEMP_UCI_FILE"."$cert_name".sub_email_address="$email_address"
		uci set "$TEMP_UCI_FILE"."$cert_name".issuer_email_address="$issuer_emailAddress"
		uci set "$TEMP_UCI_FILE"."$cert_name".useby="$useby"
		uci set "$TEMP_UCI_FILE"."$cert_name".sub_alt_name_type="$sub_alt_name_type"
		uci set "$TEMP_UCI_FILE"."$cert_name".sub_alt_name="$sub_alt_name"
		uci set "$TEMP_UCI_FILE"."$cert_name".key_encryption="$key_encryption"
		uci set "$TEMP_UCI_FILE"."$cert_name".allow_export=1
		uci set "$TEMP_UCI_FILE"."$cert_name".source=2

		#No need to copy or move any, since the old and new firmware directories are same for MR0 and 0.5
		#cp #old to #new
	#}
	elif [ "$type" = "1" ]
	then
	#{
		#Imported CA certificate
		#In MR0 infra the CA cert ext was caCert. In MR.5/1 it is _CA
		local edited_cert_name=`echo $cert_name | sed  "s/\(.*\)caCert/\1_CA/g"`

		uci set "$TEMP_UCI_FILE"."$edited_cert_name"=imported_certificate
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".type="$type"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".CertType="$type"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".cert_name="$edited_cert_name"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".sub_hash=`openssl x509 -in "$OLD_CA_DIR"/"$cert_name".pem -noout -subject_hash`
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".duration="$duration"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".signed_by="$signed_by"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".subject="$subject"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".signed_by_str="$signed_by_str"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".sub_country_name="$country_name"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".issuer_country_name="$country_name"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".sub_state="$state"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".issuer_state="$state"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".sub_locality_name="$locality_name"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".issuer_locality_name="$locality_name"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".sub_organization_name="$organization_name"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".issuer_organization_name="$organization_name"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".sub_organization_unit_name="$organization_unit_name"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".issuer_organization_unit_name="$organization_unit_name"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".sub_common_name="$common_name"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".issuer_common_name="$common_name"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".sub_email_address="$email_address"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".issuer_email_address="$email_address"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".sub_alt_name="$sub_alt_name"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".sub_alt_name_type="$sub_alt_name_type"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".is_ca_enabled=1
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".allow_export=1
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".source=2
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".useby="$useby"
		uci set "$TEMP_UCI_FILE"."$edited_cert_name".key_encryption="$key_encryption"

		#As of now we are copying the certificates only.
		cp "$OLD_CA_DIR"/"$cert_name".pem "$NEW_CA_DIR"/"$edited_cert_name".pem
	#}
	fi
}

upgrade_generatedToLatest()
{
#TODO/ doubt about variable names
	local cfg="$1"
	local cert_name type organization_name organization_unit_name common_name key_encryption signed_by signed_by_str subject duration use_by

	config_get cert_name "$cfg" cert_name
	config_get type "$cfg" type
	config_get organization_name "$cfg" organization_name
	config_get organization_unit_name "$cfg" organization_unit_name
	config_get common_name "$cfg" common_name
	config_get key_encryption "$cfg" key_encryption
	config_get signed_by "$cfg" signed_by
	config_get signed_by_str "$cfg" signed_by_str
	config_get subject "$cfg" subject
	config_get duration "$cfg" duration
	config_get useby "$cfg" useby

	uci set "$TEMP_UCI_FILE".$cert_name=generated_certificate
	uci set "$TEMP_UCI_FILE".$cert_name.type="$type"
	uci set "$TEMP_UCI_FILE".$cert_name.cert_name="$cert_name"
	uci set "$TEMP_UCI_FILE".$cert_name.sub_organization_name="$organization_name"
	uci set "$TEMP_UCI_FILE".$cert_name.sub_organization_unit_name="$organization_unit_name"
	uci set "$TEMP_UCI_FILE".$cert_name.sub_common_name="$common_name"
	uci set "$TEMP_UCI_FILE".$cert_name.key_encryption="$key_encryption"
	uci set "$TEMP_UCI_FILE".$cert_name.signed_by="$signed_by"
	uci set "$TEMP_UCI_FILE".$cert_name.signed_by_str="$signed_by_str"
	uci set "$TEMP_UCI_FILE".$cert_name.issuer_common_name="$signed_by_str"
	uci set "$TEMP_UCI_FILE".$cert_name.source=1
	uci set "$TEMP_UCI_FILE".$cert_name.has_private_key=1
	uci set "$TEMP_UCI_FILE".$cert_name.allow_export=1
	uci set "$TEMP_UCI_FILE".$cert_name.subject="$subject"
	uci set "$TEMP_UCI_FILE".$cert_name.duration="$duration"
	uci set "$TEMP_UCI_FILE".$cert_name.useby="$useby"

	#Same path for old and new, hence don't cp.
	#cp /etc/ssl/certs/Default.pem /etc/ssl/certs/Default.pem
}

#cp /tmp/etc/config/certificate /tmp/etc/config/$NEW_TEMP_UCI_FILE
logger -t system -p info "Certificates infra migration Start!"

cur_ver=`uci get certificate.version.current_version`

[ "$cur_ver" = "1" ] && {
	logger -t system -p info "Certificates infra migration already done, hence no action taken."
	exit 0
}

touch /tmp/etc/config/$TEMP_UCI_FILE

config_load certificate

config_foreach upgrade_generatedToLatest generated_certificate
config_foreach upgrade_ImportedCertsToLatest imported_certificate
#version 1 says that it is of MR0.5
uci set "$TEMP_UCI_FILE".version=infra
uci set "$TEMP_UCI_FILE".version.current_version=1

uci commit $TEMP_UCI_FILE

logger -t system -p info "Certificates for new infra prepared!"

cp /tmp/etc/config/$TEMP_UCI_FILE /tmp/etc/config/certificate
cp /tmp/etc/config/$TEMP_UCI_FILE /mnt/webrootdb/config/certificate
rm "$OLD_CA_DIR"/*.pem
sync
logger -t system -p info "Certificates infra migration done!"
exit 0
