#import "RenderingView.h"

#import "RCBridge.h"

@implementation RenderingView {
    BOOL _isMouseCaptured;
    NSTrackingArea* _trackingArea;
}

#pragma mark - Initialization

- (instancetype)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];
    return self;
}

- (instancetype)initWithCoder:(NSCoder*)coder {
    self = [super initWithCoder:coder];
    return self;
}

#pragma mark - Responder Chain

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)viewDidMoveToWindow {
    [super viewDidMoveToWindow];

    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:NSWindowDidResignKeyNotification
                                                  object:self.window];

    if (self.window) {
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(windowDidResignKey:)
                                                     name:NSWindowDidResignKeyNotification
                                                   object:self.window];
    }

    [self.window makeFirstResponder:self];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [self stopCapture];
}

#pragma mark - Tracking Area

- (void)updateTrackingAreas {
    [super updateTrackingAreas];

    if (_trackingArea) {
        [self removeTrackingArea:_trackingArea];
    }

    // We want to track mouse movement, and enter/exit events.
    // activeInKeyWindow means events are only sent when the window is active.
    NSTrackingAreaOptions options = (NSTrackingMouseMoved | NSTrackingActiveInKeyWindow | NSTrackingMouseEnteredAndExited);

    _trackingArea = [[NSTrackingArea alloc] initWithRect:self.bounds
                                                 options:options
                                                   owner:self
                                                userInfo:nil];

    [self addTrackingArea:_trackingArea];
}

#pragma mark - Mouse Capture Logic

- (void)startMouseCapture {
    if (!_isMouseCaptured) {
        _isMouseCaptured = YES;
        [NSCursor hide];
        CGAssociateMouseAndMouseCursorPosition(false);
    }
}

- (void)stopCapture {
    if (_isMouseCaptured) {
        _isMouseCaptured = NO;
        [NSCursor unhide];
        CGAssociateMouseAndMouseCursorPosition(true);
    }
}

- (void)windowDidResignKey:(NSNotification*)notification {
    [self stopCapture];
}

#pragma mark - Event Handling

- (void)keyDown:(NSEvent*)event {
    if (event.keyCode == 53) { // Escape key
        [self stopCapture];
    } else {
        [RCBridge keyDown:event.keyCode];
    }
}

- (void)keyUp:(NSEvent*)event {
    [RCBridge keyUp:event.keyCode];
}

- (void)flagsChanged:(NSEvent*)event {
    [RCBridge flagsChanged:event.modifierFlags];
}

- (void)mouseDown:(NSEvent*)event {
    [self startMouseCapture];
    [RCBridge mouseDown];
}

- (void)mouseUp:(NSEvent*)event {
    [RCBridge mouseUp];
}

- (void)rightMouseDown:(NSEvent*)event {
    [self startMouseCapture];
    [RCBridge rightMouseUp];
}

- (void)rightMouseUp:(NSEvent*)event {
    [RCBridge rightMouseUp];
}

- (void)mouseMoved:(NSEvent*)event {
    if (!_isMouseCaptured) return;
    [RCBridge mouseMoved:CGVectorMake(event.deltaX, event.deltaY)];
}

- (void)mouseDragged:(NSEvent*)event {
    if (!_isMouseCaptured) return;
    [RCBridge mouseMoved:CGVectorMake(event.deltaX, event.deltaY)];
}

- (void)scrollWheel:(NSEvent*)event {
    if (!_isMouseCaptured) return;
    [RCBridge mouseWheel:CGVectorMake(event.scrollingDeltaX, event.scrollingDeltaY)];
}

@end
