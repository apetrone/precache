#import <Cocoa/Cocoa.h>


int platform_is64bit_macosx()
{
	NSLog( @"file: %i", [[NSRunningApplication currentApplication] executableArchitecture] );
	if ([[NSRunningApplication currentApplication] executableArchitecture] == NSBundleExecutableArchitectureX86_64)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}