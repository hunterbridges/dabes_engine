#import "ConsoleView.h"

@implementation ConsoleView {
  UILabel *consoleLabel_;
}

- (id)initWithCoder:(NSCoder *)aDecoder {
  self = [super initWithCoder:aDecoder];
  if (self) {
    consoleLabel_ = [[UILabel alloc] init];
    consoleLabel_.autoresizingMask = (UIViewAutoresizingFlexibleHeight |
                                      UIViewAutoresizingFlexibleWidth);
    consoleLabel_.font = [UIFont fontWithName:@"Courier New" size:12];
    consoleLabel_.textColor = [UIColor whiteColor];
    consoleLabel_.textAlignment = NSTextAlignmentLeft;
    consoleLabel_.backgroundColor = [UIColor clearColor];
    consoleLabel_.numberOfLines = 0;
    [self addSubview:consoleLabel_];
    self.backgroundColor = [UIColor colorWithWhite:0 alpha:0.5];
    [self logText:@"Console\n-------"];
  }
  return self;
}

- (void)layoutSubviews {
  [consoleLabel_ sizeToFit];
  
  CGRect frame = CGRectMake(10,
                            self.frame.size.height - 10 - 
                                consoleLabel_.frame.size.height,
                            self.frame.size.width - 20,
                            consoleLabel_.frame.size.height);
  consoleLabel_.frame = frame;
}

- (void)logText:(NSString *)text {
  NSMutableString *mutable = [NSMutableString string];
  if (consoleLabel_.text) {
    [mutable appendFormat:@"%@\n", consoleLabel_.text];
  }
  [mutable appendFormat:@"%@", text];
  consoleLabel_.text = mutable;
  [self layoutSubviews];
}

@end
