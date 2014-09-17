set opt(chan) Channel/UnderwaterChannel 
set opt(prop) Propagation/UnderwaterPropagation
set opt(netif) Phy/UnderwaterPhy 
set opt(mac) Mac/UnderwaterMac/BroadcastMac 
set opt(ifq) Queue/DropTail/PriQueue 
set opt(ll) LL 
set opt(energy) EnergyModel
set opt(txpower) 2.0
#set opt(rxpower) 0.75
set opt(rxpower) 0.1
set opt(initialenergy) 10000
#set opt(idlepower) 0.008
set opt(idlepower) 0.01
set opt(ant) Antenna/OmniAntenna 
set opt(filters) GradientFilter ;# options can be one or more of 
;# TPP/OPP/Gear/Rmst/SourceRoute/Log/TagFilter
set opt(minspeed) 0 ;#minimum speed of node
set opt(maxspeed) 3 ;#maximum speed of node
set opt(speed) 0.5 ;#speed of node
set opt(position_update_interval) 0.3 ;# the length of period to update node's position
set opt(packet_size) 50 ;#50 bytes
set opt(routing_control_packet_size) 20 ;#bytes 

set opt(ifqlen) 50 ;# max queue length in if
set opt(nn) 200 ;# number of nodes 
set opt(x) 500 ;# X dimension of the topography
set opt(y) 500 ;# Y dimension of the topography
set opt(z) 500
set opt(seed) 12
set opt(stop) 500 ;# simulation time
set opt(prestop) 90 ;# time to prepare to stop
set opt(tr) "a.tr" ;# trace file
set opt(datafile) "a.data"
set opt(nam) "a.nam" ;# nam file
set opt(adhocRouting) DBR
set opt(interval) 10.0
set opt(range) 50 ;#range of each node in meters

if { $argc > 0 } {
set opt(seed) [lindex $argv 0]
set opt(nn) [lindex $argv 1]
set opt(datafile) [lindex $argv 2]
}

puts "the file name is $opt(datafile)"
puts "the sending interval is $opt(interval)"

# ==================================================================

LL set mindelay_ 50us
LL set delay_ 25us
LL set bandwidth_ 0 ;# not used

Queue/DropTail/PriQueue set Prefer_Routing_Protocols 1

# unity gain, omni-directional antennas
# set up the antennas to be centered in the node and 1.5 meters above it
Antenna/OmniAntenna set X_ 0
Antenna/OmniAntenna set Y_ 0
#Antenna/OmniAntenna set Z_ 1.5
Antenna/OmniAntenna set Z_ 0.05
Antenna/OmniAntenna set Gt_ 1.0
Antenna/OmniAntenna set Gr_ 1.0

Mac/UnderwaterMac set bit_rate_ 1.0e4 ;#10kbps
Mac/UnderwaterMac set encoding_efficiency_ 1
Mac/UnderwaterMac/BroadcastMac set packetheader_size_ 0 ;# #of bytes

# Initialize the SharedMedia interface with parameters to make
# it work like the 914MHz Lucent WaveLAN DSSS radio interface
Phy/UnderwaterPhy set CPThresh_ 10 ;#10.0
Phy/UnderwaterPhy set CSThresh_ 0 ;#1.559e-11
Phy/UnderwaterPhy set RXThresh_ 0 ;#3.652e-10
#Phy/WirelessPhy set Rb_ 2*1e6
Phy/UnderwaterPhy set Pt_ 0.2818
Phy/UnderwaterPhy set freq_ 25 ;# 25khz 
Phy/UnderwaterPhy set K_ 2.0 ;# spherical spreading

# =================================================================
# Main Program
# =================================================================

#
# Initialize Global Variables
# 
#set sink_ 1
set ns_ [new Simulator]
set topo [new Topography]

$topo load_cubicgrid $opt(x) $opt(y) $opt(z)
$ns_ use-newtrace
set tracefd [open $opt(tr) w]
$ns_ trace-all $tracefd

set nf [open $opt(nam) w]
$ns_ namtrace-all-wireless $nf $opt(x) $opt(y)


set data [open $opt(datafile) a]


set total_number [expr $opt(nn)-1]
set god_ [create-god $opt(nn)]


$ns_ at 0.0 "$god_ set_filename $opt(datafile)"


set chan_1_ [new $opt(chan)]


global defaultRNG
$defaultRNG seed $opt(seed)



$ns_ node-config -adhocRouting $opt(adhocRouting) \
-llType $opt(ll) \
-macType $opt(mac) \
-ifqType $opt(ifq) \
-ifqLen $opt(ifqlen) \
-antType $opt(ant) \
-propType $opt(prop) \
-phyType $opt(netif) \
#-channelType $opt(chan) \
-agentTrace OFF \
-routerTrace ON \
-macTrace OFF\
-topoInstance $topo\
-energyModel $opt(energy)\
-txPower $opt(txpower)\
-rxPower $opt(rxpower)\
-initialEnergy $opt(initialenergy)\
-idlePower $opt(idlepower)\
-channel $chan_1_


