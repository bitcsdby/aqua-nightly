// Copyright (c) 2000 by the University of Southern California
// All rights reserved.
//
// Permission to use, copy, modify, and distribute this software and its
// documentation in source and binary forms for non-commercial purposes
// and without fee is hereby granted, provided that the above copyright
// notice appear in all copies and that both the copyright notice and
// this permission notice appear in supporting documentation. and that
// any documentation, advertising materials, and other materials related
// to such distribution and use acknowledge that the software was
// developed by the University of Southern California, Information
// Sciences Institute.  The name of the University may not be used to
// endorse or promote products derived from this software without
// specific prior written permission.
//
// THE UNIVERSITY OF SOUTHERN CALIFORNIA makes no representations about
// the suitability of this software for any purpose.  THIS SOFTWARE IS
// PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Other copyrights might apply to parts of this software and are so
// noted when applicable.
//

/****************************************************************/
/* flooding.cc : Chalermek Intanagonwiwat (USC/ISI)  05/18/99   */
/****************************************************************/

// Share api with diffusion and omnicient multicast
// Using diffusion packet header 

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <signal.h>
#include <float.h>
#include <stdlib.h>

#include <tcl.h>


#include "underwatersensor/uw_common/uwvb_header.h"
#include "agent.h"
#include "tclcl.h"
#include "ip.h"
#include "config.h"
#include "packet.h"
#include "trace.h"
#include "random.h"
#include "classifier.h"
#include "node.h"
#include "vectorbasedforward.h"
//#include "iflist.h"
#include "underwatersensor/uw_common/uw_hash_table.h"
#include "arp.h"
#include "mac.h"
#include "ll.h"
#include "dsr/path.h"
#include "god.h"
#include  "underwatersensor/uw_mac/cubepropagation.h"




void UWDelayTimer:: expire(Event* e)
{
  a_->timeout(packet);
}


static class VectorbasedforwardClass : public TclClass {
public:
  VectorbasedforwardClass() : TclClass("Agent/Vectorbasedforward") {}
  TclObject* create(int argc, const char*const* argv) {
    return(new VectorbasedforwardAgent());
  }
} class_vectorbasedforward;



VectorbasedforwardAgent::VectorbasedforwardAgent() : Agent(PT_UWVB),
delaytimer(this)
{
  // Initialize variables.
  //  printf("VB initialized\n");
  pk_count = 0;
  target_ = 0;
   node = NULL;
  tracetarget = NULL;
  width=0;
  counter=0;
  priority=1.5;
 bind("width",& width);
  
}


void VectorbasedforwardAgent::recv(Packet* packet, Handler*)
{
   
 
  if (node->failure_status()==1){
     
  printf ("vectorbasedforward%d: I fails!!!!\n ",here_.addr_);
  return;
  }
  

  hdr_uwvb* vbh = HDR_UWVB(packet);
  unsigned char msg_type =vbh->mess_type;
  unsigned int dtype =0;// vbh->data_type;
  double t1=vbh->ts_;
  uw_extra_info * info=new uw_extra_info;
  
  memcpy((unsigned char*)info,packet->accessdata(),sizeof(uw_extra_info));

   position* p1=new position[1];
  p1[0].x=info->fx;
  p1[0].y=info->fy;
  p1[0].z=info->fz;    
  delete info;

  if(NOW-t1>=50)
    {
      printf("vectorbased: ops! too old %f %f!!!!!!!\n",t1, NOW);
    Packet::free(packet);  
    return;
    }
  
    neighborhood *hashPtr= PktTable.GetHash(vbh->sender_id, vbh->pk_num);

     // Received this packet before ?

    if (hashPtr != NULL) {       
    PktTable.put_in_hash(vbh,p1);
     Packet::free(packet);
     // printf("vectrobasedforward: this is duplicate packet\n"); 
                      }
      else {
 
     PktTable.put_in_hash(vbh,p1);
 
     // Take action for a new pkt.
      
     ConsiderNew(packet);     
      }
    delete p1;
}

