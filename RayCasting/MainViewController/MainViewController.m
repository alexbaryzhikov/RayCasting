#import "MainViewController.h"

#import "Config.h"
#import "Renderer.h"
#import "RenderingView.h"

@implementation MainViewController {
    RenderingView* _view;
    Renderer* _renderer;
}

- (void)loadView {
    self.view = [[RenderingView alloc] initWithFrame:NSMakeRect(0, 0, VIEW_WIDTH, VIEW_HEIGHT)];
}

- (void)viewDidLoad {
    [super viewDidLoad];

    _view = (RenderingView*)self.view;
    _view.device = MTLCreateSystemDefaultDevice();

    if (!_view.device) {
        NSLog(@"Metal is not supported on this device");
        self.view = [[NSView alloc] initWithFrame:self.view.frame];
        return;
    }

    _renderer = [[Renderer alloc] initWithMetalKitView:_view];
    [_renderer mtkView:_view drawableSizeWillChange:_view.drawableSize];
    _view.delegate = _renderer;
}

@end
