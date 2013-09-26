#ifndef __net_h
#define __net_h
#include "../prefix.h"

struct Net;
struct Engine;
typedef struct NetProto {
    int (*init)(struct Net *net, struct Engine *engine);
    int (*cleanup)(struct Net *net);
    int (*authenticate)(struct Net *net, struct Engine *engine);
    int (*authenticate_cb)(struct Net *net, struct Engine *engine);
    int (*find_matches)(struct Net *net, struct Engine *engine);
    int (*find_matches_cb)(struct Net *net, struct Engine *engine, void *assoc);
} NetProto;

typedef struct Net {
    NetProto proto;
    void *context;
} Net;

Net *Net_create(struct Engine *engine);
void Net_destroy(Net *net);

extern NetProto DefaultNetProto;

#endif