#Set the Sink node 1
#!!!segment fault not before this line
set node_(0) [ $ns_ node 0]
$node_(0) set sinkStatus_ 1

$god_ new_node $node_(0)
$node_(0) set X_ [expr $opt(x)/2]
$node_(0) set Y_ [expr $opt(y)/2]
$node_(0) set Z_ $opt(z)
$node_(0) set passive 1

set rt [$node_(0) set ragent_]
$rt set control_packet_size $opt(routing_control_packet_size)

set a_(0) [new Agent/UWSink]
$ns_ attach-agent $node_(0) $a_(0)
$a_(0) attach-routing
$a_(0) cmd set-range $opt(range) 
$a_(0) cmd set-target-x 50
$a_(0) cmd set-target-y 50
$a_(0) cmd set-target-z 100
$a_(0) cmd set-filename $opt(datafile)
$a_(0) cmd set-packetsize $opt(packet_size) ;# # of bytes


set xrand_ [new RandomVariable/Uniform]
$xrand_ set min_ 0
$xrand_ set max_ $opt(x)
set yrand_ [new RandomVariable/Uniform]
$yrand_ set min_ 0
$yrand_ set max_ $opt(y)
set zrand_ [new RandomVariable/Uniform]
$zrand_ set min_ 0
$zrand_ set max_ $opt(z)


for {set i 1} { $i < $opt(nn)-1 } {incr i} {

	set node_($i) [ $ns_ node $i]
	$node_($i) set sinkStatus_ 1
	$node_($i) random-motion 1

	$node_($i) set max_speed $opt(maxspeed)
	$node_($i) set min_speed $opt(minspeed)
	$node_($i) set position_update_interval_ $opt(position_update_interval)
	
	$god_ new_node $node_($i)
	$node_($i) set X_ [$xrand_ value]
	$node_($i) set Y_ [$yrand_ value]
	$node_($i) set Z_ [$zrand_ value]
	$node_($i) set passive 1
	$node_($i) move

	set rt [$node_($i) set ragent_]
	$rt set control_packet_size $opt(routing_control_packet_size)
	
	set a_($i) [new Agent/UWSink]
	$ns_ attach-agent $node_($i) $a_($i)
	$a_($i) attach-routing
	$a_($i) cmd set-range $opt(range) 
	$a_($i) cmd set-target-x 50
	$a_($i) cmd set-target-y 50
	$a_($i) cmd set-target-z 100
	$a_($i) cmd set-filename $opt(datafile)
	$a_($i) cmd set-packetsize $opt(packet_size) ;# # of bytes
	#$node_(1) move
}



#set node 1


set num [expr $opt(nn)-1]
#Set the source node
set node_($num) [$ns_ node $num]
$god_ new_node $node_($num)
$node_($num) set sinkStatus_ 1

$node_($num) set max_speed $opt(maxspeed)
$node_($num) set min_speed $opt(minspeed)
$node_($num) set position_update_interval_ $opt(position_update_interval)

$node_($num) set X_ [expr $opt(x)/2]
$node_($num) set Y_ [expr $opt(y)/2]
$node_($num) set Z_ 0
$node_($num) set-cx [expr $opt(x)/2]
$node_($num) set-cy [expr $opt(y)/2]
$node_($num) set-cz 0
set rt [$node_($num) set ragent_]
#$rt set control_packet_size $opt(routing_control_packet_size)


set a_($num) [new Agent/UWSink]
$ns_ attach-agent $node_($num) $a_($num)
$a_($num) attach-routing
$a_($num) cmd set-range $opt(range)
$a_($num) cmd set-target-x $opt(x)/2
$a_($num) cmd set-target-y $opt(y)/2
$a_($num) cmd set-target-z $opt(z)
$a_($num) cmd set-filename $opt(datafile)
$a_($num) cmd set-packetsize $opt(packet_size) ;# # of bytes
$a_($num) set data_rate_ [expr 1.0/$opt(interval)]

$ns_ at 2.0 "$node_($num) move"

# make nam workable
set node_size 10
for {set k 0} { $k<$opt(nn)} {incr k} {
$ns_ initial_node_pos $node_($k) $node_size
}


set opt(stop2) [expr $opt(stop)+200]


puts "Node $total_number is sending first!!"
$ns_ at 1.33 "$a_($num) cbr-start"
$ns_ at $opt(stop).001 "$a_($num) terminate"
$ns_ at $opt(stop2).002 "$a_(0) terminate"


$ns_ at $opt(stop2).003 "$god_ compute_energy"
$ns_ at $opt(stop2).004 "$ns_ nam-end-wireless $opt(stop)"
$ns_ at $opt(stop2).005 "puts \"NS EXISTING...\"; $ns_ halt"


puts $data "New simulation...."
puts $data "nodes = $opt(nn), maxspeed = $opt(maxspeed), minspeed = $opt(minspeed), random_seed = $opt(seed), sending_interval_=$opt(interval))"
puts $data "x= $opt(x) y= $opt(y) z= $opt(z)"
close $data
puts "starting Simulation..."
$ns_ run
