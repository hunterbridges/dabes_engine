#include "net_match_msg.h"
#include "../recorder/recorder.h"

dab_uint8 NET_MATCH_MSG_NULL = 0;
dab_uint8 NET_MATCH_MSG_PLAYER_ASSIGN = 1;
dab_uint8 NET_MATCH_MSG_PLAYER_READY = 2;
dab_uint8 NET_MATCH_MSG_PACKED_RECORDER = 3;
dab_uint8 NET_MATCH_MSG_JSON = 4;

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

NetMatchMsg *NetMatchMsg_null(dab_uint8 from, dab_uint8 to) {
    NetMatchMsg *msg = calloc(1, sizeof(NetMatchMsg));
    check(msg != NULL, "Couldn't create NetMatchMsg");

    msg->from = from;
    msg->to = to;
    msg->kind = NET_MATCH_MSG_NULL;

    return msg;
error:
    return NULL;
}

NetMatchMsg *NetMatchMsg_packed_recorder(dab_uint8 from, dab_uint8 to,
        struct Recorder *recorder) {
    NetMatchMsg *msg = calloc(1, sizeof(NetMatchMsg));
    check(msg != NULL, "Couldn't create NetMatchMsg");

    msg->from = from;
    msg->to = to;
    msg->kind = NET_MATCH_MSG_PACKED_RECORDER;
    recorder->_(pack)(recorder, &msg->body, &msg->size);

    return msg;
error:
    return NULL;
}

NetMatchMsg *NetMatchMsg_json(dab_uint8 from, dab_uint8 to, const char *json) {
    NetMatchMsg *msg = calloc(1, sizeof(NetMatchMsg));
    check(msg != NULL, "Couldn't create NetMatchMsg");

    msg->from = from;
    msg->to = to;
    msg->kind = NET_MATCH_MSG_JSON;

    msg->size = sizeof(char) * (strlen(json) + 1);
    msg->body = calloc(1, msg->size);
    strcpy((char *)msg->body, json);

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

