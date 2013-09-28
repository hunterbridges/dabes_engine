#ifndef __net_match_h
#define __net_match_h
#include "../prefix.h"

extern dab_uint8 NET_MATCH_MSG_NULL;
extern dab_uint8 NET_MATCH_MSG_PLAYER_ASSIGN;
extern dab_uint8 NET_MATCH_MSG_PLAYER_READY;
typedef dab_uint8 NetMatchMsgType;

typedef struct NetMatchMsg {
    dab_uint8 from;
    dab_uint8 to;
    NetMatchMsgType kind;
    size_t size;
    dab_uchar *body;
} NetMatchMsg;

NetMatchMsg *NetMatchMsg_player_assign(dab_uint8 from, dab_uint8 player);
NetMatchMsg *NetMatchMsg_player_ready(dab_uint8 from);
void NetMatchMsg_destroy(NetMatchMsg *msg);

////////////////////////////////////////////////////////////////////////////////

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
