#ifndef __net_match_h
#define __net_match_h
#include "../prefix.h"

struct Engine;
struct NetMatch;
typedef struct NetMatchProto {
    int (*init)(struct NetMatch *net, struct Engine *engine);
    int (*cleanup)(struct NetMatch *net);
    int (*send_data)(struct NetMatch *match, struct Engine *engine,
                     unsigned char *data, size_t size);
    int (*rcv_data_cb)(struct NetMatch *match, struct Engine *engine,
                       unsigned char *data, size_t size);
} NetMatchProto;

typedef struct NetMatch {
    NetMatchProto proto;
    void *context;
} NetMatch;

NetMatch *NetMatch_create(struct Engine *engine);
void NetMatch_destroy(NetMatch *match);

extern NetMatchProto DefaultNetMatchProto;

#endif
