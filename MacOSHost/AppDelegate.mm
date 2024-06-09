//
//  AppDelegate.m
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-03.
//

#import "AppDelegate.h"
#import "game.hpp"
#import "image.hpp"
#import "host_bridge.hpp"
#include "machine.hpp"

#import <Foundation/Foundation.h>

class cg_host_bridge_c : public host_bridge_c {
public:
    cg_host_bridge_c() : _yield_lock([[NSConditionLock alloc] initWithCondition:0]), _timer_lock([[NSRecursiveLock alloc] init]) {}
    
    void yield() override {
        [_yield_lock lockWhenCondition:0];
        [_yield_lock unlockWithCondition:1];
        usleep(100);
        [_yield_lock lockWhenCondition:1];
        [_yield_lock unlockWithCondition:0];
    }
    
    void pause_timers() override {
        [_timer_lock lock];
    }
    
    void resume_timers() override {
        [_timer_lock unlock];
    }
    
    template<class Commands>
    void with_paused_timers(Commands commands) {
        pause_timers();
        commands();
        resume_timers();
    }
    
    void play(const sound_c &sound) override {
        NSData *data = [NSData dataWithBytesNoCopy:(void *)sound.sample() length:sound.length() freeWhenDone:NO];
        NSSound *snd = [[NSSound alloc] initWithData:data];
        [snd play];
    }
    
private:
    NSConditionLock *_yield_lock;
    NSRecursiveLock *_timer_lock;
};

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
    cg_host_bridge_c _bridge;
    bool _saveScreenshot;
}

static NSConditionLock *_gameLock;

static void _yieldFunction() {
}

- (void)_cg_commonInit {
    host_bridge_c::set_shared(_bridge);
    _vblTimer = [NSTimer timerWithTimeInterval:1.0 / 50 target:self selector:@selector(fireVBLTimer:) userInfo:nil repeats:YES];
    [[NSRunLoop currentRunLoop] addTimer:_vblTimer forMode:NSDefaultRunLoopMode];
    _timerCTimer = [NSTimer timerWithTimeInterval:1.0 / 200 target:self selector:@selector(fireTimerCTimer:) userInfo:nil repeats:YES];
    [[NSRunLoop currentRunLoop] addTimer:_timerCTimer forMode:NSDefaultRunLoopMode];
    NSString *path = [[NSBundle mainBundle] resourcePath];
    [[NSFileManager defaultManager] changeCurrentDirectoryPath:path];
    dispatch_async(dispatch_queue_create("game_queue", NULL), ^{
        {
            auto &m = machine_c::shared();
            printf("Type %d\n\r", m.type());
            printf("Type %zu\n\r", m.max_memory());
            printf("Type %zu\n\r", m.user_memory());
            asset_manager_c::set_shared(new cgasset_manager());
            scene_manager_c manager(size_s(320, 208));
            auto intro_scene = new cgintro_scene_c(manager);
            manager.run(intro_scene, nullptr, transition_c::create(canvas_c::noise));
        }
        dispatch_async(dispatch_get_main_queue(), ^{
            [NSApp terminate:nil];
        });
    });
}

- (void)print:(id)sender {
    _saveScreenshot = true;
}

- (void)viewDidMoveToWindow {
    _trackingRect = [self addTrackingRect:[self bounds] owner:self userData:NULL assumeInside:NO];
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)fireVBLTimer:(NSTimer *)timer {
    _bridge.with_paused_timers([self]{
        _bridge.vbl_interupt();
    });
    [self setNeedsDisplay:YES];
}

- (void)fireTimerCTimer:(NSTimer *)timer {
    _bridge.with_paused_timers([self]{
        _bridge.clock_interupt();
    });
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
    _bridge.with_paused_timers([self]{
        _bridge.update_mouse(_mouse, _left, _right);
    });
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

static BOOL _savePNGImage(CGImageRef image, NSString *path) {
    CFURLRef url = (__bridge CFURLRef)[NSURL fileURLWithPath:path];
    CGImageDestinationRef destination = CGImageDestinationCreateWithURL(url, kUTTypePNG, 1, NULL);
    if (!destination) {
        return NO;
    }
    CGImageDestinationAddImage(destination, image, nil);
    if (!CGImageDestinationFinalize(destination)) {
        CFRelease(destination);
        return NO;
    }
    CFRelease(destination);
    return YES;
}

- (void)drawRect:(NSRect)dirtyRect {
    _bridge.with_paused_timers([self]{
        auto active_image = machine_c::shared().active_image();
        auto active_palette = machine_c::shared().active_palette();
        
        if (active_image == NULL) {
            NSRect bounds = [self bounds];
            [[NSColor blackColor] set];
            [NSBezierPath fillRect:bounds];
            return;
        }
        [_gameLock lockWhenCondition:1];
        const auto size = active_image->size();
        typedef struct { uint8_t rgb[3]; uint8_t _; } color_s;
        color_s palette[16] = { 0 };
        color_s buffer[320 * 200];
        memset(buffer, 0, sizeof(color_s) * 320 * 200);
        
        if (active_palette) {
            for (int i = 0; i < 16; i++) {
                active_palette->colors[i].get(&palette[i].rgb[0], &palette[i].rgb[1], &palette[i].rgb[2]);
                buffer[i]._ = 0;
            }
        }
        point_s at;
        for (at.y = 0; at.y < 200; at.y++) {
            for (at.x = 0; at.x < 320; at.x++) {
                const auto c = active_image->get_pixel(at);
                if (c != image_c::MASKED_CIDX) {
                    auto offset = (at.y * size.width + at.x);
                    buffer[offset] = palette[c];
                }
            }
        }
        
        CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
        CGContextRef ctx = CGBitmapContextCreate((void *)buffer, 320, 200, 8, size.width * 4, space, kCGImageAlphaNoneSkipLast);
        CGColorSpaceRelease(space);
        CGImageRef image = CGBitmapContextCreateImage(ctx);
        CGContextRelease(ctx);
        
        ctx = NSGraphicsContext.currentContext.CGContext;
        CGContextDrawImage(ctx, self.bounds, image);
        
        if (_saveScreenshot) {
            static NSUInteger number = 0;
            _saveScreenshot = false;
            _savePNGImage(image, [NSString stringWithFormat:@"/tmp/screenshot-%02lu.png", (unsigned long)++number]);
        }
        
        CGImageRelease(image);
        
        [_gameLock unlockWithCondition:1];
    });
}

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    self.window.acceptsMouseMovedEvents = YES;
}

@end
