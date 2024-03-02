//
//  AppDelegate.m
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-03.
//

#import "AppDelegate.h"
#import "game.hpp"
#import "graphics.hpp"
#import "system_host.hpp"

#import <Foundation/Foundation.h>

@interface AppDelegate ()
@property (strong) IBOutlet NSWindow *window;
@property (strong) IBOutlet CGGameView *gameView;
@end

@implementation CGGameView {
    NSTimer *_vblTimer;
    cgpoint_t _mouse;
    bool _left;
    bool _right;
    NSTrackingRectTag _trackingRect;
}

static NSConditionLock *_gameLock;

static void _yieldFunction() {
    [_gameLock lockWhenCondition:0];
    [_gameLock unlockWithCondition:1];
    usleep(100);
    [_gameLock lockWhenCondition:1];
    [_gameLock unlockWithCondition:0];
}

- (void)_cg_commonInit {
    _gameLock = [[NSConditionLock alloc] initWithCondition:0];
    cgg_yield_function = &_yieldFunction;
    _vblTimer = [NSTimer timerWithTimeInterval:1.0 / 50 target:self selector:@selector(fireVBLTimer:) userInfo:nil repeats:YES];
    [[NSRunLoop currentRunLoop] addTimer:_vblTimer forMode:NSDefaultRunLoopMode];
    NSString *path = [[NSBundle mainBundle] resourcePath];
    [[NSFileManager defaultManager] changeCurrentDirectoryPath:path];
    dispatch_async(dispatch_queue_create("game_queue", NULL), ^{
        cgmanager_c manager;
        auto root_scene = new cgroot_scene_c(manager);
        manager.run(root_scene);
    });
}

- (void)viewDidMoveToWindow {
    _trackingRect = [self addTrackingRect:[self bounds] owner:self userData:NULL assumeInside:NO];
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)fireVBLTimer:(NSTimer *)timer {
    cgg_vbl_interupt();
    [self setNeedsDisplay:YES];
}

- (instancetype)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];
    if (self) {
        [self _cg_commonInit];
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if (self) {
        [self _cg_commonInit];
    }
    return self;
}

- (void)_updateMouse {
    cgg_update_mouse(_mouse, _left, _right);
}

- (void)mouseMoved:(NSEvent *)event {
    NSPoint point = [self convertPoint:event.locationInWindow fromView:nil];
    _mouse.x = (int16_t)(point.x / 2);
    _mouse.y = 199 - (int16_t)(point.y / 2);
    [self _updateMouse];
}

- (void)mouseDown:(NSEvent *)event {
    _left = true;
    [self _updateMouse];
}

- (void)mouseUp:(NSEvent *)event {
    _left = false;
    [self _updateMouse];
}

- (void)rightMouseDown:(NSEvent *)event {
    _right = true;
    [self _updateMouse];
}

- (void)rightMouseUp:(NSEvent *)event {
    _right = false;
    [self _updateMouse];
}

- (void)drawRect:(NSRect)dirtyRect {
    if (cgg_active_image == NULL) {
        NSRect bounds = [self bounds];
        [[NSColor blackColor] set];
        [NSBezierPath fillRect:bounds];
        return;
    }
    [_gameLock lockWhenCondition:1];
    const auto size = cgg_active_image->get_size();
    const auto offset = cgg_active_image->get_offset();
    typedef struct { uint8_t rgb[3]; uint8_t _; } color_t;
    color_t palette[16] = { 0 };
    color_t buffer[320 * 200];
    memset(buffer, 0, sizeof(color_t) * 320 * 200);

    if (cgg_active_palette) {
        for (int i = 0; i < 16; i++) {
            cgg_active_palette->colors[i].get(&palette[i].rgb[0], &palette[i].rgb[1], &palette[i].rgb[2]);
            buffer[i]._ = 0;
        }
    }
    const_cast<cgimage_c*>(cgg_active_image)->with_clipping(false, [self, size, offset, &palette, &buffer] () {
        cgpoint_t at;
        for (at.y = 0; at.y < 200; at.y++) {
            for (at.x = 0; at.x < 320; at.x++) {
                const auto real_at = (cgpoint_t){ static_cast<int16_t>(at.x + offset.x), static_cast<int16_t>(at.y + offset.y) };
                const auto c = cgg_active_image->get_pixel(real_at);
                if (c != cgimage_c::MASKED_CIDX) {
                    auto offset = (at.y * size.width + at.x);
                    buffer[offset] = palette[c];
                }
            }
        }
    });
    
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate((void *)buffer, 320, 200, 8, size.width * 4, space, kCGImageAlphaNoneSkipLast);
    CGColorSpaceRelease(space);
    CGImageRef image = CGBitmapContextCreateImage(ctx);
    CGContextRelease(ctx);
    
    ctx = NSGraphicsContext.currentContext.CGContext;
    CGContextDrawImage(ctx, self.bounds, image);
    
    CGImageRelease(image);

    [_gameLock unlockWithCondition:1];
}

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    self.window.acceptsMouseMovedEvents = YES;
}

@end
