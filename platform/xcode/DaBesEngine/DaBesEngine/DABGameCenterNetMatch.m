//
//  DABNetMatch.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 9/24/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "DABGameCenterNetMatch.h"
#import "console.h"

@interface DABGameCenterNetMatch () <GKMatchDelegate>

@property (nonatomic, strong) NSMutableDictionary *playerAssignments;
@property (nonatomic, strong) NSMutableDictionary *idNumbers;
@property (nonatomic, strong) NSMutableDictionary *readies;
@property (nonatomic, strong) NSMutableDictionary *connections;
@property (nonatomic, assign) BOOL allPlayersConnected;
@property (nonatomic, assign) BOOL wantsPlayerNumber;

@end

@implementation DABGameCenterNetMatch

- (id)initWithEngine:(Engine *)engine {
    self = [super init];
    if (self)
    {
        _engine = engine;
        self.playerAssignments = [NSMutableDictionary dictionary];
        self.idNumbers = [NSMutableDictionary dictionary];
        self.readies = [NSMutableDictionary dictionary];
        self.connections = [NSMutableDictionary dictionary];
    }
    return self;
}

#pragma mark - Public

- (dab_uint8)playerCount {
    return self.gkMatch.playerIDs.count + 1;
}

- (void)derivePlayerNumber {
    self.wantsPlayerNumber = YES;
    
    if (self.gkMatch.expectedPlayerCount == 0 &&
            !self.allPlayersConnected) {
        self.allPlayersConnected = YES;
    }
}

- (void)setGkMatch:(GKMatch *)gkMatch {
    _gkMatch = gkMatch;
    gkMatch.delegate = self;
}

- (BOOL)sendMsg:(NetMatchMsg *)msg {
    if (msg->from == 0) {
        // Mark it as coming from local player.
        msg->from = self.localPlayerNumber;
    }
    
    NSData *data = [self dataFromNetMsg:msg];
    NSError *error = nil;
    
    if (msg->to == 0) {
        // Send to all players
        [self.gkMatch sendDataToAllPlayers:data
                              withDataMode:GKMatchSendDataReliable
                                     error:&error];
    } else {
        // Send to specific player
        NSString *player = [self getPlayerWithNumber:msg->to];
        [self.gkMatch sendData:data
                     toPlayers:@[player]
                  withDataMode:GKMatchSendDataReliable
                         error:&error];
    }
    
    if (error) {
        NSString *startStr =
            [NSString stringWithFormat:@"DABGameCenterNetMatch -sendMsg: %@",
                error.description];
        const char *cstr =
            [startStr cStringUsingEncoding:NSUTF8StringEncoding];
        self.engine->console->_(log)(self.engine->console, (char *)cstr,
                                     "WARN");
        return NO;
    }
    
    return YES;
}

- (void)getMetadataForAllPlayers {
    NSArray *allPlayers = [self.gkMatch.playerIDs arrayByAddingObject:
                           [GKLocalPlayer localPlayer].playerID];
    [GKPlayer loadPlayersForIdentifiers:allPlayers
        withCompletionHandler:^(NSArray *players, NSError *error) {
            if (error) {
                NSAssert(self, @"Metadata error: %@", error);
                return;
            }
            
            NSArray *orderedPlayers =
                [players sortedArrayWithOptions:0
                    usingComparator:^NSComparisonResult(id obj1, id obj2) {
                        GKPlayer *left = obj1;
                        GKPlayer *right = obj2;
                        NSNumber *leftNum =
                            @([self numberForPlayer:left.playerID]);
                        NSNumber *rightNum =
                            @([self numberForPlayer:right.playerID]);
                        return [leftNum compare:rightNum];
                    }];
            
            self.netMatch->_(got_metadata_cb)(self.netMatch, self.engine,
                                              (__bridge void *)orderedPlayers);
        }];
}

#pragma mark - Private

- (NSString *)getPlayerWithNumber:(dab_uint8)number {
    return self.playerAssignments[@(number)];
}

- (dab_uint8)numberForPlayer:(NSString *)playerID {
    return [self.idNumbers[playerID] intValue];
}

- (void)markReady:(dab_uint8)playerNum {
    self.readies[@(playerNum)] = @(YES);
    
    if (self.readies.count == self.playerCount &&
            self.netMatch->proto.all_ready_cb) {
        self.netMatch->_(all_ready_cb)(self.netMatch, self.engine);
    }
}

- (void)readyUp {
    NetMatchMsg *readyMsg =
        NetMatchMsg_player_ready(self.localPlayerNumber);
    [self sendMsg:readyMsg];
    NetMatchMsg_destroy(readyMsg);
    
    [self markReady:self.localPlayerNumber];
}

- (NSData *)dataFromNetMsg:(NetMatchMsg *)msg {
    NSMutableData *data = [[NSMutableData alloc] init];
    
    // From
    [data appendBytes:&msg->from length:sizeof(dab_uint8)];
    
    // To
    [data appendBytes:&msg->to length:sizeof(dab_uint8)];
    
    // Kind
    [data appendBytes:&msg->kind length:sizeof(NetMatchMsgType)];
    
    // Body
    [data appendBytes:msg->body length:msg->size];
    
    return data;
}

