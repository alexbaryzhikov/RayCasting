//
//  main.m
//  RayCasting
//
//  Created by Aleksei Baryzhikov on 30.08.25.
//

#import <Cocoa/Cocoa.h>

#import "AppDelegate.h"

int main(int argc, const char* argv[]) {
    NSApplication* application = [NSApplication sharedApplication];
    AppDelegate* appDelegate = [[AppDelegate alloc] init];
    application.delegate = appDelegate;
    return NSApplicationMain(argc, argv);
}
