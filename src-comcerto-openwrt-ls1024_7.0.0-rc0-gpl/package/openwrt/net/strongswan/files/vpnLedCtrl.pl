#!/usr/bin/perl
###########################################################################
# *  * Copyright 2015, Freescale Semiconductor, Inc. All rights reserved. #
###########################################################################

###########################################################################
#file: vpnLedCtrl.pl                                                      #
#This script is executed by cmm/init script to update sa/connection add or#
#  del events to vpnLed module                                            #
###########################################################################

use IO::Socket::INET;

$num_args = $#ARGV + 1;
if ($num_args != 1) {
    print "Usage: vpnLedCtrl <sa_add|sa_del|conn_add|conn_del|config_add|config_del|ipsec_off>\n";
    exit;
}

# flush after every write
$| = 1;

my ($socket,$data);

#  We call IO::Socket::INET->new() to create the UDP Socket
# and bind with the PeerAddr.
$socket = new IO::Socket::INET (
        PeerAddr   => '127.0.0.1:5242',
        Proto        => 'udp'
) or die "ERROR in Socket Creation : $!\n";

#send operation
$data = $ARGV[0];

if($data =~ /sa_add|sa_del|conn_add|conn_del|config_add|config_del|ipsec_off/)
{
	$socket->send($data);
#	printf("Send the dada \"$data\" successfully");
}
else
{
#	printf("Unknown command:$data\n");
	`logger -t "VPN-LED" "Unknown command:$data"`
}

$socket->close();

