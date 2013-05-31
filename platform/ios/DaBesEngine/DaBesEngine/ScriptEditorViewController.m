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
  const char *newScript =
      [codeEditor.text cStringUsingEncoding:NSUTF8StringEncoding];
  lua_State *L = _engine->scripting->L;
  int error = luaL_loadstring(_engine->scripting->L, newScript);
  if (error != 0) {
    self.editorView.syntaxValid = NO;
    printf("Error loading script:\n    %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
    return;
  }
  
  int result = lua_pcall(L, 0, 0, 0);
  if (result != 0) {
    self.editorView.syntaxValid = NO;
    printf("Error running script:\n    %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
    return;
  }
  
  self.editorView.syntaxValid = YES;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
