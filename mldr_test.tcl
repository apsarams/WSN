#####################################################################################################
################################  Maximum Lifetime Data Routing #####################################
#####################################################################################################

Node/MobileNode instproc getPhy {ifa } {
$self instvar netif_
return $netif_($ifa)
}

set val(chan)          Channel/WirelessChannel      ;# channel type
set val(prop)          Propagation/TwoRayGround     ;# radio-propagation model
set val(netif)         Phy/WirelessPhy/802_15_4     ;# network interface type
set val(mac)           Mac/802_15_4                 ;# MAC type
set val(ifq)           Queue/DropTail/PriQueue      ;# interface queue type
set val(ll)            LL                           ;# link layer type
set val(ant)           Antenna/OmniAntenna          ;# antenna model
set val(ifqlen)        100	         	    ;# max packet in ifq
set val(nn)            30			    ;# number of mobilenodes
set val(rp)            MYAODV			    ;# protocol tye
set val(x)             1000			    ;# X dimension of topography
set val(y)             1000			    ;# Y dimension of topography
set val(stop)          100			    ;# simulation period 
set val(energymodel)   EnergyModel		    ;# Energy Model
set val(initialenergy) 0.1			    ;# value

#create simualtor
set ns [new Simulator]

#open trace files 
for { set i 0 } { $i < $val(nn) } { incr i } {
	set mytrace($i) [open mytrace($i).tr w]
}
#set mytrace2 [open mytrace2.tr w]
#set mytrace3 [open mytrace3.tr w]
set tracefd [open distance_routing.tr w]
$ns trace-all $tracefd
set namtrace [open distance_routing.nam w]
$ns namtrace-all-wireless $namtrace 100 100

#set distance
set dist(5m)  7.69113e-06
set dist(9m)  2.37381e-06
set dist(10m) 1.92278e-06
set dist(11m) 1.58908e-06
set dist(12m) 1.33527e-06
set dist(13m) 1.13774e-06
set dist(14m) 9.81011e-07
set dist(15m) 8.54570e-07
set dist(16m) 7.51087e-07
set dist(20m) 4.80696e-07
set dist(25m) 3.07645e-07
set dist(30m) 2.13643e-07
set dist(35m) 1.56962e-07
set dist(40m) 1.20174e-07
Phy/WirelessPhy set CSThresh_ $dist(40m)
Phy/WirelessPhy set RXThresh_ $dist(40m)
Phy/WirelessPhy set  pt_ 0.001

#Specified Parameters for 802.15.4 MAC
Mac/802_15_4 wpanCmd verbose on   ;# to work in verbose mode 
Mac/802_15_4 wpanNam namStatus on  ;# default = off (should be turned on before other 'wpanNam' commands can work)

# set up topography object
set topo       [new Topography]
$topo load_flatgrid $val(x) $val(y)

create-god $val(nn)

###########################################################################################
#########################  configure the nodes  ###########################################
###########################################################################################
 $ns node-config -adhocRouting $val(rp) \
             -llType $val(ll) \
             -macType $val(mac) \
             -ifqType $val(ifq) \
             -ifqLen $val(ifqlen) \
             -antType $val(ant) \
             -propType $val(prop) \
             -phyType $val(netif) \
             -channel [new $val(chan)] \
             -topoInstance $topo \
             -agentTrace ON \
             -routerTrace ON \
             -macTrace  OFF \
             -movementTrace OFF \
             -energyModel $val(energymodel) \
             -rxPower 30.00e-3 \
             -txPower 30.00e-3 \
	     -idlePower 712e-6 \
	     -sleepPower 144e-9 
	     
 $ns node-config -initialEnergy 0.9
set n(0) [$ns node]    
 $ns node-config -initialEnergy 0.04
set n(1) [$ns node] 	   
             
#######################################################################################################
#########################  CREATE AND CONFIGURE NODES  ################################################
#######################################################################################################
$ns node-config -initialEnergy 0.04 
for {set i 2} {$i < $val(nn) } { incr i } {
        set n($i) [$ns node]
}
  

 
#------------------------------------ position of the nodes -------------------------------------------#
for {set i 1} {$i < $val(nn) } { incr i } {
	$n($i) set X_ [ expr { $val(x) * rand() } ]
	$n($i) set Y_ [ expr { $val(y) * rand() } ]
	$n($i) set Z_ 0
}

