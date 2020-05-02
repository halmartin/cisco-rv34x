use IO::Socket::INET;

$| = 1;

my ($socket,$data);

$socket = new IO::Socket::INET (
        PeerAddr   => '127.0.0.1:2602',
        Proto        => 'tcp'
) or die "ERROR in Socket Creation : $!\n";

#send operation
$data = "zebra\n";
my $received;
$socket->send("zebra\n") or die "Error in sending password data";
$socket->recv($received,1024,0);
#print "data: ------\n$received \n ------\n";

$socket->send("enable\n") or die "Error in enabling to configure";
$socket->recv($received,1024,0);
#print "data: ------\n$received \n ------\n";

$socket->send("configure terminal\n") or die "Error in sending configure terminal";
$socket->recv($received,1024,0);
#print "data: ------\n$received \n ------\n";

$socket->send("router rip\n") or die "Error in sending router rip";
$socket->recv($received,1024,0);
#print "data: ------\n$received \n ------\n";

#print "========$ARGV[0]============";
if ($ARGV[1] eq "add") {
$socket->send("distribute-list myprefix out $ARGV[0]\n") or die "Error in sending distribute-list cmd";
} else {
$socket->send("no distribute-list myprefix out $ARGV[0]\n") or die "Error in sending distribute-list cmd";
}
$socket->recv($received,1024,0);
#print "data: ------\n$received \n ------\n";

$socket->send("exit\n") or die "Error in sending exit command";
$socket->recv($received,1024,0);
#print "data: ------\n$received \n ------\n";

$socket->send("exit\n") or die "Error in sending exit command";
$socket->recv($received,1024,0);
#print "data: ------\n$received \n ------\n";

$socket->close();

