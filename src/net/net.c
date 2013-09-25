#include "net.h"

#if defined(DABES_MAC) || defined(DABES_IOS)
#include <GameCenterNetProto.h>
#define NET_PROTO GameCenterNetProto
#endif

#ifndef NET_PROTO
#define NET_PROTO DefaultNetProto
#endif

Net *Net_create(struct Engine *engine) {
    Net *net = calloc(1, sizeof(Net));
    check(net != NULL, "Couldn't create Net");
    
    net->proto = NET_PROTO;
    
    if (net->proto.init) {
        net->_(init)(net, engine);
    }
  
    return net;
error:
    return NULL;
}

void Net_destroy(Net *net) {
    check(net != NULL, "No Net to destroy");
    
    if (net->proto.cleanup) {
        net->_(cleanup)(net);
    }
    
    free(net);
  
    return;
error:
    return;
}

////////////////////////////////////////////////////////////////////////////////

NetProto DefaultNetProto = {
    .init = NULL,
    .cleanup = NULL,
    .authenticate = NULL,
    .authenticate_cb = NULL
};
