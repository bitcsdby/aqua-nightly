
//#include "delay.h"
//#include "connector.h"

#include "packet.h"
#include "random.h"
#include "underwatersensor/uw_common/underwatersensornode.h"
#include "mac.h"
#include "broadcastmac.h"
#include "underwaterphy.h"


StatusHandler:: StatusHandler(BroadcastMac*  p):mac_(p){}
void StatusHandler::handle(Event* e)
{ 
  //  printf("statushandler: I am done with status handler\n");
  mac_->StatusProcess(e);
 
}

CallbackHandler:: CallbackHandler(BroadcastMac*  p):mac_(p){}

void CallbackHandler::handle(Event* e)
{ 
  //  printf("callbackhandler: I am done with status handler\n");
  mac_->CallbackProcess(e); 
 
}





BackoffHandler::BackoffHandler(BroadcastMac* p):mac_(p){counter=0;}

void BackoffHandler::handle(Event* e)
{ 
  counter++;
  if(counter<MAXIMUMCOUNTER)
  //printf("backoffhandler: I am in backoff\n");
    mac_->TxProcess((Packet*)e);
    else 
    {
     clear();
 printf("backoffhandler: too many backoffs\n");
 // mac_->callback_->handle(callback_event);
      mac_->CallbackProcess(e);
      mac_->DropPacket((Packet*) e);
      //  Packet::free((Packet*)e);
    }
}

void BackoffHandler::clear()
{ 
  counter=0;
}



/* ======================================================================
   Broadcast MAC for  underwater sensor
   ====================================================================== */
static class BroadcastMacClass : public TclClass {
public:
 BroadcastMacClass():TclClass("Mac/UnderwaterMac/BroadcastMac") {}
   TclObject* create(int, const char*const*) {
	  return (new BroadcastMac());
   }
}class_broadcastmac;


BroadcastMac::BroadcastMac() : UnderwaterMac(),status_handler(this),backoff_handler(this),callback_handler(this)
{
  bind("packetheader_size_",&packetheader_size_); 
}

/*
 this program is used to handle the received packet, 
it should be virtual function, different class may have 
different versions.
*/

void 
BroadcastMac::RecvProcess(Packet* pkt){
  char* mh=(char*)pkt->access(hdr_mac::offset_);
   hdr_cmn* cmh=HDR_CMN(pkt);

  assert(initialized());
  int dst=this->hdr_dst(mh);


    if (cmh->error()) 
     {
     printf("broadcast:node %d  gets a corrupted packet at  %f\n",index_,NOW);
     if(drop_) drop_->recv(pkt,"Error/Collision");
     else Packet::free(pkt);
     return;
     }

  if(dst==MAC_BROADCAST){
    
    //     hdr_cmn* cmh=HDR_CMN(pkt);
      //  printf("underwaterbroadcastmac :a broadcast packet ptype=%d !!\n",cmh->ptype_);
    uptarget_->recv(pkt, this);
    return;
  }

   if(dst==index_){
     // printf("underwaterbroadcastmac:this is my packet \n");
    uptarget_->recv(pkt, this);
    return;
}
    printf("underwaterbroadcastmac: this is neither broadcast nor my packet, just drop it\n");
   Packet::free(pkt);
   return;
}


void 
BroadcastMac::DropPacket(Packet* pkt)
{
  {
     printf("broadcast:node %d  gets a stucked packet at  %f\n",index_,NOW);
     if(drop_) drop_->recv(pkt,"Stucked");
     else Packet::free(pkt);
     return;
     }
}


/*
 this program is used to handle the transmitted packet, 
it should be virtual function, different class may have 
different versions.
*/


void 
BroadcastMac::TxProcess(Packet* pkt){
  

  
  hdr_cmn* cmh=HDR_CMN(pkt);
  assert(initialized());
  UnderwaterSensorNode* n=(UnderwaterSensorNode*) node_;
  // printf("broadcast node receive down packet %d  at  %f \n",cmh->ptype_,NOW);

   cmh->size()+=(packetheader_size_*8);
   printf("broadcast:size is %d  at  %f \n",cmh->size(),NOW);
  
  hdr_cmn::access(pkt)->txtime()=(cmh->size()*encoding_efficiency_)/bit_rate_;
   double txtime=hdr_cmn::access(pkt)->txtime();

  if(SLEEP==n->TransmissionStatus()) {
  Poweron();
  n->SetTransmissionStatus(SEND);

  cmh->next_hop()=MAC_BROADCAST;
  cmh->direction()=hdr_cmn::DOWN; 
  cmh->addr_type()=NS_AF_ILINK;
  sendDown(pkt);
  backoff_handler.clear();

  //printf("broadcast Tx sleep set timer at %f tx is %f\n",NOW,txtime);
  Scheduler& s=Scheduler::instance();
  s.schedule(&status_handler,&status_event,txtime);
  return;
  }

  if(IDLE==n->TransmissionStatus()){
  
  n->SetTransmissionStatus(SEND);
  cmh->next_hop()=MAC_BROADCAST;
  cmh->direction()=hdr_cmn::DOWN; 
  cmh->addr_type()=NS_AF_ILINK;
  //printf("broadcast : I am going to send the packet down tx is  idle%f\n",txtime);
  sendDown(pkt);
 backoff_handler.clear();
 // printf("broadcast %d Tx Idle set timer at %f tx is %f\n",node_->nodeid(),NOW,txtime);
  Scheduler& s=Scheduler::instance();
  s.schedule(&status_handler,&status_event,txtime);
  return;
  }

  if(RECV==n->TransmissionStatus())
    {
      Scheduler& s=Scheduler::instance();
      double backoff=Random::uniform()*BACKOFF;

      //  printf("broadcast Tx set timer at %f backoff is %f\n",NOW,backoff);
      s.schedule(&backoff_handler,(Event*) pkt,backoff);
      return;
    }

if (SEND==n->TransmissionStatus())
{
  // this case is supposed not to  happen 
    //  printf("broadcast: queue send data too fast\n");
    Packet::free(pkt);
      return;
}
}
   



void 
BroadcastMac::CallbackProcess(Event* callback_event)
{
 
 callback_->handle(callback_event);
 return;
}




void 
BroadcastMac::StatusProcess(Event* p)
{
  // printf("broadcast status process1\n"); 
  
 // printf("broadcast status process2\n"); 
 UnderwaterSensorNode* n=(UnderwaterSensorNode*) node_;
 // printf("broadcast status process3\n"); 
 if(SLEEP==n->TransmissionStatus()){
  Scheduler& s=Scheduler::instance();
  s.schedule(&callback_handler,&callback_event,CALLBACK_DELAY);



  // callback_->handle(&status_event);
    return;
 }
   n->SetTransmissionStatus(IDLE);
   Scheduler& s=Scheduler::instance();
  s.schedule(&callback_handler,&callback_event,CALLBACK_DELAY);
  // callback_->handle(&status_event);
  
 return;
}



int
BroadcastMac::command(int argc, const char*const* argv)
{


     if(argc == 3) {
		TclObject *obj;
                 if (strcmp(argv[1], "node_on") == 0) {
		   Node* n1=(Node*) TclObject::lookup(argv[2]);
		   if (!n1) return TCL_ERROR;
		   node_ =n1; 
		   return TCL_OK;
		 }
     }

	return UnderwaterMac::command(argc, argv);
}


