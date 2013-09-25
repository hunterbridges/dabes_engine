#import "DABGameCenterNet.h"

#include "net.h"
#include "engine.h"

typedef struct GameCenterNetCtx {
    CFTypeRef game_center_net;
} GameCenterNetCtx;

int GameCenterNet_init(Net *net, Engine *engine) {
    check(net != NULL, "No Net to init");
    
    GameCenterNetCtx *ctx = calloc(1, sizeof(GameCenterNetCtx));
    check(ctx != NULL, "Couldn't create GameCenterNetCtx");
    
    ctx->game_center_net = CFBridgingRetain([[DABGameCenterNet alloc] init]);
    
    net->context = ctx;
    
    return 1;
error:
    return 0;
}

int GameCenterNet_cleanup(Net *net) {
    check(net != NULL, "No Net to cleanup");
    
    GameCenterNetCtx *ctx = net->context;
    CFBridgingRelease(ctx->game_center_net);
    ctx->game_center_net = NULL;
    
    free(ctx);
    
    return 1;
error:
    return 0;
}

int GameCenterNet_authenticate(Net *net, Engine *engine) {
    GameCenterNetCtx *ctx = net->context;
    DABGameCenterNet *gcn = (__bridge DABGameCenterNet *)ctx->game_center_net;
    
    [gcn authenticate];
    
    return 1;
error:
    return 0;
}

int GameCenterNet_authenticate_cb(Net *net, Engine *engine) {
    check(net != NULL, "No Net for authenticate callback");
    
    return 1;
error:
    return 0;
}

NetProto GameCenterNetProto = {
    .init = GameCenterNet_init,
    .cleanup = GameCenterNet_cleanup,
    .authenticate = GameCenterNet_authenticate,
    .authenticate_cb = GameCenterNet_authenticate_cb
};
