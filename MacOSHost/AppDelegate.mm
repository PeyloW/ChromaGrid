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
    NSTimer *_timerCTimer;
    point_s _mouse;
    bool _left;
    bool _right;
    NSTrackingRectTag _trackingRect;
    NSSound *_sound;
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
    g_yield_function = &_yieldFunction;
    _vblTimer = [NSTimer timerWithTimeInterval:1.0 / 50 target:self selector:@selector(fireVBLTimer:) userInfo:nil repeats:YES];
    [[NSRunLoop currentRunLoop] addTimer:_vblTimer forMode:NSDefaultRunLoopMode];
    _timerCTimer = [NSTimer timerWithTimeInterval:1.0 / 200 target:self selector:@selector(fireTimerCTimer:) userInfo:nil repeats:YES];
    [[NSRunLoop currentRunLoop] addTimer:_timerCTimer forMode:NSDefaultRunLoopMode];
    NSString *path = [[NSBundle mainBundle] resourcePath];
    [[NSFileManager defaultManager] changeCurrentDirectoryPath:path];
    dispatch_async(dispatch_queue_create("game_queue", NULL), ^{
        {
            scene_manager_c manager;
            auto intro_scene = new cgintro_scene_c(manager);
            auto overlay_scene = new cgoverlay_scene_c(manager);
            manager.run(intro_scene, overlay_scene);
        }
        dispatch_async(dispatch_get_main_queue(), ^{
            [NSApp terminate:nil];
        });
    });
}

- (void)viewDidMoveToWindow {
    _trackingRect = [self addTrackingRect:[self bounds] owner:self userData:NULL assumeInside:NO];
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)fireVBLTimer:(NSTimer *)timer {
    g_vbl_interupt();
    [self setNeedsDisplay:YES];
    if (g_active_sound) {
        NSData *data = [NSData dataWithBytesNoCopy:(void *)g_active_sound->get_sample() length:g_active_sound->get_length() freeWhenDone:NO];
        _sound = [[NSSound alloc] initWithData:data];
        [_sound play];
        g_active_sound = nullptr;
    }
}

- (void)fireTimerCTimer:(NSTimer *)timer {
    g_clock_interupt();
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
    g_update_mouse(_mouse, _left, _right);
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
    if (g_active_image == NULL) {
        NSRect bounds = [self bounds];
        [[NSColor blackColor] set];
        [NSBezierPath fillRect:bounds];
        return;
    }
    [_gameLock lockWhenCondition:1];
    const auto size = g_active_image->get_size();
    const auto offset = g_active_image->get_offset();
    typedef struct { uint8_t rgb[3]; uint8_t _; } color_s;
    color_s palette[16] = { 0 };
    color_s buffer[320 * 200];
    memset(buffer, 0, sizeof(color_s) * 320 * 200);

    if (g_active_palette) {
        for (int i = 0; i < 16; i++) {
            g_active_palette->colors[i].get(&palette[i].rgb[0], &palette[i].rgb[1], &palette[i].rgb[2]);
            buffer[i]._ = 0;
        }
    }
    const_cast<image_c*>(g_active_image)->with_clipping(false, [self, size, offset, &palette, &buffer] () {
        point_s at;
        for (at.y = 0; at.y < 200; at.y++) {
            for (at.x = 0; at.x < 320; at.x++) {
                const auto real_at = (point_s){ static_cast<int16_t>(at.x + offset.x), static_cast<int16_t>(at.y + offset.y) };
                const auto c = g_active_image->get_pixel(real_at);
                if (c != image_c::MASKED_CIDX) {
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