void VectorbasedforwardAgent::ConsiderNew(Packet *pkt)
{
  hdr_uwvb* vbh = HDR_UWVB(pkt);
  unsigned char msg_type =vbh->mess_type;
  unsigned int dtype =0;// vbh->data_type; 
 
  double l,h;
  
  //Pkt_Hash_Entry *hashPtr;
   neighborhood * hashPtr;
  //  Agent_List *agentPtr;
  // PrvCurPtr  RetVal;
   ns_addr_t   from_nodeID, forward_nodeID, target_nodeID;

  Packet *gen_pkt;
  hdr_uwvb *gen_vbh;
  position * p1;
   uw_extra_info * info=new uw_extra_info;
   memcpy((unsigned char*) info,pkt->accessdata(),sizeof(uw_extra_info));
  
  p1=new position[1];
  p1[0].x=info->fx;
  p1[0].y=info->fy;
  p1[0].z=info->fz;    

  printf ("vectorbasedforward:(id :%d) forward:(%d ,%d) sender is(%d,%d,%d), (%f,%f,%f) the relative position is (%f ,%f,%f) forward position is is (%f,%f,%f) at time %f type is %d real one is %d\n",here_.addr_, vbh->forward_agent_id.addr_, vbh->forward_agent_id.port_,vbh->sender_id.addr_,vbh->sender_id.port_,vbh->pk_num,node->X(),node->Y(),node->Z(),info->dx,info->dy,info->dz,info->fx,info->fy,info->fz,NOW,vbh->mess_type,DATA);
 
  
  switch (msg_type) {
    case INTEREST : 
      // printf("Vectorbasedforward:it is interest packet!\n");
      hashPtr = PktTable.GetHash(vbh->sender_id, vbh->pk_num);

      // Check if it comes from sink agent of  this node
      // If so we have to keep it in sink list 

      from_nodeID = vbh->sender_id;
      forward_nodeID = vbh->forward_agent_id;
      //  printf("Vectorbasedforward:it the from_nodeid is %d %d  and theb this node id is %d ,%d!\n", from_nodeID.addr_,from_nodeID.port_,THIS_NODE.addr_,THIS_NODE.port_ );

      if (THIS_NODE.addr_ == from_nodeID.addr_) {       
   
      MACprepare(pkt);
      MACsend(pkt,Random::uniform()*JITTER); 
      printf("vectorbasedforward: after MACprepare(pkt)\n");
      }
      else
       {
          calculatePosition(pkt);
	 //printf("vectorbasedforward: This packet is from different node\n");
	 if (IsTarget(pkt)) 
           { 
            // If this node is target?
    	      l=advance(pkt);
        
	   //  send_to_demux(pkt,0);
	      //  printf("vectorbasedforward:%d send out the source-discovery \n",here_.addr_);
	     vbh->mess_type=SOURCE_DISCOVERY;
	     set_delaytimer(pkt,l*JITTER);
                 // !!! need to re-think
	   }
	 else{ 
	   // calculatePosition(pkt);
	   // No the target forwared
          l=advance(pkt);
          h=projection(pkt);
        if (IsCloseEnough(pkt)){
	  // printf("vectorbasedforward:%d I am close enough for the interest\n",here_.addr_);
      MACprepare(pkt);
      MACsend(pkt,Random::uniform()*JITTER);//!!!! need to re-think
	}
	else { 
	  //  printf("vectorbasedforward:%d I am not close enough for the interest  \n",here_.addr_);
         Packet::free(pkt);}
	 }
       }
      // Packet::free(pkt); 
      return;


 

  case TARGET_DISCOVERY: 
// from other nodes hitted by the packet, it is supposed
// to be the one hop away from the sink 

// printf("Vectorbasedforward(%d,%d):it is target-discovery  packet(%d)! it target id is %d  coordinate is %f,%f,%f and range is %f\n",here_.addr_,here_.port_,vbh->pk_num,vbh->target_id.addr_,vbh->info.tx, vbh->info.ty,vbh->info.tz,vbh->range);    
    if (THIS_NODE.addr_==vbh->target_id.addr_) {
      //printf("Vectorbasedforward(%d,??%d):it is target-discovery  packet(%d)! it target id is %d  coordinate is %f,%f,%f and range is %f\n",here_.addr_,here_.port_,vbh->pk_num,vbh->target_id.addr_,vbh->info.tx, vbh->info.ty,vbh->info.tz,vbh->range);    
      // ns_addr_t *hashPtr= PktTable.GetHash(vbh->sender_id, vbh->pk_num);
     // Received this packet before ?
      // if (hashPtr == NULL) { 

       calculatePosition(pkt);
       DataForSink(pkt);
	 printf("Vectorbasedforward: %d is the target\n", here_.addr_);
       // } //New data Process this data 
       // 
    } else  {Packet::free(pkt);}
   return;

  case SOURCE_DISCOVERY:
      Packet::free(pkt); 
// other nodes already claim to be the source of this interest
    //   SourceTable.put_in_hash(vbh);
    return;


 case DATA_READY :
   //  printf("Vectorbasedforward(%d,%d):it is data ready packet(%d)! it target id is %d \n",here_.addr_,here_.port_,vbh->pk_num,vbh->target_id.addr_);    
      from_nodeID = vbh->sender_id;
      if (THIS_NODE.addr_ == from_nodeID.addr_) {       
	// come from the same node, broadcast it
      MACprepare(pkt);
      MACsend(pkt,Random::uniform()*JITTER); 
      return;      
          }
          calculatePosition(pkt);
      if (THIS_NODE.addr_==vbh->target_id.addr_)
               {
        printf("Vectorbasedforward: %d is the target\n", here_.addr_);
	      DataForSink(pkt); // process it
	       } 
	else{
	  // printf("Vectorbasedforward: %d is the not  target\n", here_.addr_); 
      MACprepare(pkt);
      MACsend(pkt, Random::uniform()*JITTER);
	}
      return;

    /*
  case DATA_READY :

      // put source_agent in source list of routing table   
      agentPtr = new Agent_List;
      AGT_ADDR(agentPtr) = vbh->sender_id;
      agentPtr->next = routing_table[dtype].source;
      routing_table[dtype].source = agentPtr;
    
       // !! this part will be modified later
      God::instance()->AddSource(dtype, (vbh->sender_id).addr_);
	
      gen_pkt = prepare_message(dtype, vbh->sender_id, DATA_REQUEST);
      gen_vbh = HDR_UWVB(gen_pkt);
      //      gen_vbh->report_rate = ORIGINAL;
      send_to_dmux(gen_pkt, 0);
      Packet::free(pkt);
      return;
    */
 
    case DATA :
     printf("Vectorbasedforward(%d,%d):it is data packet(%d)! it target id is %d  coordinate is %f,%f,%f and range is %f\n",here_.addr_,here_.port_,vbh->pk_num,vbh->target_id.addr_,info->tx, info->ty,info->tz,vbh->range);  

   printf("Vectorbasedforward(%d) the traget address is %d\n",THIS_NODE.addr_,vbh->sender_id.addr_);   
  
      from_nodeID = vbh->sender_id;
      if (THIS_NODE.addr_ == from_nodeID.addr_) {       
	// come from the same node, broadcast it
      MACprepare(pkt);
      MACsend(pkt,0); 
      return;      
}
         	    calculatePosition(pkt);
		    //  printf("vectorbasedforward: after MACprepare(pkt)\n");
          l=advance(pkt);
         h=projection(pkt);
	

	 if (THIS_NODE.addr_==vbh->target_id.addr_)
               {
	 printf("Vectorbasedforward: %d is the target\n", here_.addr_);
	      DataForSink(pkt); // process it
	       }

	else{
       printf("Vectorbasedforward: %d is the not  target\n", here_.addr_);
	  if (IsCloseEnough(pkt)){
	      double delay=calculateDelay(pkt,p1);
              double d2=(TRANSMISSION_DISTANCE-distance(pkt))/SPEED_OF_LIGHT;
	      set_delaytimer(pkt,(sqrt(delay)*DELAY+d2*2));
	    
	  }
	  else { Packet::free(pkt);   }
	}
      return;

    default : 
      
      Packet::free(pkt);        
      break;
  }
  delete info;
  delete p1;
}


