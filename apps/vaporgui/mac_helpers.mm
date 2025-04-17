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
    // Ensure native view exists
    widget->winId();

    NSView *nsView = (NSView *)widget->window()->winId();
    if (![nsView isKindOfClass:[NSView class]]) return;

    NSWindow *nsWindow = [nsView window];
    if (!nsWindow) return;

    // Remove fullscreen behavior
    NSUInteger behavior = [nsWindow collectionBehavior];
    behavior &= ~NSWindowCollectionBehaviorFullScreenPrimary;
    [nsWindow setCollectionBehavior:behavior];

    // Keep resizable, but disable fullscreen by overriding standardWindowButton
    NSButton *zoomButton = [nsWindow standardWindowButton:NSWindowZoomButton];
    [zoomButton setTarget:nil];
    [zoomButton setAction:nil];
}
