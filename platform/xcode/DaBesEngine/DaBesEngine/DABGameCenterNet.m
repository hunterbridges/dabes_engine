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

@property (nonatomic, strong) NSWindow *popup;

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

#ifdef DABES_IOS
#endif

#ifdef DABES_MAC

- (void)authenticate
{
    /*
    DABMacEngineViewController *vc =
        [DABMacEngineViewController sharedInstance];
     */
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
}

- (void)findMatches
{
    /*
    DABMacEngineViewController *vc =
        [DABMacEngineViewController sharedInstance];
     */
    GKMatchRequest *request = [[GKMatchRequest alloc] init];
    request.minPlayers = 2;
    request.maxPlayers = 4;
    
    GKMatchmakerViewController *mmvc =
        [[GKMatchmakerViewController alloc] initWithMatchRequest:request];
    mmvc.matchmakerDelegate = self;
    [[GKDialogController sharedDialogController] presentViewController:mmvc];
}

- (void)sheetDidEnd:(NSWindow *)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    
}

#endif

#pragma mark - Matchmaker Delegate

- (void)matchmakerViewController:(GKMatchmakerViewController *)viewController
                didFailWithError:(NSError *)error
{
    
}

- (void)matchmakerViewController:(GKMatchmakerViewController *)viewController
                    didFindMatch:(GKMatch *)match
{
    
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
#ifdef DABES_MAC
    [[GKDialogController sharedDialogController] dismiss:viewController];
#endif
}

@end
