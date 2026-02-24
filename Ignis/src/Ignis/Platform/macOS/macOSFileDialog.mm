#include "Ignis/Core/File/FileDialog.h"
#import <Cocoa/Cocoa.h>
#include <filesystem>

namespace ignis {

std::string FileDialog::OpenFile()
{
    @autoreleasepool {
        NSOpenPanel* panel = [NSOpenPanel openPanel];
        [panel setCanChooseFiles:YES];
        [panel setCanChooseDirectories:NO];
        [panel setAllowsMultipleSelection:NO];
        
        // Set allowed file types (all supported formats)
        NSMutableArray* allowedTypes = [NSMutableArray array];
        
        // 3D Model formats
        [allowedTypes addObject:@"obj"];
        [allowedTypes addObject:@"fbx"];
        [allowedTypes addObject:@"FBX"];
        [allowedTypes addObject:@"gltf"];
        [allowedTypes addObject:@"glb"];
        
        // Image formats
        [allowedTypes addObject:@"png"];
        [allowedTypes addObject:@"jpg"];
        [allowedTypes addObject:@"jpeg"];
        [allowedTypes addObject:@"tga"];
        [allowedTypes addObject:@"bmp"];
        [allowedTypes addObject:@"hdr"];
        
        [panel setAllowedFileTypes:allowedTypes];
        [panel setAllowsOtherFileTypes:NO];
        
        // Set title
        [panel setTitle:@"Select File"];
        [panel setPrompt:@"Open"];
        
        // Run modal dialog
        NSModalResponse response = [panel runModal];
        
        if (response == NSModalResponseOK)
        {
            NSURL* url = [[panel URLs] objectAtIndex:0];
            NSString* path = [url path];
            
            // Convert to std::string and make absolute
            std::string pathStr = std::string([path UTF8String]);
            return std::filesystem::absolute(pathStr).string();
        }
        
        return "";
    }
}

std::vector<std::string> FileDialog::OpenMultipleFiles()
{
    std::vector<std::string> result;
    
    @autoreleasepool {
        NSOpenPanel* panel = [NSOpenPanel openPanel];
        [panel setCanChooseFiles:YES];
        [panel setCanChooseDirectories:NO];
        [panel setAllowsMultipleSelection:YES];
        
        // Set allowed file types
        NSMutableArray* allowedTypes = [NSMutableArray array];
        
        // 3D Model formats
        [allowedTypes addObject:@"obj"];
        [allowedTypes addObject:@"fbx"];
        [allowedTypes addObject:@"FBX"];
        [allowedTypes addObject:@"gltf"];
        [allowedTypes addObject:@"glb"];
        
        // Image formats
        [allowedTypes addObject:@"png"];
        [allowedTypes addObject:@"jpg"];
        [allowedTypes addObject:@"jpeg"];
        [allowedTypes addObject:@"tga"];
        [allowedTypes addObject:@"bmp"];
        [allowedTypes addObject:@"hdr"];
        
        [panel setAllowedFileTypes:allowedTypes];
        [panel setAllowsOtherFileTypes:NO];
        
        // Set title
        [panel setTitle:@"Select Files"];
        [panel setPrompt:@"Open"];
        
        // Run modal dialog
        NSModalResponse response = [panel runModal];
        
        if (response == NSModalResponseOK)
        {
            NSArray* urls = [panel URLs];
            for (NSURL* url in urls)
            {
                NSString* path = [url path];
                std::string pathStr = std::string([path UTF8String]);
                result.push_back(std::filesystem::absolute(pathStr).string());
            }
        }
    }
    
    return result;
}

std::string FileDialog::SaveFile()
{
    @autoreleasepool {
        NSSavePanel* panel = [NSSavePanel savePanel];
        
        // Set title
        [panel setTitle:@"Save File"];
        [panel setPrompt:@"Save"];
        
        // Run modal dialog
        NSModalResponse response = [panel runModal];
        
        if (response == NSModalResponseOK)
        {
            NSURL* url = [panel URL];
            NSString* path = [url path];
            
            std::string pathStr = std::string([path UTF8String]);
            return std::filesystem::absolute(pathStr).string();
        }
        
        return "";
    }
}

} // namespace ignis