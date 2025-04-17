#include "mac_helpers.h"
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#include <QWidget>

bool MacIsDarkMode()
{
    NSString *appearanceName = [[NSAppearance currentAppearance] name];
	return [appearanceName containsString:@"Dark"];
}

void DisableMacFullscreen(QWidget* widget) {
    //widget->winId(); // ensure NSWindow exists
    //NSView *nsView = (NSView *)widget->window()->winId();
    //NSWindow *nsWindow = [nsView window]; // 💡 this is the real NSWindow
    //NSUInteger behavior = [nsWindow collectionBehavior];
    //behavior &= ~NSWindowCollectionBehaviorFullScreenPrimary;
    //[nsWindow setCollectionBehavior:behavior];

    //NSView *nsView = (NSView *)widget->window()->winId();
    //NSWindow *nsWindow = [nsView window];
    //if (!nsWindow) return;

    //NSUInteger behavior = [nsWindow collectionBehavior];
    //behavior &= ~NSWindowCollectionBehaviorFullScreenPrimary;
    //[nsWindow setCollectionBehavior:behavior];

    // Ensure native view exists
    widget->winId();

    NSView *nsView = (NSView *)widget->window()->winId();
    if (![nsView isKindOfClass:[NSView class]]) return;

    NSWindow *nsWindow = [nsView window];
    if (!nsWindow) return;

    // ✅ Remove fullscreen behavior
    NSUInteger behavior = [nsWindow collectionBehavior];
    behavior &= ~NSWindowCollectionBehaviorFullScreenPrimary;
    [nsWindow setCollectionBehavior:behavior];

    // ✅ Keep resizable, but disable fullscreen by overriding standardWindowButton
    NSButton *zoomButton = [nsWindow standardWindowButton:NSWindowZoomButton];
    [zoomButton setTarget:nil];
    [zoomButton setAction:nil];
}
