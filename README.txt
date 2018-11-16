Maximum Lifetime Data Aggreagation in Wireless Sensor Networks. 

The project consist of the Maximum lifetime data routing method for 
simulating a Wireless Sensor Network based on energy constraints and 
the sensing range of each sensor nodes. The programe is stored in a 
folder named "mldr" which needs to be added to  ns2 and re-compiled. 
The test run (tcl file) can be used to test the protocol. 

The current run of the protocol has been done on Linux mint 17.1

Tools : 

>> ns2.354(Can be downloaded from https://sourceforge.net/projects/nsnam/files/allinone/ns-allinone-2.34/)
>> nam 1.14

Installation : 

Installing ns2.34 in Linux

In the terminal, type the following. 

STEP 1: ~$ sudo apt-get install gcc-4.4 g++-4.4
STEP 2: ~$ sudo apt-get install libxt-dev libx11-dev libxmu-dev xorg-dev xgraph
STEP 3:  extract ns-allinone-2.34.tar.gz into home directory. 
STEP 4: In terminal, change directory to ns-allinone-2.34 
	~$ cd /home/<your home folder>/ns-allinone-2.34
STEP 5: Install ns2.34
	~$ ./install
STEP 6: open file configure in /home/<your home foilder>/ns-allinone-2.34/otcl1.13/configure 
	and make the following changes 
	Linux*)
	SHLIB_CFLAGS=”-fpic”
	#SHLIB_LD=”ld -shared”       //disabled this line
	SHLIB_LD=”gcc -shared”       // add this line
	SHLIB_SUFFIX=”.so”
	DL_LIBS=”-ldl”
	SHLD_FLAGS=””
	;;

STEP 7: open file ranvar.cc in  /home/<your home foilder>/ns-allinone-2.34/ns-2.34/tools/ranvar.cc
	and make following changes. 
	double GammaRandomVariable::value()
	{
		// Proposed by Marsaglia in 2000:
		// G. Marsaglia, W. W. Tsang: A simple method for gereating Gamma variables
		// ACM Transactions on mathematical software, Vol. 26, No. 3, Sept. 2000
		if (alpha_ < 1) {
			  double u = rng_->uniform(1.0);
			  //return GammaRandomVariable::GammaRandomVariable(1.0 + alpha_, beta_).value() * pow (u, 1.0 / alpha_); /*  Disabled*/
			  return GammaRandomVariable(1.0 + alpha_, beta_).value() * pow (u, 1.0 / alpha_); /*add this line*/
	}

STEP 8: open file nakagami.cc from /home/<your home foilder>/ns-allinone-2.34/ns-2.34/mobile/nakagami.cc
	and make the following changes.
	if (int_m == m) {
		//resultPower = ErlangRandomVariable::ErlangRandomVariable(Pr/m, int_m).value();
		resultPower = ErlangRandomVariable(Pr/m, int_m).value();
	} else {
		//resultPower = GammaRandomVariable::GammaRandomVariable(m, Pr/m).value();
		resultPower = GammaRandomVariable(m, Pr/m).value();
	}

STEP 9: open Makefile.in from /home/<your home foilder>/ns-allinone-2.34/tcl8.4.18/Makefile.in
	and make the following changes 	
	#CC = @CC@ 
	CC = @CC@-4.4
STEP 10: open Makefile.in from /home/<your home foilder>/ns-allinone-2.34/Makefile.in
	and make the following changes 
	#CC = @CC@ 
	#CPP = @CXX@
	CC = @CC@-4.4
	CPP = @CXX@-4.4
STEP 11: open Makefile from /home/<your home foilder>/ns-allinone-2.34/Makefile
	and make the following changes 
	#CC = gcc
	#CPP = g++
	CC = gcc-4.4	
	CPP = g++-4.4
STEP 12: Re-run
	Repeat Step 5. 

refer link : https://abdusyarif.wordpress.com/2012/07/11/solved-problem-step-by-step-installing-ns-2-34-ubuntu-12-04-lts/


Installing the MLDR files in ns2. 
Make changes to the following files 

$NS_ROOT/Makefile
$NS_ROOT/queue/priqueue.cc
$NS_ROOT/common/packet.h
$NS_ROOT/trace/cmu-trace.h
$NS_ROOT/trace/cmu-trace.cc
$NS_ROOT/tcl/lib/ns-packet.tcl
$NS_ROOT/tcl/lib/ns-lib.tcl
$NS_ROOT/tcl/lib/ns-agent.tcl
$NS_ROOT/tcl/lib/ns-mobilenode.tcl

STEP 1:copy the folders "mldr" and "myagent" into /home/<your home foilder>/ns-allinone-2.34/ns-2.34/
STEP 2:open /ns-allinone-2.34/ns-2.34/Makefile add following line at 269
	mldr/myaodv.o
	mdlr/myaodv_logs.o
	mldr/myaodv_rqueue.o
	mldr/myaodv_rtable.o
	myagent/myagent.o
STEP 3: Add following lines to ~/ns-allinone-2.34/ns-2.34/queue/priqueue.cc from line 93.
	//MLDR PATCH
	case PT_MYAODV:
STEP 4:To define new routing protocol packet type we have to modify ~/ns-allinone-2.34/ns-2.34/common/packet.h file.
 	We change PT_NTYPE to 63, and for our protocol PT_MYAODV = 62. If you have already installed another routing protocol. 
	Just make sure PT_NTYPE is last, and protocol number is ordered sequentially. 
	// MYAODV packet
	static const packet_t PT_MYAODV = 62;
	// insert new packet types here
	static packet_t PT_NTYPE = 63; // This MUST be the LAST one
STEP 5: change at line 254 of ~/ns-allinone-2.34/ns-2.34/common/packet.h. 
	type == PT_AODV ||
	type == PT_MYAODV)	
