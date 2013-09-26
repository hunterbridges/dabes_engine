#include "net_match.h"

#if defined(DABES_MAC) || defined(DABES_IOS)
#include <GameCenterNetMatchProto.h>
#define NET_MATCH_PROTO GameCenterNetMatchProto
#endif

#ifndef NET_MATCH_PROTO
#define NET_MATCH_PROTO DefaultNetMatchProto
#endif

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
    .send_data = NULL,
    .rcv_data_cb = NULL
};
