//
//  main.m
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 30.08.25.
//

#import "AppDelegate.h"
#import <Cocoa/Cocoa.h>

int main(int argc, const char* argv[]) {
    NSApplication* application = [NSApplication sharedApplication];
    AppDelegate* appDelegate = [[AppDelegate alloc] init];
    application.delegate = appDelegate;
    return NSApplicationMain(argc, argv);
}
