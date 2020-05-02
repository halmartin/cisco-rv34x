#!/usr/bin/perl -w
###########################################################################
# *  * Copyright 2015, Freescale Semiconductor, Inc. All rights reserved. #
###########################################################################

###########################################################################
#file: vpnLed.pl                                                          #
#This script is executed by init script to to handle sa/conn events and   #
#  control vpnLeds                                                        #
###########################################################################

$|++;
use strict;
use IO::Socket;

my $log=0;

my $led_script="rv340_led.sh";
#my $led_script="rv16x_26x_led.sh";

#my $ledState="AMBER";
my $ledState="OFF";

my $server_port=5242;

listern_udp_server();

sub sa_add_action() {
	if($ledState =~ /AMBER/ || $ledState =~ /OFF/) {
		$ledState="GREEN_SOLID";
	}
}

sub sa_del_action()
{
#find num of active SAs;
	sleep(1);
	my $num_sa=`setkey -D|grep -E "^[0-9]"|wc -l`;

	if ($log == 1){
		system("logger","-t VPN-LED","Number of IPSec SAs found is $num_sa");
	}

	if ($num_sa == 0 || $num_sa == 1) {
		$ledState="AMBER";
	}
	my $active_config=`uci show strongswan |grep "enable=1"|wc -l`;

	if($active_config == 0) {
		$ledState="OFF";
	}
}

sub conn_add_action()
{
	sleep(1);
	my $num_conns=`cmm -c query connections 2>/dev/null | grep "^IPSEC"|wc -l`;

	if ($log == 1){
		system("logger","-t VPN-LED","Number of IPSec connection found is $num_conns");
	}

	if ($num_conns > 0) {
		$ledState="GREEN_FLASH";
	}
}

sub conn_del_action()
{
#find num of active connections;
	sleep(1);
	my $num_conns=`cmm -c query connections 2>/dev/null | grep "^IPSEC"|wc -l`;

	if ($log == 1){
		system("logger","-t VPN-LED","Number of IPSec connection found is $num_conns");
	}

	if ($num_conns == 0) {
		$ledState="GREEN_SOLID";
	}

	my $num_sa=`setkey -D|grep -E "^[0-9]"|wc -l`;
	if ($num_sa == 0 || $num_sa == 1) {
		$ledState="AMBER";
	}
	my $active_config=`uci show strongswan |grep "enable=1"|wc -l`;

	if($active_config == 0) {
		$ledState="OFF";
	}
}

sub config_add_action()
{
	my $active_config=`uci show strongswan |grep "enable=1"|wc -l`;
        my $num_conns=`cmm -c query connections 2>/dev/null | grep "^IPSEC"|wc -l`;
	my $num_sa=`setkey -D|grep -E "^[0-9]"|wc -l`;

	if($active_config == 0) {
		$ledState="OFF";
	} 
        elsif ($num_conns > 0) {
                $ledState="GREEN_FLASH";
        }
	elsif ($num_sa == 0 || $num_sa == 1) {
		$ledState="AMBER";
	}
	else {
		$ledState="GREEN_SOLID"
	}
}


sub config_del_action()
{
	my $active_config=`uci show strongswan |grep "enable=1"|wc -l`;

	if($active_config == 0) {
		$ledState="OFF";
	}
}

sub config_ipsec_off_action()
{
	$ledState="OFF";
}

sub set_led_state()
{
	if ($ledState =~ /AMBER/){
		system("kill_led.sh vpn");
		system($led_script." vpn off");
		system($led_script." vpn amber");
		if ($log == 1){
			system("logger","-t VPN-LED","Moving VPN LED to AMBER state");
		}
	}elsif ($ledState =~ /GREEN_SOLID/) {
		system("kill_led.sh vpn");
		system($led_script." vpn off");
		system($led_script." vpn green");
		if ($log == 1){
			system("logger","-t VPN-LED","Moving VPN LED to Solid Green state");
		}
	}elsif ($ledState =~ /GREEN_FLASH/) {
		system("kill_led.sh vpn");
		system($led_script." vpn off");
		system($led_script." vpn fast &");
		if ($log == 1){
			system("logger","-t VPN-LED","Moving VPN LED to Fast Blink state");
		}
	}elsif ($ledState =~ /OFF/) {
		system("kill_led.sh vpn");
                system($led_script." vpn off");
		if ($log == 1){
			system("logger","-t VPN-LED","Setting VPN LED to OFF");
		}
	}
}

sub listern_udp_server()
{
	my $server = IO::Socket::INET->new(LocalAddr=>'127.0.0.1',LocalPort=>$server_port,Proto=>"udp")
		or die "Can't create UDP server: $@";
	my ($datagram,$flags);

	while ($server->recv($datagram,42,$flags)) {
		my $ipaddr = $server->peerhost;
#		print "Oooh -- udp from $ipaddr, flags ",$flags || "none",": $datagram\n";

		if($datagram =~ /sa_add/) {
			if ($log == 1){
				system("logger","-t VPN-LED","Processing SA ADD Action");
			}
			sa_add_action();
		} elsif ($datagram =~ /sa_del/) {
			if ($log == 1){
				system("logger","-t VPN-LED","Processing SA DEL Action");
			}
			sa_del_action();
		} elsif ($datagram =~ /conn_add/) {
			if ($log == 1){
				system("logger","-t VPN-LED","Processing Connection ADD Action");
			}
			conn_add_action();
		} elsif ($datagram =~ /conn_del/) {
			if ($log == 1){
				system("logger","-t VPN-LED","Processing Connection DEL Action");
			}
			conn_del_action();
		} elsif ($datagram =~ /config_add/) {
			if ($log == 1){
				system("logger","-t VPN-LED","Processing Configuration ADD Action");
			}
			config_add_action();
		} elsif ($datagram =~ /config_del/) {
			if ($log == 1){
				system("logger","-t VPN-LED","Processing Configuration DEL Action");
			}
			config_del_action();
		} elsif ($datagram =~ /ipsec_off/) {
			if ($log == 1){
				system("logger","-t VPN-LED","Global IPSec disabled, setting VPN LED to OFF");
			}
			config_ipsec_off_action();
			set_led_state();
			$server->close();
			exit(0);
		}
		set_led_state();
	}
}
