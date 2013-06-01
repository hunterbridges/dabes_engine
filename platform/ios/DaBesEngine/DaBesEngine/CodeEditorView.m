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
@property (nonatomic, strong) LuaSyntaxHighlighter *highlighter;
@property (nonatomic, strong) CodeEditorTextView *textView;
@property (nonatomic, strong) UIScrollView *scrollView;
@property (nonatomic, strong) CADisplayLink *displayLink;
@property (nonatomic, strong) NSMutableArray *errorHighlightViews;

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
      
      self.highlighter = [[LuaSyntaxHighlighter alloc] init];
  
      [self.scrollView addSubview:self.textView];
    }
    return self;
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

- (void)setErrorLine:(NSNumber *)errorLine {
  _errorLine = [errorLine copy];
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
    for (UITextSelectionRect *rect in selRects) {
      CGRect frame = rect.rect;
      frame.origin.x = 0;
      frame.size.width = self.eightyCols.width;
      UIView *highlight = [[UIView alloc] initWithFrame:frame];
      highlight.backgroundColor = [UIColor codeBackgroundErrorColor];
      [self.scrollView addSubview:highlight];
      [self.errorHighlightViews addObject:highlight];
    }
    [self.scrollView bringSubviewToFront:self.textView];
  } else {
    if (self.errorHighlightViews) {
      [self.errorHighlightViews makeObjectsPerformSelector:@selector(removeFromSuperview)];
      self.errorHighlightViews = nil;
    }
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
  return self.highlighter.text;
}

- (void)setText:(NSString *)text {
  self.highlighter.text = text;
  self.textView.textColor = [UIColor codeDefaultColor];
  self.textView.attributedText = self.highlighter.attributedText;
}


#pragma mark - UIScrollViewDelegate

- (void)scrollViewWillBeginDragging:(UIScrollView *)scrollView
{
  [self startDisplayLinkIfNeeded];
}

- (void)scrollViewDidEndDragging:(UIScrollView *)scrollView willDecelerate:(BOOL)decelerate
{
  if (!decelerate) {
    [self stopDisplayLink];
  }
}

- (void)scrollViewDidEndDecelerating:(UIScrollView *)scrollView
{
  [self stopDisplayLink];
}
#pragma mark Display Link

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
  [self startDisplayLinkIfNeeded];
  [self.highlighter replaceThenRehighlightCharactersInRange:range
                                                 withString:text];
  return YES;
}

- (void)textViewDidChange:(UITextView *)textView {
  NSRange range = textView.selectedRange;
  CGPoint offset = self.scrollView.contentOffset;
  textView.attributedText = self.highlighter.attributedText;
  textView.selectedRange = range;
  [self.scrollView setContentOffset:offset animated:NO];
  [self.delegate codeEditorDidChange:self];
  [self stopDisplayLink];
}


@end
