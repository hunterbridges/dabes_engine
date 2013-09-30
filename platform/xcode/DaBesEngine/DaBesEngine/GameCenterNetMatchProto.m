#import <GameKit/GameKit.h>
#import <DABGameCenterNetMatch.h>
#include "net_match.h"
#include "engine.h"

typedef struct GameCenterNetMatchCtx {
    CFTypeRef gcn_match;
} GameCenterNetMatchCtx;

int GameCenterNetMatch_init(NetMatch *match, Engine *engine) {
    GameCenterNetMatchCtx *ctx = calloc(1, sizeof(GameCenterNetMatchCtx));
    
    match->context = ctx;
    
    DABGameCenterNetMatch *gcn_match =
        [[DABGameCenterNetMatch alloc] initWithEngine:engine];
    ctx->gcn_match = CFBridgingRetain(gcn_match);
    return 1;
}

int GameCenterNetMatch_cleanup(NetMatch *match) {
    GameCenterNetMatchCtx *ctx = match->context;
    if (ctx->gcn_match) {
        CFBridgingRelease(ctx->gcn_match);
        ctx->gcn_match = NULL;
    }
    free(ctx);
    return 1;
}

int GameCenterNetMatch_associate_native(NetMatch *match, void *assoc) {
    // Our *assoc is a GKMatch in disguise!
    GKMatch *gk_match = (__bridge GKMatch *)assoc;
    
    GameCenterNetMatchCtx *ctx = match->context;
    DABGameCenterNetMatch *gcn_match =
        (__bridge DABGameCenterNetMatch *)ctx->gcn_match;
    
    gcn_match.gkMatch = gk_match;
    gcn_match.netMatch = match;
    
    return 1;
}

int GameCenterNetMatch_handshake(NetMatch *match, struct Engine *engine) {
    GameCenterNetMatchCtx *ctx = match->context;
    DABGameCenterNetMatch *gcn_match =
        (__bridge DABGameCenterNetMatch *)ctx->gcn_match;
    
    [gcn_match derivePlayerNumber];
    
    return 1;
}

int GameCenterNetMatch_all_ready_cb(NetMatch *match, struct Engine *engine) {
    Scripting_call_hook(engine->scripting, match, "all_ready");
    
    return 1;
}

int GameCenterNetMatch_get_metadata(NetMatch *match, struct Engine *engine) {
    GameCenterNetMatchCtx *ctx = match->context;
    DABGameCenterNetMatch *gcn_match =
        (__bridge DABGameCenterNetMatch *)ctx->gcn_match;
    
    [gcn_match getMetadataForAllPlayers];
    
    return 1;
}

static int metadata_unpack_players(lua_State *L, void *context) {
    NSArray *arr = (__bridge NSArray *)context;
    // Expected an ordered NSArray of GKPlayers
    
    lua_createtable(L, (int)arr.count, 0);
    int player_number = 1;
    for (GKPlayer *player in arr) {
        lua_pushinteger(L, player_number);
        lua_newtable(L);
        
        const char *name_str =
            [[player displayName] cStringUsingEncoding:NSUTF8StringEncoding];
        lua_pushstring(L, name_str);
        lua_setfield(L, -2, "display_name");
        
        const char *alias_str =
            [[player alias] cStringUsingEncoding:NSUTF8StringEncoding];
        lua_pushstring(L, alias_str);
        lua_setfield(L, -2, "alias");
        
        lua_settable(L, -3);
        player_number++;
    }
    
    return 1;
}

int GameCenterNetMatch_got_metadata_cb(NetMatch *match, struct Engine *engine,
                                       void *metadata) {
    Scripting_dhook_arg_closure closure = {
        .function = metadata_unpack_players,
        .context = metadata
    };
    Scripting_call_dhook(engine->scripting, match, "got_metadata",
                         LUA_TFUNCTION, &closure, nil);
    
    return 1;
}


