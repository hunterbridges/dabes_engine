//
//  ScriptEditorViewController.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 5/31/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "CodeEditorView.h"
#import "ScriptEditorViewController.h"
#import "util.h"
#import "engine.h"
#import "scripting.h"

@interface ScriptEditorViewController () <CodeEditorViewDelegate>

@property (nonatomic, strong) CodeEditorView *editorView;
@property (nonatomic, assign) Engine *engine;

@end

@implementation ScriptEditorViewController

- (id)initWithEngine:(Engine *)engine {
  self = [super init];
  if (self) {
    _engine = engine;
  }
  return self;
}

- (void)viewDidLoad
{
  [super viewDidLoad];
	// Do any additional setup after loading the view.
  self.editorView = [[CodeEditorView alloc] initWithFrame:self.view.bounds];
  self.editorView.autoresizingMask = (UIViewAutoresizingFlexibleWidth |
                                    UIViewAutoresizingFlexibleHeight);
  self.editorView.delegate = self;
  [self.view addSubview:self.editorView];
  
  [self loadScript];
}

- (void)loadScript {
  char *script;
  int size;
  int ret = read_text_file("media/scripts/boxfall/entities/megaman.lua",
                           &script, &size);
  if (ret) {
    NSString *str = [[NSString alloc] initWithCString:script
                                             encoding:NSUTF8StringEncoding];
    self.editorView.text = str;
  }
}

- (void)codeEditorDidChange:(CodeEditorView *)codeEditor {
  if (codeEditor.text == nil) return;
  const char *newScript =
      [codeEditor.text cStringUsingEncoding:NSUTF8StringEncoding];
  lua_State *L = _engine->scripting->L;
  int error = luaL_loadstring(_engine->scripting->L, newScript);
  if (error != 0) {
    NSString *errMsg = nil;
    self.editorView.errorLine = [self extractErrorLine:lua_tostring(L, -1)
                                               message:&errMsg];
    //NSLog(@"Error running script:\n    %@\n", errMsg);
    lua_pop(L, 1);
    return;
  }
  
  int result = lua_pcall(L, 0, 0, 0);
  if (result != 0) {
    NSString *errMsg = nil;
    self.editorView.errorLine = [self extractErrorLine:lua_tostring(L, -1)
                                               message:&errMsg];
    //NSLog(@"Error running script:\n    %@\n", errMsg);
    lua_pop(L, 1);
    return;
  }
  
  self.editorView.errorLine = nil;
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
