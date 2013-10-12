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
#define VC_CLASS NSViewController
#endif

#ifdef DABES_IOS
#import <DABiOSEngineViewController.h>
#define VC_CLASS UIViewController
#endif

@interface DABGameCenterNet () <GKMatchmakerViewControllerDelegate>

@property (nonatomic, assign) BOOL didAuthenticate;
@property (nonatomic, assign) BOOL didRegisterInviteHandler;
@property (nonatomic, strong) GKMatch *currentMatch;
@property (nonatomic, strong) VC_CLASS *mmvc;

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

- (void)callAuthCallback {
  Net *net = self.engine->net;
  net->_(authenticate_cb)(net, self.engine);
}

- (void)authenticate:(BOOL)active
{
    if (self.didAuthenticate) {
      [self callAuthCallback];
      return;
    }
  
    if ([GKLocalPlayer localPlayer].isAuthenticated) {
        Net *net = self.engine->net;
        net->_(authenticate_cb)(net, self.engine);
        return;
    }
    
    [GKLocalPlayer localPlayer].authenticateHandler =
        ^(VC_CLASS *viewController, NSError *error) {
            if (error) {
                NSLog(@"%@", [error description]);
            }
          
            if (viewController && active) {
#ifdef DABES_MAC
                [[GKDialogController sharedDialogController]
                     presentViewController:(NSViewController<GKViewController> *)viewController];
#endif
#ifdef DABES_IOS
                [[DABiOSEngineViewController sharedInstance]
                     presentViewController:viewController
                     animated:YES
                     completion:^{
                     
                     }];
#endif
            } else {
                if ([GKLocalPlayer localPlayer].isAuthenticated) {
                    self.didAuthenticate = YES;
                }
            }
        };
}

- (void)registerInviteHandler {
  __weak typeof(self) wSelf = self;
  [GKMatchmaker sharedMatchmaker].inviteHandler =
      ^(GKInvite *acceptedInvite, NSArray *playerIDsToInvite) {
          __strong typeof(wSelf) sSelf = wSelf;
          GKMatchmakerViewController *mmvc =
            [[GKMatchmakerViewController alloc] initWithInvite:acceptedInvite];
          mmvc.matchmakerDelegate = sSelf;
        
          if (acceptedInvite) {
            if (sSelf.mmvc) {
              [sSelf dismiss:^{
                sSelf.mmvc = mmvc;
                [sSelf presentViewController:mmvc];
              }];
            } else {
              sSelf.mmvc = mmvc;
              [sSelf presentViewController:mmvc];
            }
          }
        
      };
  
  self.didRegisterInviteHandler = YES;
}

- (void)setDidAuthenticate:(BOOL)didAuthenticate {
  _didAuthenticate = didAuthenticate;
  if (didAuthenticate) {
    [self callAuthCallback];
     if (!self.didRegisterInviteHandler) {
       [self registerInviteHandler];
     }
  }
  
}

- (void)presentViewController:(VC_CLASS *)vc
{
#ifdef DABES_MAC
    [[GKDialogController sharedDialogController] presentViewController:vc];
#endif
#ifdef DABES_IOS
    [[DABiOSEngineViewController sharedInstance]
         presentViewController:vc animated:YES completion:^{
             
         }];
#endif
}

- (void)findMatches
{
    GKMatchRequest *request = [[GKMatchRequest alloc] init];
    request.minPlayers = 2;
    request.maxPlayers = 4;
  
    __weak typeof(request) wReq = request;
    __weak typeof(self) wSelf = self;
    request.inviteeResponseHandler = ^(NSString *playerID, GKInviteeResponse response) {
      if (response == GKInviteeResponseAccepted) {
        [[GKMatchmaker sharedMatchmaker] findMatchForRequest:wReq withCompletionHandler:^(GKMatch *match, NSError *error) {
          if (error) {
            // TODO: Handle error better
            NSAssert(NO, @"%@", error);
          }
          
          __strong typeof(wSelf) sSelf = wSelf;
          
          [[GKMatchmaker sharedMatchmaker] finishMatchmakingForMatch:match];
          [sSelf joinMatch:match];
        }];
      }
    };
    
    GKMatchmakerViewController *mmvc =
        [[GKMatchmakerViewController alloc] initWithMatchRequest:request];
    mmvc.matchmakerDelegate = self;
    self.mmvc = mmvc;
  
    [self presentViewController:mmvc];
}

- (void)joinMatch:(GKMatch *)match {
  if (self.currentMatch == match) {
      NSAssert(NO, @"Tried to join %@ twice", match);
  }
  
  if (self.currentMatch) {
      // TODO: Cleanup previous match
  }
  
  Net *net = self.engine->net;
  Engine *engine = self.engine;
  self.currentMatch = match;
  net->proto.joined_match_cb(net, engine, (__bridge void *)match);
}

#pragma mark - Matchmaker Delegate

- (void)dismiss:(void (^)())completion
{
#ifdef DABES_MAC
    [[GKDialogController sharedDialogController] dismiss:nil];
    self.mmvc = nil;
    if (completion) completion();
#endif
#ifdef DABES_IOS
  [[DABiOSEngineViewController sharedInstance] dismissViewControllerAnimated:YES completion:^{
    self.mmvc = nil;
    if (completion) completion();
  }];
#endif
    
}

- (void)matchmakerViewController:(GKMatchmakerViewController *)viewController
                didFailWithError:(NSError *)error
{
    
}

- (void)matchmakerViewController:(GKMatchmakerViewController *)viewController
                    didFindMatch:(GKMatch *)match
{
    [self dismiss:^{
        [self joinMatch:match];
    }];
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
    [self dismiss:^{
        Net *net = self.engine->net;
        net->_(joined_match_cb)(net, self.engine, NULL);
    }];
}

@end