void VectorbasedforwardAgent::reset()
{
  PktTable.reset();
  /*
  for (int i=0; i<MAX_DATA_TYPE; i++) {
    routing_table[i].reset();
  }
  */
}


void VectorbasedforwardAgent::Terminate() 
{
#ifdef DEBUG_OUTPUT
	printf("node %d: remaining energy %f, initial energy %f\n", THIS_NODE, 
	       node->energy_model()->energy(), 
	       node->energy_model()->initialenergy() );
#endif
}


void VectorbasedforwardAgent::StopSource()
{

}


Packet * VectorbasedforwardAgent:: create_packet()
{
  Packet *pkt = Packet::alloc(sizeof(uw_extra_info));

  if (pkt==NULL) return NULL;

  hdr_cmn*  cmh = HDR_CMN(pkt);
  cmh->size() = 36;

  hdr_uwvb* vbh = HDR_UWVB(pkt);
  vbh->ts_ = NOW;
   
  //!! I add new part

  uw_extra_info* info=new uw_extra_info;
 
  info->ox=node->CX();
  info->oy=node->CY(); 
  info->oz=node->CZ(); 
  info->fx=node->CX(); 
  info->fy=node->CY();
  info->fz=node->CZ();
 memcpy(pkt->accessdata(),info,sizeof(uw_extra_info));
 delete info;

  return pkt;
}


