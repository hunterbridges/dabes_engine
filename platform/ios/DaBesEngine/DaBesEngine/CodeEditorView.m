//
//  CodeEditorTextView.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 5/31/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import "CodeEditorView.h"
#import "UIColor+LuaHighlighting.h"
#import "LuaSyntaxHighlighter.h"
#import "LuaSyntaxStyler.h"

@interface CodeEditorTextView : UITextView

@end

@implementation CodeEditorTextView

- (BOOL)canPerformAction:(SEL)action withSender:(id)sender {
  if (action == @selector(cut:)) return NO;
  if (action == @selector(copy:)) return NO;
  if (action == @selector(paste:)) return NO;
  if (action == @selector(delete:)) return NO;
  if (action == @selector(_define:)) return NO;
  
  return [super canPerformAction:action withSender:sender];
}

@end

////////////////////////////////////////////////////////////////////////////////

@interface CodeEditorView ()

@property (nonatomic, assign) CGSize eightyCols;
@property (nonatomic, strong) LuaSyntaxStyler *styler;
@property (nonatomic, strong) LuaSyntaxHighlighter *highlighter;
@property (nonatomic, strong) CodeEditorTextView *textView;
@property (nonatomic, strong) UIScrollView *scrollView;
@property (nonatomic, strong) CADisplayLink *displayLink;
@property (nonatomic, strong) NSMutableArray *errorHighlightViews;
@property (nonatomic, assign) NSInteger indentJump;
@property (nonatomic, copy) NSNumber *errorLine;
@property (nonatomic, copy) NSString *errorMessage;
@property (nonatomic, strong) UIButton *errorButton;

@end

@implementation CodeEditorView

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
      self.scrollView = [[UIScrollView alloc] init];
      self.scrollView.delegate = self;
      self.scrollView.backgroundColor = [UIColor codeBackgroundColor];
      self.scrollView.autoresizingMask = (UIViewAutoresizingFlexibleWidth |
                                          UIViewAutoresizingFlexibleHeight);
      self.scrollView.autoresizesSubviews = NO;
      [self addSubview:self.scrollView];
      
      self.textView = [[CodeEditorTextView alloc] init];
      self.textView.delegate = self;
      self.textView.font = [UIFont fontWithName:@"Inconsolata" size:14];
      self.textView.textColor = [UIColor codeDefaultColor];
      self.textView.backgroundColor = [UIColor clearColor];
      self.textView.delegate = self;
      self.textView.autocorrectionType = UITextAutocorrectionTypeNo;
      self.textView.autocapitalizationType = UITextAutocapitalizationTypeNone;
      
      NSMutableString *eighty = [[NSMutableString alloc] init];
      for (int i = 0; i < 80; i++) {
        [eighty appendString:@"*"];
      }
      self.eightyCols = [eighty sizeWithFont:self.textView.font];
      
      self.errorLine = nil;
      
      self.highlightSyntax = NO;
      
      self.highlighter = [[LuaSyntaxHighlighter alloc] init];
      self.styler = [[LuaSyntaxStyler alloc] init];
      
      [self.scrollView addSubview:self.textView];
    }
    return self;
}

- (void)viewDidLoad {
  [self startDisplayLinkIfNeeded];
}

- (void)dealloc {
  [self stopDisplayLink];
}

- (void)layoutSubviews {
  [super layoutSubviews];
  self.scrollView.frame = self.bounds;
  
  CGSize size = [self.textView sizeThatFits:CGSizeMake(self.eightyCols.width,
                                                       CGFLOAT_MAX)];
  CGRect newFrame = {0, 0, size};
  self.textView.frame = newFrame;
  self.scrollView.contentSize = self.textView.bounds.size;
}

#pragma mark - Properties

