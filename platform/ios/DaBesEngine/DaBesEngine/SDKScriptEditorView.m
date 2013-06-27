//
//  SDKScriptEditorView.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/26/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <MGSFragaria/MGSFragaria.h>
#import "SDKEngineViewController.h"
#import "SDKScriptEditorView.h"

@interface SDKScriptEditorView () <MGSFragariaTextViewDelegate>

@property (nonatomic, strong) MGSFragaria *fragaria;
@property (nonatomic, strong) NSString *queuedScript;

@end

@implementation SDKScriptEditorView

- (id)initWithFrame:(NSRect)frame
{
  self = [super initWithFrame:frame];
  if (self) {
      // Initialization code here.
    self.fragaria = [[MGSFragaria alloc] init];
    [self.fragaria setObject:self forKey:MGSFODelegate];
    [self.fragaria setObject:@"Lua" forKey:MGSFOSyntaxDefinitionName];
    [self.fragaria embedInView:self];
    
    NSString *bundlePath = [[[NSBundle mainBundle] resourcePath]
                            stringByAppendingPathComponent:@"scripts/boxfall/entities/megaman.lua"];
    NSString *megamanScript =
        [NSString stringWithContentsOfFile:bundlePath
                                  encoding:NSUTF8StringEncoding
                                     error:nil];
    self.fragaria.string = megamanScript;
    [self.fragaria addObserver:self
                    forKeyPath:@"string"
                       options:NSKeyValueObservingOptionNew
                       context:NULL];
    [[NSNotificationCenter defaultCenter]
        addObserver:self
        selector:@selector(loadQueuedScript)
        name:kEngineReadyForScriptNotification
        object:nil];
    
  }
  
  return self;
}

- (void)dealloc {
  [self.fragaria removeObserver:self forKeyPath:@"string"];
}

- (void)drawRect:(NSRect)dirtyRect
{
  // Drawing code here.
}

- (void)textDidChange:(NSNotification *)notification {
  self.queuedScript = self.fragaria.string;
}

- (NSNumber *)extractErrorLine:(const char *)luaError
                       message:(NSString **)message {
  NSString *string = [[NSString alloc] initWithCString:luaError
                                              encoding:NSUTF8StringEncoding];
  NSCharacterSet *numberSet =
      [NSCharacterSet characterSetWithCharactersInString:@"1234567890"];
  NSScanner *scanner = [NSScanner scannerWithString:string];
  [scanner scanUpToString:@"]:" intoString:nil];
  
  scanner.scanLocation += 2;
  NSString *lineNumber = nil;
  [scanner scanCharactersFromSet:numberSet intoString:&lineNumber];
  scanner.scanLocation += 2;
  
  *message = [string substringFromIndex:scanner.scanLocation];
  
  NSNumberFormatter *f = [[NSNumberFormatter alloc] init];
  [f setNumberStyle:NSNumberFormatterDecimalStyle];
  NSNumber *line = [f numberFromString:lineNumber];
  return line;
}

- (void)loadQueuedScript {
  if (self.queuedScript == nil) return;
  const char *newScript =
      [self.queuedScript cStringUsingEncoding:NSUTF8StringEncoding];
  self.queuedScript = nil;
  lua_State *L = self.scriptManager.engineVC.engine->scripting->L;
  int error = luaL_loadstring(self.scriptManager.engineVC.engine->scripting->L,
                              newScript);
  if (error != 0) {
    NSString *errMsg = nil;
    NSNumber *errorLine = [self extractErrorLine:lua_tostring(L, -1)
                                         message:&errMsg];
    [self setErrorLine:errorLine withMessage:errMsg];
    //NSLog(@"Error running script:\n    %@\n", errMsg);
    lua_pop(L, 1);
    return;
  }
  
  int result = lua_pcall(L, 0, 0, 0);
  if (result != 0) {
    NSString *errMsg = nil;
    NSNumber *errorLine = [self extractErrorLine:lua_tostring(L, -1)
                                         message:&errMsg];
    [self setErrorLine:errorLine withMessage:errMsg];
    //NSLog(@"Error running script:\n    %@\n", errMsg);
    lua_pop(L, 1);
    return;
  }
  
  [self setErrorLine:nil withMessage:nil];
}

- (void)setErrorLine:(NSNumber *)errorLine withMessage:(NSString *)message {
  if (errorLine) {
    SMLSyntaxError *syntaxError = [[SMLSyntaxError alloc] init];
    syntaxError.description = message;
    syntaxError.line = [errorLine intValue] + 1;
    
    self.fragaria.syntaxErrors = @[syntaxError];
  } else {
    self.fragaria.syntaxErrors = nil;
  }
  
}


@end
