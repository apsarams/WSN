/*
Copyright (c) 1997, 1998 Carnegie Mellon University.  All Rights
Reserved. 

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The AODV code developed by the CMU/MONARCH group was optimized and tuned by Samir Das and Mahesh Marina, University of Cincinnati. The work was partially done in Sun Microsystems.

*/

#ifndef __myaodv_h__
#define __myaodv_h__

//#include <agent.h>
//#include <packet.h>
//#include <sys/types.h>
//#include <cmu/list.h>
//#include <scheduler.h>

#include <cmu-trace.h>
#include <priqueue.h>
#include <myaodv/myaodv_rtable.h>
#include <myaodv/myaodv_rqueue.h>
#include <classifier/classifier-port.h>
#include <mobilenode.h>  //added by anu
//#include <wireless-phy.h>
/*
  Allows local repair of routes 
*/
#define MY_AODV_LOCAL_REPAIR

/*
  Allows AODV to use link-layer (802.11) feedback in determining when
  links are up/down.
*/
#define MY_AODV_LINK_LAYER_DETECTION

/*
  Causes AODV to apply a "smoothing" function to the link layer feedback
  that is generated by 802.11.  In essence, it requires that RT_MAX_ERROR
  errors occurs within a window of RT_MAX_ERROR_TIME before the link
  is considered bad.
*/
#define MY_AODV_USE_LL_METRIC

/*
  Only applies if AODV_USE_LL_METRIC is defined.
  Causes AODV to apply omniscient knowledge to the feedback received
  from 802.11.  This may be flawed, because it does not account for
  congestion.
*/
//#define AODV_USE_GOD_FEEDBACK


class MYAODV;

#define MY_ROUTE_TIMEOUT        10                      	// 100 seconds
#define ACTIVE_ROUTE_TIMEOUT    10				// 50 seconds
#define REV_ROUTE_LIFE          6				// 5  seconds
#define BCAST_ID_SAVE           6				// 3 seconds


// No. of times to do network-wide search before timing out for 
// MAX_RREQ_TIMEOUT sec. 
#define RREQ_RETRIES            3  
// timeout after doing network-wide search RREQ_RETRIES times
#define MAX_RREQ_TIMEOUT	10.0 //sec

/* Various constants used for the expanding ring search */
#define TTL_START     5
#define TTL_THRESHOLD 7
#define TTL_INCREMENT 2 

// This should be somewhat related to arp timeout
#define NODE_TRAVERSAL_TIME     0.03             // 30 ms
#define LOCAL_REPAIR_WAIT_TIME  0.15 //sec

// Should be set by the user using best guess (conservative) 
#define NETWORK_DIAMETER        30             // 30 hops

// Must be larger than the time difference between a node propagates a route 
// request and gets the route reply back.

//#define RREP_WAIT_TIME     (3 * NODE_TRAVERSAL_TIME * NETWORK_DIAMETER) // ms
//#define RREP_WAIT_TIME     (2 * REV_ROUTE_LIFE)  // seconds
#define RREP_WAIT_TIME         1.0  // sec

#define ID_NOT_FOUND    0x00
#define ID_FOUND        0x01
//#define INFINITY        0xff

// The followings are used for the forward() function. Controls pacing.
#define DELAY 1.0           // random delay
#define NO_DELAY -1.0       // no delay 

// think it should be 30 ms
#define ARP_DELAY 0.01      // fixed delay to keep arp happy


#define HELLO_INTERVAL          1               // 1000 ms
#define ALLOWED_HELLO_LOSS      3               // packets
#define BAD_LINK_LIFETIME       3               // 3000 ms
#define MaxHelloInterval        (1.25 * HELLO_INTERVAL)
#define MinHelloInterval        (0.75 * HELLO_INTERVAL)

/*
  Timers (Broadcast ID, Hello, Neighbor Cache, Route Cache)
*/
class MyBroadcastTimer : public Handler {
public:
        MyBroadcastTimer(MYAODV* a) : agent(a) {}
        void	handle(Event*);
private:
        MYAODV    *agent;
	Event	intr;
};

class MyHelloTimer : public Handler {
public:
        MyHelloTimer(MYAODV* a) : agent(a) {}
        void	handle(Event*);
private:
        MYAODV    *agent;
	Event	intr;
};

class MyNeighborTimer : public Handler {
public:
        MyNeighborTimer(MYAODV* a) : agent(a) {}
        void	handle(Event*);
private:
        MYAODV    *agent;
	Event	intr;
};

class MyRouteCacheTimer : public Handler {
public:
        MyRouteCacheTimer(MYAODV* a) : agent(a) {}
        void	handle(Event*);
private:
        MYAODV    *agent;
	Event	intr;
};

