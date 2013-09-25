#import "DABGameCenterNet.h"

#include "net.h"
#include "engine.h"

typedef struct GameCenterNetCtx {
    CFTypeRef game_center_net;
} GameCenterNetCtx;

int GameCenterNet_init(Net *net, Engine *engine) {
    GameCenterNetCtx *ctx = calloc(1, sizeof(GameCenterNetCtx));
    
    DABGameCenterNet *gcn = [[DABGameCenterNet alloc] initWithEngine:engine];
    ctx->game_center_net = CFBridgingRetain(gcn);
    
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
    
    Scripting_call_hook(engine->scripting, net, "authenticated");
    
    return 1;
error:
    return 0;
}

int GameCenterNet_find_matches(Net *net, Engine *engine) {
    GameCenterNetCtx *ctx = net->context;
    DABGameCenterNet *gcn = (__bridge DABGameCenterNet *)ctx->game_center_net;
    
    [gcn findMatches];
    
    return 1;
error:
    return 0;
}

int GameCenterNet_find_matches_cb(Net *net, Engine *engine, int success) {
    check(net != NULL, "No Net for find_matches callback");
    
    if (success) {
        Scripting_call_hook(engine->scripting, net, "found_match");
    } else {
        Scripting_call_hook(engine->scripting, net, "find_failed");
    }
    
    return 1;
error:
    return 0;
}

NetProto GameCenterNetProto = {
    .init = GameCenterNet_init,
    .cleanup = GameCenterNet_cleanup,
    .authenticate = GameCenterNet_authenticate,
    .authenticate_cb = GameCenterNet_authenticate_cb,
    .find_matches = GameCenterNet_find_matches,
    .find_matches_cb = GameCenterNet_find_matches_cb
};