#------------------------------------- position of sink ----------------------------------------------#
$n(0) set X_ [ expr {$val(x)/2 + 5} ]
$n(0) set Y_ [ expr {$val(y)/2 + 5} ]
$n(0) set Z_ 0.0
$n(0) label "Base Station"

for {set i 0} {$i < $val(nn)} { incr i } {
	$ns initial_node_pos $n($i) 5
}
###########################################################################################################
################################  SET CONNECTIONS AND TRAFFIC FOR NODES ###################################
###########################################################################################################

#----------------------------------- Setup a UDP connection ----------------------------------------------#
for {set j 1} {$j < $val(nn) } { incr j } {
	set udp($j) [new Agent/UDP]
	$ns attach-agent $n($j) $udp($j)
}

set udp(0) [new Agent/Null]
$ns attach-agent $n(0) $udp(0)

for {set j 1} {$j < $val(nn) } { incr j } {
	$ns connect $udp($j) $udp(0)
	$udp($j) set fid_ 2
}

#---------------------------------------- Setup a CBR over UDP connection ---------------------------------------#
for {set k 1} {$k < $val(nn) } { incr k } {
	set cbr($k) [new Application/Traffic/CBR]
	$cbr($k) attach-agent $udp($k)
	#$cbr($k) set type_ CBR
	$cbr($k) set packet_size_ 10
	$cbr($k) set interval_ 1
	$cbr($k) set random_ 0
	#$cbr($k) set rate_ 0.1Mb
}
#-------------- TEST SET-UP OF NODE 1 -------------------#

for { set a 0 } { $a < $val(nn) } { incr a } {
	set myagent($a) [new Agent/Energy]
	$ns attach-agent $n(0) $myagent($a)
}
#--------------------------------------------------------#
####################################################################################################################
#################################  MONITORING NODES  ###############################################################
####################################################################################################################

proc record { mytrace myagent node1 } {
     
   set ns1 [Simulator instance]
   set mytime 0.5 
   set now [$ns1 now]
   set id [$node1 node-addr]
   $myagent set node_id [$node1 node-addr] 
   $myagent get-energy 			   
   set enrg [$myagent set energy_]	   
   foreach {index link} [$node1 neighbors] {
   
		set L [split $index :] 
		set n1 [lindex $L 0]
		set n2 [lindex $L 0]
		
		#check if route exists
		$myagent check-route [$n1 node-addr]
		set exists [$myagent set exists]
		
		if {
			$exists == 0	
		} then {
			$myagent make-route [$n1 node-addr]	#add route
		}
		
		#$node1 add-route $n1 $n2
		#puts "route added [$n($i) node-addr] --- > [lindex $L $j]"

   }
   #if { $id == 1 || $id == 2} { 
   puts $mytrace "$now\t$enrg" 	
   #} 
   $ns1 at [expr $now + $mytime] "record $mytrace $myagent $node1"
   
}
#------------------ get RXThresh value ----------------------#
set phy [$n(0) getPhy 0]
set rx [$phy set RXThresh_]
#------------------------------------------------------------#