Packet *VectorbasedforwardAgent::prepare_message(unsigned int dtype, ns_addr_t to_addr,  int msg_type)

{
  Packet *pkt;
  hdr_uwvb *vbh;
  //hdr_ip *iph;

    pkt = create_packet();
    vbh = HDR_UWVB(pkt);
    // iph = HDR_IP(pkt);
    
    vbh->mess_type = msg_type;
    vbh->pk_num = pk_count;
    pk_count++;
    vbh->sender_id = here_;
    //   vbh->data_type = dtype;
    vbh->forward_agent_id = here_;

    vbh->ts_ = NOW;
    //    vbh->num_next = 1;
    // I am not sure if we need this
    // vbh->next_nodes[0] = to_addr.addr_;


    // I am not sure if we need it?    
    /*
    iph->src_ = here_;
    iph->dst_ = to_addr;
    */
    return pkt;
}

void VectorbasedforwardAgent::MACprepare(Packet *pkt)
{

  hdr_uwvb* vbh = HDR_UWVB(pkt);
  hdr_cmn* cmh = HDR_CMN(pkt);
  //  hdr_ip*  iph = HDR_IP(pkt); // I am not sure if we need it


  vbh->forward_agent_id = here_; 
 
  cmh->xmit_failure_ = 0;
  // printf("vectorbased: the mac_Broadcast is:%d\n",MAC_BROADCAST);
  cmh->next_hop() = MAC_BROADCAST; 
  cmh->addr_type() = NS_AF_ILINK;  
  // cmh->txtime()=0;
 
  cmh->direction() = hdr_cmn::DOWN;


  uw_extra_info* info=new uw_extra_info;

  if(!node->sinkStatus()){       //!! I add new part
  info->fx=node->CX();
  info->fy=node->CY();
  info->fz=node->CZ();
  }
  else{
   info->fx=node->X();
   info->fy=node->Y();
   info->fz=node->Z();
}

 memcpy(pkt->accessdata(),info,sizeof(uw_extra_info));
 delete info;

}


void VectorbasedforwardAgent::MACsend(Packet *pkt, Time delay)
{
  hdr_cmn*  cmh = HDR_CMN(pkt);
  hdr_uwvb* vbh = HDR_UWVB(pkt);

  // I don't understand why it works like this way
  /*
  if (vbh->mess_type == DATA)
    cmh->size() = (God::instance()->data_pkt_size) + 4*(vbh->num_next - 1);
  else
    cmh->size() = 36 + 4*(vbh->num_next -1);
  */
 
 if (vbh->mess_type == DATA)
   // cmh->size() = (God::instance()->data_pkt_size)+12 ;
  cmh->size() = 65+12 ;
  else
    cmh->size() =32;
  
 //if(!ll) printf("ok, the LL is empty\n");
 //cmh->ptype_==PT_UWVB;
 //printf("vectorbased: the address type is :%d uid is %d\n", cmh->addr_type(),pkt->uid_);
  printf("vectorbased node %d: the packet type is :%d\n", here_.addr_,vbh->mess_type);
// ll->handle(pkt);
  Scheduler::instance().schedule(ll, pkt, delay);
}


void VectorbasedforwardAgent::DataForSink(Packet *pkt)
{


  
  //  printf("DataforSink: the packet is send to demux\n");
      send_to_dmux(pkt, 0);

}



void VectorbasedforwardAgent::trace (char *fmt,...)
{
  va_list ap;

  if (!tracetarget)
    return;

  va_start (ap, fmt);
  vsprintf (tracetarget->pt_->buffer(), fmt, ap);
  tracetarget->pt_->dump ();
  va_end (ap);
}

