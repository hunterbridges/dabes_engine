//
//  CodeEditorTextView.h
//  DaBesEngine
//
//  Created by Hunter Bridges on 5/31/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <UIKit/UIKit.h>

@class CodeEditorView;
@class CodeEditorTextView;
@protocol CodeEditorViewDelegate <NSObject>

- (void)codeEditorDidChange:(CodeEditorView *)codeEditor;

@end

@interface CodeEditorView : UIView <UITextViewDelegate, UIScrollViewDelegate>

@property (nonatomic, copy, readonly) NSNumber *errorLine;
@property (nonatomic, weak) id<CodeEditorViewDelegate> delegate;
@property (nonatomic, copy) NSString *text;
@property (nonatomic, weak) id linkTo;
@property (nonatomic, readonly) UITextView *textView;
@property (nonatomic, readonly) UIScrollView *scrollView;
@property (nonatomic, assign) BOOL highlightSyntax;

- (void)setErrorLine:(NSNumber *)errorLine
         withMessage:(NSString *)errorMessage;
  @end
