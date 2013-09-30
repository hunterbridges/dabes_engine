#ifndef __net_match_msg_h
#define __net_match_msg_h
#include "../prefix.h"

extern dab_uint8 NET_MATCH_MSG_NULL;
extern dab_uint8 NET_MATCH_MSG_PLAYER_ASSIGN;
extern dab_uint8 NET_MATCH_MSG_PLAYER_READY;
extern dab_uint8 NET_MATCH_MSG_PACKED_RECORDER;
extern dab_uint8 NET_MATCH_MSG_JSON;
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
NetMatchMsg *NetMatchMsg_null(dab_uint8 from, dab_uint8 to);

struct Recorder;
NetMatchMsg *NetMatchMsg_packed_recorder(dab_uint8 from, dab_uint8 to,
        struct Recorder *recorder);

NetMatchMsg *NetMatchMsg_json(dab_uint8 from, dab_uint8 to, const char *json);

void NetMatchMsg_destroy(NetMatchMsg *msg);

#endif
