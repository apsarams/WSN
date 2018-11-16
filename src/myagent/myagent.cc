#include <stdio.h>
#include <string.h>
#include "agent.h"
#include <myaodv/myaodv.h>
#include <myaodv/myaodv_rtable.h>
class MyAgent : public Agent {
public:
        MyAgent();
protected:
        int command(int argc, const char*const* argv);
public: 
	MYAODV* myaodvagent;
	double energy_;
	nsaddr_t node_id;
	//double rx_value;
	//double cs_value;
	//void sendRXThresh( double rx);
	//void sendCSThresh( double cs);
	//void set_neighbors(nsaddr_t id);
	void set_route(nsaddr_t id);
	void check_route(nsaddr_t id);
	void delete_route(nsaddr_t id);
	int exists;
};

static class MyAgentClass : public TclClass {
public:
       MyAgentClass() : TclClass("Agent/Energy") {}
        TclObject* create(int argc, const char*const* argv) {
                return(new MyAgent());
        }
} class_My_agent;

MyAgent::MyAgent() : Agent(PT_MYAODV) {
       bind("energy_", &energy_);
       bind("node_id", &node_id);
       //bind("rx_value", &rx_value);
       energy_ = 0.0;
      /* bind("My_var2_otcl", &My_var2);*/
}
int MyAgent::command(int argc, const char*const* argv) {
      myaodvagent = new MYAODV(node_id);
  
      if(argc == 2) {
           if(strcmp(argv[1], "get-energy") == 0) {
                  //retrieve energy of the node from node in aodv --> argv[2]
                  energy_ = myaodvagent->getEnergy(); 
                  
                  return(TCL_OK);
           }
           if(strcmp(argv[1], "make-route") == 0) {
           	  int num = atoi(argv[2]);
		  nsaddr_t a = (nsaddr_t)num;
		  set_route(a);
		  
		  return(TCL_OK);
           }
           if(strcmp(argv[1], "check-route") == 0) {
		  int num = atoi(argv[2]);
		  nsaddr_t a = (nsaddr_t)num;
		  check_route(a);
		  
		  return(TCL_OK);
           }
           if(strcmp(argv[1], "delete-route") == 0) {
		  int num = atoi(argv[2]);
		  nsaddr_t a = (nsaddr_t)num;
		  delete_route(a);
		  
		  return(TCL_OK);
           }
           
      }
     return(Agent::command(argc, argv));
}
/*void MyAgent::sendRXThresh(double rx) {
	rx_value = rx;
}
void MyAgent::sendCSThresh(double cs) {
	cs_value = cs;
}

void MyAgent::set_neighbors(nsaddr_t id) {
	myaodv_rt_entry *entry = new myaodv_rt_entry();
	MYAODV_Neighbor* nb = new MYAODV_Neighbor(myaodvagent->index);
	nb = entry->nb_lookup(id);
	if(nb != NULL){
		set_route(id);
	}
	//else set the neighbour. 
	
}
*/

void MyAgent::set_route(nsaddr_t id) {
	myaodv_rt_entry* rt;
	rt = myaodvagent->rtable.rt_add(id);
	//reverse route
}
void MyAgent::check_route(nsaddr_t id) {
	myaodv_rt_entry* rt;
	rt = myaodvagent->rtable.rt_lookup(id);
	if(rt){
		exists = 1;
	}
	else {
		exists = 0;
	}
	//reverse route
}
void MyAgent::delete_route(nsaddr_t id) {
	myaodv_rt_entry* rt;
	rt = myaodvagent->rtable.rt_lookup(id);
	myaodvagent->rt_down(rt);
	//reverse route
}

