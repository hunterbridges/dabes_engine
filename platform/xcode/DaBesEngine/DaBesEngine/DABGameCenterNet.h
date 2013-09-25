//
//  DABGameCenterNet.h
//  DaBesEngine
//
//  Created by Hunter Bridges on 9/24/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <Foundation/Foundation.h>

#ifdef DABES_MAC
#import <DaBes-Mac.h>
#endif
#ifdef DABES_IOS
#import <DaBes-iOS.h>
#endif

@interface DABGameCenterNet : NSObject

@property (nonatomic, assign, readonly) Engine *engine;

- (id)initWithEngine:(Engine *)engine;

- (void)authenticate;
- (void)findMatches;

@end
