#include "mac_darkmode.h"
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

bool MacIsDarkMode()
{
    NSString *appearanceName = [[NSAppearance currentAppearance] name];
	return [appearanceName containsString:@"Dark"];
}
