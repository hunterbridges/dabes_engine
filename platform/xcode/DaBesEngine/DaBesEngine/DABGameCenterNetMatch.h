//
//  DABNetMatch.h
//  DaBesEngine
//
//  Created by Hunter Bridges on 9/24/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <GameKit/GameKit.h>

#ifdef DABES_MAC
#import <DaBes-Mac.h>
#endif
#ifdef DABES_IOS
#import <DaBes-iOS.h>
#endif

@interface DABGameCenterNetMatch : NSObject

@property (nonatomic, assign, readonly) NSInteger playerCount;
@property (nonatomic, assign, readonly) Engine *engine;
@property (nonatomic, strong) GKMatch *gkMatch;
@property (nonatomic, assign) NetMatch *netMatch;

@property (nonatomic, assign) NSInteger localPlayerNumber;

- (id)initWithEngine:(Engine *)engine;
- (BOOL)sendMsg:(NetMatchMsg *)msg;
- (void)derivePlayerNumber;

@end