void VectorbasedforwardAgent::set_delaytimer(Packet* pkt, double c){
 delaytimer.packet=pkt; 
 delaytimer.resched(c);
}

void VectorbasedforwardAgent::timeout(Packet * pkt){

 hdr_uwvb* vbh = HDR_UWVB(pkt);
 unsigned char msg_type =vbh->mess_type;
 neighborhood  *hashPtr;

   uw_extra_info * info=new uw_extra_info;
   memcpy((unsigned char*) info,pkt->accessdata(),sizeof(uw_extra_info));



 
 switch (msg_type){
  
 case DATA:
       hashPtr= PktTable.GetHash(vbh->sender_id, vbh->pk_num);
	if (hashPtr != NULL) {
          int num_neighbor=hashPtr->number;          
	  // printf("vectorbasedforward: node %d have received %d when wake up at %f\n",here_.addr_,num_neighbor,NOW);
	  if (num_neighbor!=1){
      // Some guys transmit the data before me
	    if (num_neighbor==MAX_NEIGHBOR) {
             //I have too many neighbors, I quit
                  Packet::free(pkt);
		  return;  
	    }
	    else //I need to calculate my delay time again 
             {  
	       int i=0;
	       position* tp;
               tp=new position[1];

	      	 tp[0].x=hashPtr->neighbor[i].x;   
	         tp[0].y=hashPtr->neighbor[i].y;   
        	 tp[0].z=hashPtr->neighbor[i].z; 
                double tdelay=calculateDelay(pkt,tp);
                // double tdelay=5;
		i++;
                double c=1;
	         while (i<num_neighbor){
		   c=c*2;
		   tp[0].x=hashPtr->neighbor[i].x;   
		   tp[0].y=hashPtr->neighbor[i].y;   
		   tp[0].z=hashPtr->neighbor[i].z;   
		 double t2delay=calculateDelay(pkt,tp);
		 if (t2delay>tdelay) tdelay=t2delay;
		 	 i++; 
		  }
 
	       delete tp;
               if(tdelay<=(priority/c)) { 
               MACprepare(pkt);
               MACsend(pkt,0);      
		 }
               else
	   Packet::free(pkt);//to much overlap, don;t send 
	     }// end of calculate my new delay time 
	  }

 else{// I am the only neighbor
	       position* tp;
               tp=new position[1];
		 tp[0].x=info->fx;   
	         tp[0].y=info->fy;   
        	 tp[0].z=info->fz;   
		 double delay=calculateDelay(pkt,tp);
                 delete info;
                 delete tp;
		 if (delay<=priority){
		   // printf("vectorbasedforward: !!%f\n",delay);
                    MACprepare(pkt);
                    MACsend(pkt,0);      
		              }
		 else  Packet::free(pkt);     
	 
	return;
 }
	}
	break; 
 default:
    Packet::free(pkt);  
   break;
 }
}



void VectorbasedforwardAgent::calculatePosition(Packet* pkt)
{
 
 hdr_uwvb     *vbh  = HDR_UWVB(pkt); 
 uw_extra_info* info=new uw_extra_info;

 memcpy((unsigned char*) info,pkt->accessdata(),sizeof(uw_extra_info));

 double fx=info->fx;
 double fy=info->fy;
 double fz=info->fz;

 double dx=info->dx;
 double dy=info->dy;
 double dz=info->dz;

 node->CX_=fx+dx;
 node->CY_=fy+dy;
 node->CZ_=fz+dz;
 // printf("vectorbased: my position is computed as (%f,%f,%f)\n",node->CX_, node->CY_,node->CZ_);
 delete info;
}