int GameCenterNetMatch_get_player_count(NetMatch *match, struct Engine *engine) {
    GameCenterNetMatchCtx *ctx = match->context;
    DABGameCenterNetMatch *gcn_match =
        (__bridge DABGameCenterNetMatch *)ctx->gcn_match;
    
    return gcn_match.playerCount;
}

int GameCenterNetMatch_get_player_number(NetMatch *match, struct Engine *engine) {
    GameCenterNetMatchCtx *ctx = match->context;
    DABGameCenterNetMatch *gcn_match =
        (__bridge DABGameCenterNetMatch *)ctx->gcn_match;
    
    return gcn_match.localPlayerNumber;
}

int GameCenterNetMatch_send_msg(struct NetMatch *match, struct Engine *engine,
                                NetMatchMsg *msg) {
    GameCenterNetMatchCtx *ctx = match->context;
    DABGameCenterNetMatch *gcn_match =
        (__bridge DABGameCenterNetMatch *)ctx->gcn_match;
    return [gcn_match sendMsg:msg];
}

int json_decode_func(lua_State *L, void *context) {
    NetMatchMsg *msg = context;
    
    if (msg->size == 0) {
        lua_pushnil(L);
        return 1;
    }
    
    lua_getglobal(L, "json");
    lua_getfield(L, -1, "decode");
    lua_remove(L, -2);
    lua_pushstring(L, (const char *)msg->body);
    int rc = lua_pcall(L, 1, 1, 1);
    if (rc) {
        const char *error = lua_tostring(L, -1);
        printf("JSON Decode error: %s", error);
        lua_pop(L, 1);
        lua_pushnil(L);
    }
    
    return 1;
}

int GameCenterNetMatch_rcv_msg_cb(struct NetMatch *match, struct Engine *engine,
                                  NetMatchMsg *msg) {
    if (msg->kind == NET_MATCH_MSG_NULL) {
        Scripting_call_dhook(engine->scripting, match, "received_msg",
                             LUA_TNUMBER, (float)msg->from,
                             LUA_TNUMBER, (float)msg->kind,
                             nil);
    } else if (msg->kind == NET_MATCH_MSG_PACKED_RECORDER) {
        Recorder *recorder = luaL_instantiate_recorder(engine->scripting->L);
        
        // TODO This is not a flexible solution.
        ChipmunkRecorder_contextualize(recorder);
        recorder->_(unpack)(recorder, msg->body, msg->size);
    
        Scripting_call_dhook(engine->scripting, match, "received_msg",
                             LUA_TNUMBER, (float)msg->from,
                             LUA_TNUMBER, (float)msg->kind,
                             LUA_TUSERDATA, recorder,
                             nil);
        
        lua_pop(engine->scripting->L, 1);
    } else if (msg->kind == NET_MATCH_MSG_JSON) {
        lua_State *L = engine->scripting->L;
        int irc = luaL_dostring(L, "require 'lib.json'");
        if (irc) {
            return 0;
        }
        
        Scripting_dhook_arg_closure closure = {
            .function = json_decode_func,
            .context = msg
        };
        
        Scripting_call_dhook(engine->scripting, match, "received_msg",
                             LUA_TNUMBER, (float)msg->from,
                             LUA_TNUMBER, (float)msg->kind,
                             LUA_TFUNCTION, &closure,
                             nil);
        
        lua_pop(engine->scripting->L, 1);
    }
    
    return 1;
}

NetMatchProto GameCenterNetMatchProto = {
    .init = GameCenterNetMatch_init,
    .cleanup = GameCenterNetMatch_cleanup,
    .associate_native = GameCenterNetMatch_associate_native,
    .handshake = GameCenterNetMatch_handshake,
    .all_ready_cb = GameCenterNetMatch_all_ready_cb,
    .get_metadata = GameCenterNetMatch_get_metadata,
    .got_metadata_cb = GameCenterNetMatch_got_metadata_cb,
    .get_player_count = GameCenterNetMatch_get_player_count,
    .get_player_number = GameCenterNetMatch_get_player_number,
    .send_msg = GameCenterNetMatch_send_msg,
    .rcv_msg_cb = GameCenterNetMatch_rcv_msg_cb
};