class MyLocalRepairTimer : public Handler {
public:
        MyLocalRepairTimer(MYAODV* a) : agent(a) {}
        void	handle(Event*);
private:
        MYAODV    *agent;
	Event	intr;
};


/*
  Broadcast ID Cache
*/
class BroadcastID {
        friend class MYAODV;
 public:
        BroadcastID(nsaddr_t i, u_int32_t b) { src = i; id = b;  }
 protected:
        LIST_ENTRY(BroadcastID) link;
        nsaddr_t        src;
        u_int32_t       id;
        double          expire;         // now + BCAST_ID_SAVE s
};

LIST_HEAD(myaodv_bcache, BroadcastID);


/*
  The Routing Agent
*/
class MYAODV: public Agent {

  /*
   * make some friends first 
   */

        friend class myaodv_rt_entry;
        friend class MyBroadcastTimer;
        friend class MyHelloTimer;
        friend class MyNeighborTimer;
        friend class MyRouteCacheTimer;
        friend class MyLocalRepairTimer;

 public:
        MYAODV(nsaddr_t id);

        void		recv(Packet *p, Handler *);

 protected:
        int             command(int, const char *const *);
        int             initialized() { return 1 && target_; }

        /*
         * Route Table Management
         */
        void            rt_resolve(Packet *p);
        void            rt_update(myaodv_rt_entry *rt, u_int32_t seqnum,
		     	  	u_int16_t metric, nsaddr_t nexthop,
		      		double expire_time);
       
        void            local_rt_repair(myaodv_rt_entry *rt, Packet *p);
 public:
  	void            rt_down(myaodv_rt_entry *rt);
        void            rt_ll_failed(Packet *p);
        void            handle_link_failure(nsaddr_t id);
 protected:
        void            rt_purge(void);

        void            enque(myaodv_rt_entry *rt, Packet *p);
        Packet*         deque(myaodv_rt_entry *rt);

        /*
         * Neighbor Management
         */
        void            nb_insert(nsaddr_t id);
        MYAODV_Neighbor*       nb_lookup(nsaddr_t id);
        void            nb_delete(nsaddr_t id);
        void            nb_purge(void);

        /*
         * Broadcast ID Management
         */

        void            id_insert(nsaddr_t id, u_int32_t bid);
        bool	        id_lookup(nsaddr_t id, u_int32_t bid);
        void            id_purge(void);

        /*
         * Packet TX Routines
         */
        void            forward(myaodv_rt_entry *rt, Packet *p, double delay);
        void            sendHello(void);
        void            sendRequest(nsaddr_t dst);

        void            sendReply(nsaddr_t ipdst, u_int32_t hop_count,
                                  nsaddr_t rpdst, u_int32_t rpseq,
                                  u_int32_t lifetime, double timestamp);
        void            sendError(Packet *p, bool jitter = true);
                                          
        /*
         * Packet RX Routines
         */
        void            recvMYAODV(Packet *p);
        void            recvHello(Packet *p);
        void            recvRequest(Packet *p);
        void            recvReply(Packet *p);
        void            recvError(Packet *p);

	/*
	 * History management
	 */
	
	double 		PerHopTime(myaodv_rt_entry *rt);


        
        u_int32_t       seqno;                  // Sequence Number
        int             bid;                    // Broadcast ID

        myaodv_rtable         rthead;                 // routing table
        myaodv_ncache         nbhead;                 // Neighbor Cache
        myaodv_bcache          bihead;                 // Broadcast ID Cache
        
        
	/* 
	* Energy & position variables for node : anu  
	*/
	double xpos;
	double ypos;
	double zpos; 
	double iEnergy;
	//double rx_value;
	MobileNode *iNode;
public: 
	nsaddr_t        index;                  // IP Address of this node
	double   getEnergy();
	//double   getRX();
	
        /*
         * Timers
         */
        MyBroadcastTimer  btimer;
        MyHelloTimer      htimer;
        MyNeighborTimer   ntimer;
        MyRouteCacheTimer rtimer;
        MyLocalRepairTimer lrtimer;

        /*
         * Routing Table
         */
        myaodv_rtable          rtable;
        /*
         *  A "drop-front" queue used by the routing layer to buffer
         *  packets to which it does not have a route.
         */
        myaodv_rqueue         rqueue;

        /*
         * A mechanism for logging the contents of the routing
         * table.
         */
        Trace           *logtarget;

        /*
         * A pointer to the network interface queue that sits
         * between the "classifier" and the "link layer".
         */
        PriQueue        *ifqueue;

        /*
         * Logging stuff
         */
        void            log_link_del(nsaddr_t dst);
        void            log_link_broke(Packet *p);
        void            log_link_kept(nsaddr_t dst);

	/* for passing packets up to agents */
	PortClassifier *dmux_;

};

#endif /* __myaodv_h__ */