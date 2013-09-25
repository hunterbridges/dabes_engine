//
//  DABGameCenterNet.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 9/24/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "DABGameCenterNet.h"
#import <GameKit/GameKit.h>

#ifdef DABES_MAC
#import <DABMacEngineViewController.h>
#endif
#ifdef DABES_IOS
#import <DABiOSEngineViewController.h>
#endif

@interface DABGameCenterNet () <GKMatchmakerViewControllerDelegate>

@end

@implementation DABGameCenterNet

- (id)initWithEngine:(Engine *)engine
{
    self = [super init];
    if (self)
    {
        _engine = engine;
    }
    return self;
}

- (void)authenticate
{
    if ([GKLocalPlayer localPlayer].isAuthenticated) {
        Net *net = self.engine->net;
        net->_(authenticate_cb)(net, self.engine);
        return;
    }
    
#ifdef DABES_MAC
    [GKLocalPlayer localPlayer].authenticateHandler =
        ^(NSViewController *viewController, NSError *error) {
            if (error) {
                NSLog(@"%@", [error description]);
            }
            
            if (viewController) {
                [[GKDialogController sharedDialogController]
                     presentViewController:(NSViewController<GKViewController> *)viewController];
            } else {
                Net *net = self.engine->net;
                net->_(authenticate_cb)(net, self.engine);
            }
        
        };
#endif
    
#ifdef DABES_IOS
    [GKLocalPlayer localPlayer].authenticateHandler =
        ^(UIViewController *viewController, NSError *error) {
            if (error) {
                NSLog(@"%@", [error description]);
            }
            
            if (viewController) {
                [[DABiOSEngineViewController sharedInstance]
                    presentViewController:viewController
                    animated:YES
                    completion:^{
                     
                    }];
            } else {
                Net *net = self.engine->net;
                net->_(authenticate_cb)(net, self.engine);
            }
        
        };
#endif
}

- (void)findMatches
{
    GKMatchRequest *request = [[GKMatchRequest alloc] init];
    request.minPlayers = 2;
    request.maxPlayers = 4;
    
    GKMatchmakerViewController *mmvc =
        [[GKMatchmakerViewController alloc] initWithMatchRequest:request];
    mmvc.matchmakerDelegate = self;
#ifdef DABES_MAC
    [[GKDialogController sharedDialogController] presentViewController:mmvc];
#endif
#ifdef DABES_IOS
    [[DABiOSEngineViewController sharedInstance]
         presentViewController:mmvc animated:YES completion:^{
             
         }];
#endif
}

#pragma mark - Matchmaker Delegate

- (void)dismiss
{
#ifdef DABES_MAC
    [[GKDialogController sharedDialogController] dismiss:viewController];
#endif
#ifdef DABES_IOS
    [[DABiOSEngineViewController sharedInstance] dismissViewControllerAnimated:YES completion:nil];
#endif
    
}

- (void)matchmakerViewController:(GKMatchmakerViewController *)viewController
                didFailWithError:(NSError *)error
{
    
}

- (void)matchmakerViewController:(GKMatchmakerViewController *)viewController
                    didFindMatch:(GKMatch *)match
{
    [self dismiss];
    Net *net = self.engine->net;
    net->_(find_matches_cb)(net, self.engine, 1);
}

- (void)matchmakerViewController:(GKMatchmakerViewController *)viewController
                  didFindPlayers:(NSArray *)playerIDs
{
    
}

- (void)matchmakerViewController:(GKMatchmakerViewController *)viewController
    didReceiveAcceptFromHostedPlayer:(NSString *)playerID
{
    
}

- (void)matchmakerViewControllerWasCancelled:(GKMatchmakerViewController *)viewController
{
    [self dismiss];
    Net *net = self.engine->net;
    net->_(find_matches_cb)(net, self.engine, 0);
}

@end
