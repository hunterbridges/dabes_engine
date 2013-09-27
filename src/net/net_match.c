#include "net_match.h"

#if defined(DABES_MAC) || defined(DABES_IOS)
#include <GameCenterNetMatchProto.h>
#define NET_MATCH_PROTO GameCenterNetMatchProto
#endif

#ifndef NET_MATCH_PROTO
#define NET_MATCH_PROTO DefaultNetMatchProto
#endif

dab_uint8 NET_MATCH_MSG_NULL = 0;
dab_uint8 NET_MATCH_MSG_PLAYER_ASSIGN = 1;
dab_uint8 NET_MATCH_MSG_PLAYER_READY = 2;

NetMatchMsg *NetMatchMsg_player_assign(dab_uint8 from, dab_uint8 player) {
    NetMatchMsg *msg = calloc(1, sizeof(NetMatchMsg));
    check(msg != NULL, "Couldn't create NetMatchMsg");
    
    msg->from = from;
    msg->to = player;
    msg->kind = NET_MATCH_MSG_PLAYER_ASSIGN;
    
    return msg;
error:
    return NULL;
}

NetMatchMsg *NetMatchMsg_player_ready(dab_uint8 from) {
    NetMatchMsg *msg = calloc(1, sizeof(NetMatchMsg));
    check(msg != NULL, "Couldn't create NetMatchMsg");
    
    msg->from = from;
    msg->to = 0;
    msg->kind = NET_MATCH_MSG_PLAYER_READY;
    
    return msg;
error:
    return NULL;
}

void NetMatchMsg_destroy(NetMatchMsg *msg) {
    check(msg != NULL, "No NetMatchMsg to destroy");
    if (msg->body) {
        free(msg->body);
    }
    
    free(msg);
    return;
error:
    return;
}

////////////////////////////////////////////////////////////////////////////////

NetMatch *NetMatch_create(struct Engine *engine) {
    NetMatch *match = calloc(1, sizeof(NetMatch));
    check(match != NULL, "Couldn't create NetMatch");
    
    match->proto = NET_MATCH_PROTO;
  
    if (match->proto.init) {
        match->_(init)(match, engine);
    }
    
    return match;
error:
    return NULL;
}

void NetMatch_destroy(NetMatch *match) {
    check(match != NULL, "No NetMatch to destroy");
    
    if (match->proto.cleanup) {
        match->_(cleanup)(match);
    }
    
    free(match);
  
    return;
error:
    return;
}

////////////////////////////////////////////////////////////////////////////////

NetMatchProto DefaultNetMatchProto = {
    .init = NULL,
    .cleanup = NULL,
    .associate_native = NULL,
    .handshake = NULL,
    .all_ready_cb = NULL,
    .get_player_count = NULL,
    .get_player_number = NULL,
    .send_msg = NULL,
    .rcv_msg_cb = NULL
};