#------------------------------------- GET TREE BY CHECKING AND ADDING NODES -----------------------------------------#
set now [$ns now]
for { set i 0 } { $i < $val(nn) } { incr i } {
	#for { set j 0 } { $j < $val(nn) } { incr j } {
	$myagent($i) set node_id [$n($i) node-addr]	
   	$myagent($i) get-energy				
   	set enrg($i) [$myagent($i) set energy_]		
   	if {
   		$enrg($i) ==  0.0
   	} then {
   		$ns detach-agent $n($i) $udp($i)
   		$myagent($i) delete-route [$n($i) node-addr]
   	}
   	if { 
   		$i != 0 
   	} then {
		$ns at 0.0 "start_sim $val(nn) $i $cbr($i)"
	} 
	if { $i == 1 } { 
   		puts $mytrace($i) "$now\t$enrg($i)" 
   		$ns at 1.0 "record $mytrace($i) $myagent($i) $n($i)"
	} elseif { $i == 2 } { 
   		puts $mytrace($i) "$now\t$enrg($i)" 
   		$ns at 1.0 "record $mytrace($i) $myagent($i) $n($i)"
   	} elseif { $i == 3 } { 
   		puts $mytrace($i) "$now\t$enrg($i)" 
   		$ns at 1.0 "record $mytrace($i) $myagent($i) $n($i)"
   	} elseif { $i == 9 } { 
   		puts $mytrace($i) "$now\t$enrg($i)" 
   		$ns at 1.0 "record $mytrace($i) $myagent($i) $n($i)"
   	} elseif { $i == 4 } { 
   		puts $mytrace($i) "$now\t$enrg($i)" 
   		$ns at 1.0 "record $mytrace($i) $myagent($i) $n($i)"
   	} elseif { $i == 5 } { 
   		puts $mytrace($i) "$now\t$enrg($i)" 
   		$ns at 1.0 "record $mytrace($i) $myagent($i) $n($i)"
   	} elseif { $i == 6 } { 
   		puts $mytrace($i) "$now\t$enrg($i)" 
   		$ns at 1.0 "record $mytrace($i) $myagent($i) $n($i)"
   	} elseif { $i == 7 } { 
   		puts $mytrace($i) "$now\t$enrg($i)" 
   		$ns at 1.0 "record $mytrace($i) $myagent($i) $n($i)"
   	} elseif { $i == 8 } { 
   		puts $mytrace($i) "$now\t$enrg($i)" 
   		$ns at 1.0 "record $mytrace($i) $myagent($i) $n($i)"
   	} else {
   	
   		$ns at 1.0 "record $mytrace($i) $myagent($i) $n($i)"
   	} 
	#$ns at 1.0 "record $mytrace $myagent($i) $n($i)"
	#}
	
}


######################################################################################################################
##################################################  START SIMULATION #################################################
######################################################################################################################
 
proc start_sim { nn k cbr} {
	set ns [Simulator instance]
	for { set n 0 } { $n < $nn  } { incr n } {
		$ns at [ expr { ($n * 10)+ $k } ] "$cbr start"
		$ns at [ expr { ($n * 10) + $k + 0.9 }] "$cbr stop"
	}
	
}

######################################################################################################################
####################################  ENDING SIMULATION  #############################################################
######################################################################################################################

#-------------------------------- Telling nodes when the simulation ends --------------------------------------------#
for {set i 0} {$i < $val(nn) } { incr i } {
    $ns at $val(stop) "$n($i) reset;"
}


#------------------------------------- ending nam and the simulation  -----------------------------------------------#
$ns at $val(stop) "$ns nam-end-wireless $val(stop)"
$ns at $val(stop) "stop"
$ns at [expr $val(stop) + 0.01] "puts \"end simulation\"; $ns halt"

#######################################################################################################################
#####################################   STOP PROCEDURE  ###############################################################
#######################################################################################################################

proc stop {} {
    global ns tracefd namtrace
    $ns flush-trace
    
    close $tracefd
    close $namtrace
    exec xgraph mytrace(1).tr mytrace(2).tr mytrace(3).tr mytrace(4).tr mytrace(5).tr mytrace(6).tr mytrace(7).tr mytrace(8).tr mytrace(9).tr mytrace(10).tr mytrace(11).tr mytrace(12).tr mytrace(13).tr mytrace(14).tr mytrace(15).tr mytrace(16).tr mytrace(17).tr mytrace(18).tr mytrace(19).tr mytrace(20).tr mytrace(21).tr mytrace(22).tr mytrace(23).tr mytrace(24).tr mytrace(25).tr mytrace(26).tr mytrace(27).tr mytrace(28).tr mytrace(29).tr -geometry 800x400 &
    exec nam distance_routing.nam &
    exit 0
}
#------------ RUN SIMULATION ------------------#
$ns run