- (void)setErrorLine:(NSNumber *)errorLine
         withMessage:(NSString *)errorMessage {
  _errorLine = [errorLine copy];
  
  if (errorMessage) {
    self.errorMessage = errorMessage;
    if (!self.errorButton) {
      self.errorButton = [UIButton buttonWithType:UIButtonTypeInfoLight];
      self.errorButton.frame = CGRectMake(0, 0, 20, 20);
      [self.errorButton addTarget:self
                           action:@selector(errorButtonPressed:)
                 forControlEvents:UIControlEventTouchUpInside];
      [self.scrollView addSubview:self.errorButton];
    }
  } else {
    [self.errorButton removeFromSuperview];
    self.errorButton = nil;
  }
  
  if (errorLine) {
    if (self.errorHighlightViews) {
      [self.errorHighlightViews makeObjectsPerformSelector:@selector(removeFromSuperview)];
      [self.errorHighlightViews removeAllObjects];
    } else {
      self.errorHighlightViews = [[NSMutableArray alloc] init];
    }
    
    NSRange range = [self rangeForLine:errorLine.intValue];
    UITextRange *oldRange = self.textView.selectedTextRange;
    self.textView.selectedRange = range;
    UITextRange *textRange = self.textView.selectedTextRange;
    self.textView.selectedTextRange = oldRange;
    NSArray *selRects = [self.textView selectionRectsForRange:textRange];
    
    CGFloat minY = CGFLOAT_MAX;
    CGFloat maxY = 0;
    
    for (UITextSelectionRect *rect in selRects) {
      CGRect frame = rect.rect;
      frame.origin.x = 0;
      frame.size.width = self.eightyCols.width;
      minY = MIN(minY, CGRectGetMinY(frame));
      maxY = MAX(maxY, CGRectGetMaxY(frame));
      UIView *highlight = [[UIView alloc] initWithFrame:frame];
      highlight.backgroundColor = [UIColor codeBackgroundErrorColor];
      [self.scrollView addSubview:highlight];
      [self.errorHighlightViews addObject:highlight];
    }
    
    self.errorButton.center = CGPointMake(CGRectGetMaxX(self.bounds) - 20,
                                          (minY + maxY) / 2.0);
    
    [self.scrollView bringSubviewToFront:self.textView];
    [self.scrollView bringSubviewToFront:self.errorButton];
  } else {
    [self.errorHighlightViews makeObjectsPerformSelector:@selector(removeFromSuperview)];
    self.errorHighlightViews = nil;
  }
}

- (NSRange)rangeForLine:(int)line {
  NSRange range = NSMakeRange(0, 0);
  NSCharacterSet *newline = [NSCharacterSet characterSetWithCharactersInString:@"\n"];
  NSScanner *scanner = [NSScanner scannerWithString:self.textView.text];
  scanner.charactersToBeSkipped = nil;
  for (int i = 0; i < line; i++) {
    range.location = scanner.scanLocation;
    NSString *line = nil;
    [scanner scanUpToCharactersFromSet:newline
                            intoString:&line];
    range.length = scanner.scanLocation - range.location;
    [scanner scanString:@"\n" intoString:nil];
  }
  return range;
}

- (NSString *)text {
  return self.styler.text;
}

- (void)setText:(NSString *)text {
  self.styler.text = text;
  self.textView.textColor = [UIColor codeDefaultColor];
  if (self.highlightSyntax) {
    self.highlighter.text = text;
    self.textView.attributedText = self.highlighter.attributedText;
  } else {
    self.textView.text = self.styler.text;
  }
}

- (void)errorButtonPressed:(UIButton *)button {
  UIAlertView *alert =
      [[UIAlertView alloc] initWithTitle:@"Lua Error"
                                 message:self.errorMessage
                                delegate:nil
                       cancelButtonTitle:@"OK"
                       otherButtonTitles:nil];
  [alert show];
}
#pragma mark - Display Link

- (void)scrollViewDidScroll:(UIScrollView *)scrollView {
  if (self.linkTo) {
    [self.linkTo performSelector:@selector(fullUpdate)];
  }
}

- (void)startDisplayLinkIfNeeded
{
  if (!_displayLink && self.linkTo) {
    _displayLink = [CADisplayLink displayLinkWithTarget:self.linkTo
                                               selector:@selector(fullUpdate)];
    [_displayLink addToRunLoop:[NSRunLoop mainRunLoop]
                       forMode:NSDefaultRunLoopMode];
  }
}

- (void)stopDisplayLink
{
  [_displayLink invalidate];
  _displayLink = nil;
}

#pragma mark - UITextViewDelegate

- (void)textViewDidChangeSelection:(UITextView *)textView {
  // TODO: Contextual tools
}

- (BOOL)textView:(UITextView *)textView
    shouldChangeTextInRange:(NSRange)range
            replacementText:(NSString *)text {
  NSRange styledRange;
  NSString *styledString = nil;
  [self.styler replaceThenRestyleCharactersInRange:range
                                        withString:text
                                       styledRange:&styledRange
                                      styledString:&styledString];
  if (self.highlightSyntax) {
    [self.highlighter replaceThenRehighlightCharactersInRange:styledRange
                                                   withString:styledString];
  }
  return YES;
}

- (void)textViewDidChange:(UITextView *)textView {
  NSRange range = textView.selectedRange;
  CGPoint offset = self.scrollView.contentOffset;
  if (self.highlightSyntax) {
    textView.attributedText = self.highlighter.attributedText;
  } else {
    textView.text = self.styler.text;
  }
  range.location += self.styler.lastIndentJump;
  textView.selectedRange = range;
  [self.scrollView setContentOffset:offset animated:NO];
  [self.delegate codeEditorDidChange:self];
}


@end
