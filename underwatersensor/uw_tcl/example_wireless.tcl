#Define options
set val(chan)	Channel/WirelessChannel	;#channel type
set val(prop)	Propagation/TwoRayGround ;#radio model
set val(netif)	Phy/WirelessPhy	;#network interface type
set val(mac)	Mac/802_11	;#MAC type
set val(ifq)	Queue/DropTail/PriQueue	;#interface queue type
set val(ll)	LL	;#Link layer type
set val(ant)	Antenna/OmniAntenna	;#antenna model
set val(ifqlen)	50	;#max packet in ifq
set val(nn)	2	;#number of mobilenodes
set val(rp)	AODV	;#routing protocol
set val(x)	500
set val(y)	500	;#x y topography

#Main Program

#Initialize Global Variables

set ns [new Simulator]

set tracefd [open example_wireless.tr w]
$ns trace-all $tracefd
set namtracefd [open example_wireless.nam w]
$ns namtrace-all-wireless $namtracefd $val(x) $val(y)

#set up topography object
set topo [new Topography]
$topo load_flatgrid $val(x) $val(y)

#Create God
create-god $val(nn)

#Create the specified number of mobilenodes and "attach" them
#to the channel
#create and config nodes

$ns node-config -adhocRouting $val(rp)\
	-llType $val(ll)\
	-macType $val(mac)\
	-ifqType $val(ifq)\
	-ifqLen $val(ifqlen)\
	-antType $val(ant)\
	-propType $val(prop)\
	-phyType $val(netif)\
	-channelType $val(chan)\
	-topoInstance $topo\
	-agentTrace ON\
	-routerTrace ON\
	-macTrace OFF\
	-movementsTrace OFF\

for {set i 0} {$i < $val(nn)} {incr i} {
	set node_($i) [$ns node]
	$node_($i) random-motion 0	;#disable random motion
}

#Provide initial(X Y for now Z=0)
$node_(0) set X_ 5.0
$node_(0) set Y_ 2.0
$node_(0) set Z_ 0.0

$node_(1) set X_ 390.0
$node_(1) set Y_ 385.0
$node_(1) set Z_ 0.0

#Now produce some simple node movements
#Node_(1) starts to move towards node_(0)

$ns at 5.0 "$node_(1) setdest 25.0 20.0 15.0"
$ns at 1.0 "$node_(0) setdest 20.0 18.0 1.0"

#Node(1) then stars to move away from node_(0)
$ns at 10.0 "$node_(1) setdest 490.0 480.0 15.0"

#tcp connections between node(0) and node(1)
set tcp [new Agent/TCP]
$tcp set class_ 2
set sink [new Agent/TCPSink]
$ns attach-agent $node_(0) $tcp
$ns attach-agent $node_(1) $sink
$ns connect $tcp $sink
set ftp [new Application/FTP]
$ftp attach-agent $tcp
$ns at 1.0 "$ftp start"

#end the simulation
for {set i 0} {$i < $val(nn)} {incr i} {
	$ns at 15.0 "$node_($i) reset";
}

$ns at 15.0 "stop"

proc stop {} {
	global ns tracefd namtracefd
	$ns flush-trace
	close $tracefd
	close $namtracefd
	exec nam example_wireless.nam &
	exit 0
}

$ns run


