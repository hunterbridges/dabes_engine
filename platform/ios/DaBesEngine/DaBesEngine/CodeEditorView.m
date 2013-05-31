//
//  CodeEditorTextView.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 5/31/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import "CodeEditorView.h"

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

@property (nonatomic, assign) CGFloat eightyCols;
@property (nonatomic, strong) CodeEditorTextView *textView;
@property (nonatomic, strong) UIScrollView *scrollView;
@property (nonatomic, strong) CADisplayLink *displayLink;

@end

@implementation CodeEditorView

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
      self.scrollView = [[UIScrollView alloc] init];
      self.scrollView.delegate = self;
      self.scrollView.backgroundColor = [UIColor whiteColor];
      self.scrollView.autoresizingMask = (UIViewAutoresizingFlexibleWidth |
                                          UIViewAutoresizingFlexibleHeight);
      self.scrollView.autoresizesSubviews = NO;
      [self addSubview:self.scrollView];
      
      self.textView = [[CodeEditorTextView alloc] init];
      self.textView.delegate = self;
      self.textView.font = [UIFont fontWithName:@"Inconsolata" size:14];
      self.textView.delegate = self;
      self.textView.autocorrectionType = UITextAutocorrectionTypeNo;
      self.textView.autocapitalizationType = UITextAutocapitalizationTypeNone;
  
      [self.scrollView addSubview:self.textView];
    }
    return self;
}

- (void)layoutSubviews {
  [super layoutSubviews];
  self.scrollView.frame = self.bounds;
  
  NSMutableString *eighty = [[NSMutableString alloc] init];
  for (int i = 0; i < 80; i++) {
    [eighty appendString:@"*"];
  }
  self.eightyCols = [eighty sizeWithFont:self.textView.font].width + 1;
  CGSize size = [self.textView sizeThatFits:CGSizeMake(_eightyCols, CGFLOAT_MAX)];
  CGRect newFrame = {0, 0, size};
  self.textView.frame = newFrame;
  self.scrollView.contentSize = self.textView.bounds.size;
}

#pragma mark - Properties
- (void)setSyntaxValid:(BOOL)valid {
  _syntaxValid = valid;
  if (valid) {
    self.textView.backgroundColor = [UIColor whiteColor];
  } else {
    self.textView.backgroundColor = [UIColor colorWithRed:1.0 green:0.9 blue:0.9 alpha:1.0];
  }
}

- (NSString *)text {
  return self.textView.text;
}

- (void)setText:(NSString *)text {
  self.textView.text = text;
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
  /*
  NSArray *rects = [textView selectionRectsForRange:textView.selectedTextRange];
  for (UITextSelectionRect *rect in rects) {
    UIView *v = [[UIView alloc] initWithFrame:rect.rect];
    v.backgroundColor = [UIColor colorWithRed:1.0 green:0 blue:0 alpha:0.5];
    [self.textView addSubview:v];
    [v performSelector:@selector(removeFromSuperview)
            withObject:nil
            afterDelay:2.0];
  }
   */
}

- (void)textViewDidChange:(UITextView *)textView {
  [self.delegate codeEditorDidChange:self];
}


@end