- (NetMatchMsg *)netMsgFromData:(NSData *)data {
    NSRange dataRange = NSMakeRange(0, data.length);
    NetMatchMsg *msg = calloc(1, sizeof(NetMatchMsg));
    
    size_t el_sz = 0;
    NSData *tmp = nil;
    
    // From
    el_sz = sizeof(dab_uint8);
    tmp = [data subdataWithRange:NSMakeRange(dataRange.location, el_sz)];
    dataRange.location += el_sz;
    dataRange.length -= el_sz;
    memcpy(&msg->from, tmp.bytes, el_sz);
    
    // To
    el_sz = sizeof(dab_uint8);
    tmp = [data subdataWithRange:NSMakeRange(dataRange.location, el_sz)];
    dataRange.location += el_sz;
    dataRange.length -= el_sz;
    memcpy(&msg->to, tmp.bytes, el_sz);
    
    // Kind
    el_sz = sizeof(NetMatchMsgType);
    tmp = [data subdataWithRange:NSMakeRange(dataRange.location, el_sz)];
    dataRange.location += el_sz;
    dataRange.length -= el_sz;
    memcpy(&msg->kind, tmp.bytes, el_sz);
    
    if (dataRange.length) {
        // Body
        tmp = [data subdataWithRange:dataRange];
        msg->size = dataRange.length;
        dab_uchar *body = calloc(1, dataRange.length);
        memcpy(body, tmp.bytes, dataRange.length);
        msg->body = body;
    }
    
    return msg;
}

- (void)assignPlayerNumber:(dab_uint8)num toPlayerID:(NSString *)pid {
    self.playerAssignments[@(num)] = pid;
    self.idNumbers[pid] = @(num);
    
    if ([pid isEqualToString:[GKLocalPlayer localPlayer].playerID]) {
       self.localPlayerNumber = num;
    }
}

- (void)derivePlayerNumberIfNeeded {
    if (!self.allPlayersConnected || !self.wantsPlayerNumber) {
        return;
    }
    
    [self.gkMatch chooseBestHostPlayerWithCompletionHandler:^(NSString *playerID) {
        if ([playerID isEqualToString:[GKLocalPlayer localPlayer].playerID]) {
            // I am the host, I am player 1 and need to order the others
            NSMutableArray *orderedIDs = [NSMutableArray array];
            [orderedIDs addObject:playerID];
            [orderedIDs addObjectsFromArray:self.gkMatch.playerIDs];
            
            NSInteger i = 1;
            for (NSString *pid in orderedIDs) {
                [self assignPlayerNumber:i toPlayerID:pid];
                
                NetMatchMsg *assignMsg =
                    NetMatchMsg_player_assign(self.localPlayerNumber, i);
                [self sendMsg:assignMsg];
                
                i++;
            }
            
            [self markReady:self.localPlayerNumber];
        } else if (playerID) {
            // I need to wait for a player assign message
        } else if (playerID == nil) {
            // Retry
            // TODO Limit retries and eventually error out if it fails too much
            [NSTimer scheduledTimerWithTimeInterval:2.0 target:self selector:@selector(derivePlayerNumberIfNeeded) userInfo:nil repeats:NO];
        }
       }];
}

- (void)setWantsPlayerNumber:(BOOL)wantsPlayerNumber {
    _wantsPlayerNumber = wantsPlayerNumber;
    [self derivePlayerNumberIfNeeded];
}

- (void)setAllPlayersConnected:(BOOL)allPlayersConnected {
    _allPlayersConnected = allPlayersConnected;
    [self derivePlayerNumberIfNeeded];
}

#pragma mark - GKMatch Delegate

- (void)match:(GKMatch *)match
    didReceiveData:(NSData *)data
    fromPlayer:(NSString *)playerID {
    
    NetMatchMsg *msg = [self netMsgFromData:data];
    if (msg->kind == NET_MATCH_MSG_PLAYER_ASSIGN) {
        [self assignPlayerNumber:1 toPlayerID:playerID];
        [self markReady:msg->from];
        
        [self assignPlayerNumber:msg->to
                      toPlayerID:[GKLocalPlayer localPlayer].playerID];
        [self readyUp];
    }
    
    if (msg->kind == NET_MATCH_MSG_PLAYER_READY) {
        [self assignPlayerNumber:msg->from toPlayerID:playerID];
        [self markReady:msg->from];
    }
    
    dab_uint8 localFrom = [self numberForPlayer:playerID];
    NSAssert(msg->from == localFrom,
             @"Message said it was from player %d, but local player thinks "
             @"ID %@ is player %d", msg->from, playerID, localFrom);
    NSAssert(msg->to == 0 || msg->to == self.localPlayerNumber,
             @"Message was meant for player %d, but local player is player %d",
             msg->to, self.localPlayerNumber);
    
    self.netMatch->_(rcv_msg_cb)(self.netMatch, self.engine, msg);
    
    NetMatchMsg_destroy(msg);
}

- (void)match:(GKMatch *)match didFailWithError:(NSError *)error {
    NSAssert(self, @"%@", error);
}

- (void)match:(GKMatch *)match
    player:(NSString *)playerID
    didChangeState:(GKPlayerConnectionState)state {
    
    switch (state) {
        case GKPlayerStateConnected:
            // Handle a new player connection.
            self.connections[playerID] = @(YES);
            break;
        case GKPlayerStateDisconnected:
            // A player just disconnected.
            [self.connections removeObjectForKey:playerID];
            break;
    }
    if (!self.allPlayersConnected && match.expectedPlayerCount == 0) {
        self.allPlayersConnected = YES;
    }
}

@end
