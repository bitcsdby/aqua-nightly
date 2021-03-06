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
//  Copyright (C) 1998 by Mingzhou Sun. All rights reserved.
//  This software is developed at Rensselaer Polytechnic Institute under 
//  DARPA grant No. F30602-97-C-0274
//  Redistribution and use in source and binary forms are permitted
//  provided that the above copyright notice and this paragraph are
//  duplicated in all such forms and that any documentation, advertising
//  materials, and other materials related to such distribution and use
//  acknowledge that the software was developed by Mingzhou Sun at the
//  Rensselaer  Polytechnic Institute.  The name of the University may not 
//  be used to endorse or promote products derived from this software 
//  without specific prior written permission.
//
// $Header: /nfs/jade/vint/CVSROOT/ns-2/diffusion/diff_header.h,v 1.3 2001/11/08 18:16:32 haldar Exp $

/******************************************************/
/* diff_header.h : Chalermek Intanagonwiwat  08/16/99 */
/******************************************************/

#ifndef ns_uwvb_header_h
#define ns_uwvb_header_h

//#include "ip.h"

#include "config.h"
#include "packet.h"

#define INTEREST      1
#define DATA          2
#define DATA_READY    3
#define SOURCE_DISCOVERY 4
#define SOURCE_TIMEOUT   5
#define TARGET_DISCOVERY 6
#define TARGET_REQUEST 7
#define SOURCE_DENY  8

//#define DATA_READY    3 // not used yet
//#define DATA_REQUEST  4 // not used yet
//#define POS_REINFORCE 5 //not used yet
//#define NEG_REINFORCE 6 //not used yet
//#define INHIBIT       7 //not used yet
//#define TX_FAILED     8 //not used yet
//#define DATA_STOP     9 

#define MAX_ATTRIBUTE 3
#define MAX_NEIGHBORS 30
#define MAX_DATA_TYPE 30
#define MAX_NEIGHBOR 10
#define WINDOW_SIZE  19 
//used by hash table to limited the maximum length



#define ROUTING_PORT 255

//#define ORIGINAL    100        
//#define SUB_SAMPLED 1         

// For positive reinforcement in simple mode. 
// And for TX_FAILED of backtracking mode.


struct position{
  double x;
  double y;
  double z;
};

struct neighborhood{
  int number;
  position neighbor[MAX_NEIGHBOR];
};


struct uw_extra_info {

  // ns_addr_t osender_id;            // The original sender of this message
  // unsigned int seq;           //  sequence number

  double ox;  // the original sender's position
  double oy;
  double oz;

  //ns_addr_t sender_id;            // The forwarder of this message

  double fx;  // the original sender's position
  double fy;
  double fz;

  double tx;
  double ty;
  double tz;

// this is the information about relative position of the receiver to the forwarder
  double dx;
  double dy;
  double dz; 

};


#endif
