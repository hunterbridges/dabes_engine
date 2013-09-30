#ifndef __net_match_h
#define __net_match_h
#include "net_match_msg.h"

struct Engine;
struct NetMatch;
typedef struct NetMatchProto {
    int (*init)(struct NetMatch *net, struct Engine *engine);
    int (*cleanup)(struct NetMatch *net);
    int (*associate_native)(struct NetMatch *net, void *assoc);
    int (*handshake)(struct NetMatch *net, struct Engine *engine);
    int (*all_ready_cb)(struct NetMatch *net, struct Engine *engine);
    int (*get_metadata)(struct NetMatch *net, struct Engine *engine);
    int (*got_metadata_cb)(struct NetMatch *net, struct Engine *engine,
                           void *metadata);
    int (*get_player_count)(struct NetMatch *net, struct Engine *engine);
    int (*get_player_number)(struct NetMatch *net, struct Engine *engine);
    int (*send_msg)(struct NetMatch *match, struct Engine *engine,
                    NetMatchMsg *msg);
    int (*rcv_msg_cb)(struct NetMatch *match, struct Engine *engine,
                      NetMatchMsg *msg);
} NetMatchProto;

typedef struct NetMatch {
    NetMatchProto proto;
    void *context;
} NetMatch;

NetMatch *NetMatch_create(struct Engine *engine);
void NetMatch_destroy(NetMatch *match);

extern NetMatchProto DefaultNetMatchProto;

#endif