STEP 6: at line 390 of the same file
	// MYAODV patch
	name_[PT_MYAODV] = "MYAODV";
STEP 7: To add trace function we add following line to ~/ns-allinone-2.34/ns-2.34/trace/cmu-trace.h at line 163:
	void    format_myaodv(Packet *p, int offset);
STEP 8: ~/ns-allinone-2.34/ns-2.34/trace/cmu-trace.cc must be added following code at line 1071
	// MYAODV patch

void
	CMUTrace::format_myaodv(Packet *p, int offset)
	{
	    struct hdr_myaodv *wh = HDR_MYAODV(p);
	    struct hdr_myaodv_beacon *wb = HDR_MYAODV_BEACON(p);
	    struct hdr_myaodv_error  *we = HDR_MYAODV_ERROR(p);
	     switch(wh->pkt_type) {
	        case MYAODV_BEACON:
	            if (pt_->tagged()) {
	                sprintf(pt_->buffer() + offset,
	                          "-myaodv:t %x -myaodv:h %d -myaodv:b %d -myaodv:s %d "
	                          "-myaodv:px %d -myaodv:py %d -myaodv:ts %f "
	                          "-myaodv:c BEACON ",
	                          wb->pkt_type,
	                          wb->beacon_hops,
	                          wb->beacon_id,
        	                  wb->beacon_src,
	                          wb->beacon_posx,
	                          wb->beacon_posy,
	                          wb->timestamp);
	            } else if (newtrace_) {
	                sprintf(pt_->buffer() + offset,
	                          "-P myaodv -Pt 0x%x -Ph %d -Pb %d -Ps %d -Ppx %d -Ppy %d -Pts %f -Pc BEACON ",
	                          wb->pkt_type,
	                          wb->beacon_hops,
	                         wb->beacon_id,
	                          wb->beacon_src,
	                        wb->beacon_posx,
	                          wb->beacon_posy,
	                          wb->timestamp);
	            } else {
	                sprintf(pt_->buffer() + offset,
	                         "[0x%x %d %d [%d %d] [%d %f]] (BEACON)",
	                          wb->pkt_type,
	                          wb->beacon_hops,
	                          wb->beacon_id,
	                          wb->beacon_src,
	                         wb->beacon_posx,
	                          wb->beacon_posy,
	                          wb->timestamp);
	            }
	            break;
		       case MYAODV_ERROR:
	            // TODO: need to add code
		            break;
	      default:
	#ifdef WIN32
        	    fprintf(stderr,
                	      "CMUTrace::format_myaodv: invalid MLDR packet typen");
	#else
	           fprintf(stderr,
        	            "%s: invalid MLDR packet typen", __FUNCTION__);
	#endif
	           abort();
 	  }
	}

STEP  9: modify ~/ns-allinone-2.34/ns-2.34/tcl/lib/ns-packet.tcl @ line 172
	# MYAODV patch	
	MYAODV
STEP 10: modify ~/ns-allinone-2.34/ns-2.34/tcl/lib/ns-lib.tcl @ line 633
	MYAODV {
  	set ragent [$self create-MYAODV-agent $node]
	}
STEP 11: from line 860 of the same file following code should be added.
	Simulator instproc create-MYAODV-agent { node } {
	   #  Create MYAODV routing agent
	   set ragent [new Agent/MYAODV [$node node-addr]]
	   $self at 0.0 "$ragent start"
	   $node set ragent_ $ragent
	   return $ragent
	}
STEP 12: Modify ~/ns-allinone-2.34/ns-2.34/tcl/lib/ns-agent.tcl line 202
	Agent/MYAODV instproc init args {
	  $self next $args
	}
	Agent/MYAODV set sport_   0
	Agent/MYAODV set dport_   0

STEP 13: change in file ~/ns-allinone-2.34/ns-2.34/tcl/lib/ns-mobilenode.tcl line 201
	# Special processing for MYAODV
	set MYAODVonly [string first "MYAODV" [$agent info class]]
	if {$MYAODVonly != -1 } {
	  $agent if-queue [$self set ifq_(0)]   ;# ifq between LL and MAC
	}
STEP 14: go to ~/ns-allinone-2.34/ns-2.34/ directory on the terminal and run the following command
	./configure
	make clean
	make
	make install
STEP 15: run the test file mldr_test.tcl on a terminal. 
	ns mldr_test.tcl

refer link : http://elmurod.net/en/index.php/archives/157

