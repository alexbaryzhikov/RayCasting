#import "MainViewController.h"
#import "CPURenderer.h"
#import "Config.h"
#import "GPURenderer.h"
#import "Renderer.h"
#import "RenderingView.h"

@implementation MainViewController {
    RenderingView* _view;
    id<Renderer> _renderer;
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

    if (GPU_RENDERING) {
        _renderer = [[GPURenderer alloc] initWithMetalKitView:_view];
    } else {
        _renderer = [[CPURenderer alloc] initWithMetalKitView:_view];
    }

    [_renderer mtkView:_view drawableSizeWillChange:_view.drawableSize];
    _view.delegate = _renderer;
}

@end
