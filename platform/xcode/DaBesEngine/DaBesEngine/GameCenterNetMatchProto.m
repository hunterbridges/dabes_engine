#include "net_match.h"
#include "engine.h"

NetMatchProto GameCenterMatchProto = {
    .init = NULL,
    .cleanup = NULL,
    .send_data = NULL,
    .rcv_data_cb = NULL
};