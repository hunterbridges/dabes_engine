//
//  SDKScriptEditorView.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/26/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <MGSFragaria/MGSFragaria.h>
#import <DaBes-Mac/DABMacEngineViewController.h>
#import "SDKScriptEditorView.h"
#import "SDKScriptEditorWindowController.h"
#import "SDKScriptTabModel.h"

@interface SDKScriptEditorView () <MGSFragariaTextViewDelegate>

@property (nonatomic, strong) NSString *queuedScript;
@property (nonatomic, strong) MGSFragaria *fragaria;
@property (nonatomic, copy) NSNumber *errorLine;
@property (nonatomic, copy) NSString *errorMessage;

@end

@implementation SDKScriptEditorView

- (id)initWithPath:(NSString *)path
{
  self = [super init];
  if (self) {
    self.fragaria = [[MGSFragaria alloc] init];
    [self.fragaria setObject:self forKey:MGSFODelegate];
    [self.fragaria setObject:@"Lua" forKey:MGSFOSyntaxDefinitionName];
    [self.fragaria embedInView:self];
    
    self.path = path;
    
    [[NSNotificationCenter defaultCenter]
        addObserver:self
        selector:@selector(loadQueuedScript)
        name:kEngineReadyForScriptNotification
        object:nil];
    
  }
  
  return self;
}

- (void)setPath:(NSString *)path {
  _path = [path copy];
  if (path) {
    NSError *error = nil;
    NSString *script = [NSString stringWithContentsOfFile:path
                                                 encoding:NSUTF8StringEncoding
                                                    error:&error];
    if (error) {
      return;
    }
    self.fragaria.string = script;
  }
}

- (void)reinject {
  self.queuedScript = self.fragaria.string;
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
  self.tabModel.isEdited = YES;
}

+ (NSNumber *)extractErrorLine:(NSString *)string
                      filename:(NSString **)filename
                       message:(NSString **)message {
  NSRange colonRange = [string rangeOfString:@":"];
  if (colonRange.location == NSNotFound) {
    *message = string;
    return nil;
  }
  
  NSCharacterSet *numberSet =
      [NSCharacterSet characterSetWithCharactersInString:@"1234567890"];
  NSScanner *scanner = [NSScanner scannerWithString:string];
  
  NSString *fn = nil;
  [scanner scanUpToString:@":" intoString:&fn];
  if (filename != NULL) *filename = fn;
  
  scanner.scanLocation += 1;
  NSString *lineNumber = nil;
  [scanner scanCharactersFromSet:numberSet intoString:&lineNumber];
  scanner.scanLocation += 1;
  
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
    NSString *string = [[NSString alloc] initWithCString:lua_tostring(L, -1)
                                                encoding:NSUTF8StringEncoding];
    NSNumber *errorLine = [SDKScriptEditorView extractErrorLine:string
                                                       filename:NULL
                                                        message:&errMsg];
    [self setErrorLine:errorLine withMessage:errMsg];
    //NSLog(@"Error running script:\n    %@\n", errMsg);
    lua_pop(L, 1);
    return;
  }
  
  int result = lua_pcall(L, 0, 0, 0);
  if (result != 0) {
    NSString *errMsg = nil;
    NSString *string = [[NSString alloc] initWithCString:lua_tostring(L, -1)
                                                encoding:NSUTF8StringEncoding];
    NSNumber *errorLine = [SDKScriptEditorView extractErrorLine:string
                                                       filename:NULL
                                                        message:&errMsg];
    [self setErrorLine:errorLine withMessage:errMsg];
    //NSLog(@"Error running script:\n    %@\n", errMsg);
    lua_pop(L, 1);
    return;
  }
  
  [self setErrorLine:nil withMessage:nil];
}

- (void)setErrorLine:(NSNumber *)errorLine withMessage:(NSString *)message {
  if ([errorLine isEqual:self.errorLine] &&
      [message isEqualToString:self.errorMessage]) return;
  if (errorLine) {
    SMLSyntaxError *syntaxError = [[SMLSyntaxError alloc] init];
    syntaxError.description = message;
    syntaxError.line = [errorLine intValue] + 1;
    
    self.fragaria.syntaxErrors = @[syntaxError];
    self.tabModel.objectCount = 1;
  } else {
    self.fragaria.syntaxErrors = nil;
    self.tabModel.objectCount = 0;
  }
  
  self.errorLine = errorLine;
  self.errorMessage = message;
}

- (NSString *)string {
  return self.fragaria.string;
}

@end