double VectorbasedforwardAgent::calculateDelay(Packet* pkt,position* p1)
{
 
 hdr_uwvb     *vbh  = HDR_UWVB(pkt); 
 uw_extra_info* info=new uw_extra_info;

 memcpy((unsigned char*) info,pkt->accessdata(),sizeof(uw_extra_info));

 double fx=p1->x;
 double fy=p1->y;
 double fz=p1->z;

 double dx=node->CX_-fx; 
 double dy=node->CY_-fy;
 double dz=node->CZ_-fz;

  
 double tx=info->tx;
 double ty=info->ty;
 double tz=info->tz; 

 double dtx=tx-fx;
 double dty=ty-fy;
 double dtz=tz-fz;  

 double dp=dx*dtx+dy*dty+dz*dtz;

 // double a=advance(pkt);
 double p=projection(pkt);
 double d=sqrt((dx*dx)+(dy*dy)+ (dz*dz));
 double l=sqrt((dtx*dtx)+(dty*dty)+ (dtz*dtz));
 double cos_theta=dp/(d*l);
 // double delay=(TRANSMISSION_DISTANCE-d*cos_theta)/TRANSMISSION_DISTANCE;
   double delay=(p/width) +((TRANSMISSION_DISTANCE-d*cos_theta)/TRANSMISSION_DISTANCE);
 // double delay=(p/width) +((TRANSMISSION_DISTANCE-d)/TRANSMISSION_DISTANCE)+(1-cos_theta);
  //printf("vectorbased: node(%d) projection is %f, and cos is %f, and d is %f)\n",here_.addr_,p, cos_theta, d);
   return delay;
}


double VectorbasedforwardAgent::distance(Packet* pkt)
{
 
 hdr_uwvb     *vbh  = HDR_UWVB(pkt); 
 uw_extra_info* info=new uw_extra_info;
 memcpy((unsigned char*) info,pkt->accessdata(),sizeof(uw_extra_info));


 double tx=info->fx;
 double ty=info->fy;
 double tz=info->fz;
 // printf("vectorbased: the target is %lf,%lf,%lf \n",tx,ty,tz);
 double x=node->CX(); //change later
 double y=node->CY();// printf(" Vectorbasedforward: I am in advanced\n");
 double z=node->CZ();
 // printf("the target is %lf,%lf,%lf and my coordinates are %lf,%lf,%lf\n",tx,ty,tz,x,y,z);

 delete info;
 return sqrt((tx-x)*(tx-x)+(ty-y)*(ty-y)+ (tz-z)*(tz-z));
}


double VectorbasedforwardAgent::advance(Packet* pkt)
{
 
 hdr_uwvb     *vbh  = HDR_UWVB(pkt); 
 uw_extra_info* info=new uw_extra_info;
 memcpy((unsigned char*) info,pkt->accessdata(),sizeof(uw_extra_info));

 double tx=info->tx;
 double ty=info->ty;
 double tz=info->tz;
 // printf("vectorbased: the target is %lf,%lf,%lf \n",tx,ty,tz);
 double x=node->CX(); //change later
 double y=node->CY();// printf(" Vectorbasedforward: I am in advanced\n");
 double z=node->CZ();
 // printf("the target is %lf,%lf,%lf and my coordinates are %lf,%lf,%lf\n",tx,ty,tz,x,y,z);

 delete info;
 return sqrt((tx-x)*(tx-x)+(ty-y)*(ty-y)+ (tz-z)*(tz-z));
}

double VectorbasedforwardAgent::projection(Packet* pkt)
{
 hdr_uwvb     *vbh  = HDR_UWVB(pkt);
 uw_extra_info* info=new uw_extra_info;
 memcpy((unsigned char*) info,pkt->accessdata(),sizeof(uw_extra_info)); 


 double tx=info->tx;
 double ty=info->ty;
 double tz=info->tz;
 

 double ox=info->ox;
 double oy=info->oy;
 double oz=info->oz;

 delete info;

 double x=node->CX();
 double y=node->CY();
 double z=node->CZ();
 
 double wx=tx-ox;
 double wy=ty-oy;
 double wz=tz-oz;

 double vx=x-ox;
 double vy=y-oy;
 double vz=z-oz;

 double cross_product_x=vy*wz-vz*wy;
 double cross_product_y=vz*wx-vx*wz;
 double cross_product_z=vx*wy-vy*wx;
  
 double area=sqrt(cross_product_x*cross_product_x+ 
          cross_product_y*cross_product_y+cross_product_z*cross_product_z);
 double length=sqrt((tx-ox)*(tx-ox)+(ty-oy)*(ty-oy)+ (tz-oz)*(tz-oz));
 // printf("vectorbasedforward: the area is %f and length is %f\n",area,length);
 return area/length;
}

bool VectorbasedforwardAgent::IsTarget(Packet* pkt)
{
  hdr_uwvb * vbh=HDR_UWVB(pkt);

  if (vbh->target_id.addr_==0){

  //  printf("vectorbased: advanced is %lf and my range is %f\n",advance(pkt),vbh->range);
    return (advance(pkt)<vbh->range);
}
  else return(THIS_NODE.addr_==vbh->target_id.addr_);


}



