//
//  ScriptEditorViewController.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 5/31/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "CodeEditorView.h"
#import "EngineViewController.h"
#import "ScriptEditorViewController.h"
#import "util.h"
#import "engine.h"
#import "scripting.h"

static const NSTimeInterval kDebounceInterval = 0.1;

@interface ScriptEditorViewController () <CodeEditorViewDelegate>

@property (nonatomic, strong) CodeEditorView *editorView;
@property (nonatomic, weak) EngineViewController *engineVC;
@property (nonatomic, strong) NSTimer *debounce;
@property (nonatomic, strong) NSString *path;

@end

@implementation ScriptEditorViewController

- (id)initWithEngineVC:(EngineViewController *)engineVC
              withPath:(NSString *)path {
  self = [super init];
  if (self) {
    self.engineVC = engineVC;
    self.path = path;
    
    [[NSNotificationCenter defaultCenter]
        addObserver:self
        selector:@selector(loadQueuedScript)
        name:kEngineReadyForScriptNotification
        object:nil];
  }
  return self;
}

- (void)dealloc {
  [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)viewDidLoad
{
  [super viewDidLoad];
  
  self.navigationItem.title = [self.path lastPathComponent];
  
  self.editorView = [[CodeEditorView alloc] initWithFrame:self.view.bounds];
  self.editorView.autoresizingMask = (UIViewAutoresizingFlexibleWidth |
                                    UIViewAutoresizingFlexibleHeight);
  self.editorView.delegate = self;
  self.editorView.linkTo = self.engineVC;
  [self.view addSubview:self.editorView];
  
  [self loadScript];
}

- (void)loadScript {
  NSError *error = nil;
  NSString *str = [NSString stringWithContentsOfFile:self.path
                                            encoding:NSUTF8StringEncoding
                                               error:&error];
  if (error) {
    NSLog(@"%@", error.description);
    return;
  }
  self.editorView.text = str;
}

- (void)codeEditorDidChange:(CodeEditorView *)codeEditor {
  if (codeEditor.text == nil) return;
  if (self.debounce) [self.debounce invalidate];
  self.debounce = [NSTimer scheduledTimerWithTimeInterval:kDebounceInterval
                                                   target:self
                                                 selector:@selector(enqueueScript)
                                                 userInfo:nil
                                                  repeats:NO];
}

- (void)enqueueScript {
  self.debounce = nil;
  self.queuedScript = self.editorView.text;
}

- (void)loadQueuedScript {
  if (self.queuedScript == nil) return;
  const char *newScript =
      [self.queuedScript cStringUsingEncoding:NSUTF8StringEncoding];
  self.queuedScript = nil;
  lua_State *L = self.engineVC.engine->scripting->L;
  int error = luaL_loadstring(self.engineVC.engine->scripting->L, newScript);
  if (error != 0) {
    NSString *errMsg = nil;
    NSNumber *errorLine = [self extractErrorLine:lua_tostring(L, -1)
                                         message:&errMsg];
    [self.editorView setErrorLine:errorLine withMessage:errMsg];
    //NSLog(@"Error running script:\n    %@\n", errMsg);
    lua_pop(L, 1);
    return;
  }
  
  int result = lua_pcall(L, 0, 0, 0);
  if (result != 0) {
    NSString *errMsg = nil;
    NSNumber *errorLine = [self extractErrorLine:lua_tostring(L, -1)
                                         message:&errMsg];
    [self.editorView setErrorLine:errorLine withMessage:errMsg];
    //NSLog(@"Error running script:\n    %@\n", errMsg);
    lua_pop(L, 1);
    return;
  }
  
  [self.editorView setErrorLine:nil withMessage:nil];
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

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
