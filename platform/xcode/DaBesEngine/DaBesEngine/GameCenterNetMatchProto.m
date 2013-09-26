#import <GameKit/GameKit.h>
#include "net_match.h"
#include "engine.h"

typedef struct GameCenterNetMatchCtx {
    CFTypeRef gk_match;
} GameCenterNetMatchCtx;

int GameCenterNetMatch_init(NetMatch *match, Engine *engine) {
    GameCenterNetMatchCtx *ctx = calloc(1, sizeof(GameCenterNetMatchCtx));
    
    match->context = ctx;
    
error:
    return 0;
}

int GameCenterNetMatch_cleanup(NetMatch *match) {
    GameCenterNetMatchCtx *ctx = match->context;
    if (ctx->gk_match) {
        CFBridgingRelease(ctx->gk_match);
        ctx->gk_match = NULL;
    }
    free(ctx);
    
error:
    return 0;
}

int GameCenterNetMatch_associate_native(NetMatch *match, void *assoc) {
    // Our *assoc is a GKMatch in disguise!
    GKMatch *gk_match = (__bridge GKMatch *)assoc;
    
    GameCenterNetMatchCtx *ctx = match->context;
    ctx->gk_match = CFBridgingRetain(gk_match);
    
    return 1;
error:
    return 0;
}

NetMatchProto GameCenterNetMatchProto = {
    .init = GameCenterNetMatch_init,
    .cleanup = GameCenterNetMatch_cleanup,
    .associate_native = GameCenterNetMatch_associate_native,
    .send_data = NULL,
    .rcv_data_cb = NULL
};