#import "MainViewController.h"
#import "Config.h"
#import "GPURenderer.h"
#import "Renderer.h"
#import "RenderingView.h"

@implementation MainViewController {
    RenderingView* _view;
    id<Renderer> _renderer;
}

- (void)loadView {
    self.view = [[RenderingView alloc] initWithFrame:NSMakeRect(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT)];
}

- (void)viewDidLoad {
    [super viewDidLoad];

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device) {
        NSLog(@"Metal is not supported on this device");
        self.view = [[NSView alloc] initWithFrame:self.view.frame];
        return;
    }

    _view = (RenderingView*)self.view;
    _view.device = device;
    _renderer = [[GPURenderer alloc] initWithMetalKitView:_view];
    _view.delegate = _renderer;
}

@end
