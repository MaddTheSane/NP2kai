/*
 *  macalert.cpp
 *  np2
 *
 *  Created by tk800 on Fri Oct 31 2003.
 *
 */

#include "compiler.h"
#include "resource.h"
#include "np2.h"
#include "macalert.h"
#include <Cocoa/Cocoa.h>

static NSInteger showCautionAlert(NSString *title, NSString *string, NSString *button) {
	NSAlert *alert = [[NSAlert alloc] init];
    NSInteger hit;
    
	alert.messageText = title;
	alert.informativeText = string;
	if (button) {
		[alert addButtonWithTitle:button];
	} else {
		[alert addButtonWithTitle:@"OK"];
	}
	[alert addButtonWithTitle:@"Cancel"];

	NSBeep();
    hit = [alert runModal];
	[alert release];
    return(hit);
}

void ResumeErrorDialogProc(void) {
	NSAlert *alert = [[NSAlert alloc] init];
	alert.alertStyle = NSAlertStyleCritical;
	alert.messageText = NSLocalizedString(@"Couldn't restart", "Resume Error Message");
	alert.informativeText = NSLocalizedString(@"An error occured when reading the np2.sav file. Neko Project IIx couldn't restart.", "Resume Error Description");

    NSBeep();
	[alert runModal];
	[alert release];
}

int ResumeWarningDialogProc(const char *string) {
    NSInteger	ret;
    
    ret = showCautionAlert(	NSLocalizedString(@"The Save Data file is conflicting", "bad data"),
                            [NSString stringWithUTF8String:string],
						   NSLocalizedString(@"Continue", "OK"));
	if (ret == NSAlertFirstButtonReturn) {
        return(IDOK);
    }
    return(IDCANCEL);
}

bool ResetWarningDialogProc(void) {
    SInt16	ret;
    
    if (np2oscfg.comfirm) {
        ret = showCautionAlert(NSLocalizedString(@"Reset", "Reset title"),
							   NSLocalizedString(@"Are you sure you want to reset?", "Reset causion string"),
                                NULL);
        if (ret == NSAlertSecondButtonReturn) {
            return(false);
        }
    }
    return(true);
}

bool QuitWarningDialogProc(void) {
    SInt16	ret;
    
    if (np2oscfg.comfirm) {
        ret = showCautionAlert(NSLocalizedString(@"Quit", "Quit title"),
							   NSLocalizedString(@"Are you sure you want to quit?", "Quit caution string"),
                                NULL);
        if (ret == NSAlertSecondButtonReturn) {
            return(false);
        }
    }
    return(true);
}