bool VectorbasedforwardAgent::IsCloseEnough(Packet* pkt)
{
  hdr_uwvb     *vbh  = HDR_UWVB(pkt);
  double range=vbh->range;
  //double range=width;

  //  printf("underwatersensor: The width is %f\n",range);

  uw_extra_info* info=new uw_extra_info;
 memcpy((unsigned char*) info,pkt->accessdata(),sizeof(uw_extra_info));

 double ox=info->ox;
 double oy=info->oy;
 double oz=info->oz;

 double tx=info->tx;
 double ty=info->ty;
 double tz=info->tz;

 double fx=info->fx;
 double fy=info->fy;
 double fz=info->fz;

 delete info;

 double x=node->CX(); //change later
 double y=node->CY();
 double z=node->CZ();

 double d=sqrt((tx-fx)*(tx-fx)+(ty-fy)*(ty-fy)+(tz-fz)*(tz-fz));
 //double l=sqrt((tx-ox)*(tx-ox)+(ty-oy)*(ty-oy)+ (tz-oz)*(tz-oz));
 double l=advance(pkt);
 // if (l<range)
 // {
     // printf("vectorbasedforward: IsClose?too close! it should be target!\n");
     // return true;
 // }
 // else {
   //double c=d/l;
   double c=1;
   // if ((d<=range)&&((z-oz)<0.01))  return true;
 if ((projection(pkt)<=(c*width)))  return true;
 return false;

}





int VectorbasedforwardAgent::command(int argc, const char*const* argv)
{  
  Tcl& tcl =  Tcl::instance();

  if (argc == 2) {

    if (strcasecmp(argv[1], "reset-state")==0) {
      
      reset();
      return TCL_OK;
    }

    if (strcasecmp(argv[1], "reset")==0) {
      
      return Agent::command(argc, argv);
    }

    if (strcasecmp(argv[1], "start")==0) {
      return TCL_OK;
    }

    if (strcasecmp(argv[1], "stop")==0) {
      return TCL_OK;
    }

    if (strcasecmp(argv[1], "terminate")==0) {
      Terminate();
      return TCL_OK;
    }

   if (strcasecmp(argv[1], "name")==0) {
     printf("vectorbased \n");
      return TCL_OK;
    }
    if (strcasecmp(argv[1], "stop-source")==0) {
      StopSource();
      return TCL_OK;
    }

  } else if (argc == 3) {

    if (strcasecmp(argv[1], "on-node")==0) {
      //   printf ("inside on node\n");
      node = (UnderwaterSensorNode *)tcl.lookup(argv[2]);
      return TCL_OK;
    }
    /*
      if (strcasecmp(argv[1], "set-port")==0) {
      printf ("inside on node\n");
      port_number=atoi(argv[2]);
      return TCL_OK;
    }
    */
    if (strcasecmp(argv[1], "add-ll") == 0) {

      TclObject *obj;

      if ( (obj = TclObject::lookup(argv[2])) == 0) {
    fprintf(stderr, "Vectorbasedforwarding Node: %d lookup of %s failed\n", THIS_NODE.addr_, argv[2]);
	return TCL_ERROR;
      }
      ll = (NsObject *) obj;

     return TCL_OK;
    }

    if (strcasecmp (argv[1], "tracetarget") == 0) {
      TclObject *obj;
      if ((obj = TclObject::lookup (argv[2])) == 0) {
	  fprintf (stderr, "%s: %s lookup of %s failed\n", __FILE__, argv[1],
		   argv[2]);
	  return TCL_ERROR;
      }

      tracetarget = (Trace *) obj;
      return TCL_OK;
    }

    if (strcasecmp(argv[1], "port-dmux") == 0) {
      // printf("vectorbasedforward:port demux is called \n");
      TclObject *obj;

      if ( (obj = TclObject::lookup(argv[2])) == 0) {
	fprintf(stderr, "VB node Node: %d lookup of %s failed\n", THIS_NODE.addr_, argv[2]);
	return TCL_ERROR;
      }
      port_dmux = (NsObject *) obj;
      return TCL_OK;
    }

  } 

  return Agent::command(argc, argv);
}


