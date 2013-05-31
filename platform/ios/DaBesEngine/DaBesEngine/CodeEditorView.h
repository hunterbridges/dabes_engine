//
//  CodeEditorTextView.h
//  DaBesEngine
//
//  Created by Hunter Bridges on 5/31/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <UIKit/UIKit.h>

@class CodeEditorView;
@protocol CodeEditorViewDelegate <NSObject>

- (void)codeEditorDidChange:(CodeEditorView *)codeEditor;

@end

@interface CodeEditorView : UIView <UITextViewDelegate, UIScrollViewDelegate>

@property (nonatomic, assign) BOOL syntaxValid;
@property (nonatomic, weak) id<CodeEditorViewDelegate> delegate;
@property (nonatomic, copy) NSString *text;
@property (nonatomic, weak) id linkTo;

@end
