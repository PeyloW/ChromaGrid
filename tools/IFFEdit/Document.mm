//
//  Document.m
//  IFFEdit
//
//  Created by Fredrik on 2024-04-05.
//

#import "Document.h"
#include "iff_file.hpp"

using namespace toybox;

@interface ContentFormatter : NSFormatter
@end

@implementation ContentFormatter
- (NSString *)stringForObjectValue:(id)obj {
    if ([obj isKindOfClass:[NSData class]]) {
        NSData *data = obj;
        NSMutableString *str = [NSMutableString string];
        for (int i = 0; i < data.length; i++) {
            [str appendFormat:@"%02X ", ((uint8_t *)data.bytes)[i]];
        }
        return str;
    }
    return nil;
}
- (BOOL)getObjectValue:(out id  _Nullable __autoreleasing *)obj forString:(NSString *)string errorDescription:(out NSString * _Nullable __autoreleasing *)error {
    NSScanner *scanner = [NSScanner scannerWithString:string];
    NSMutableData *data = [NSMutableData data];
    unsigned int i;
    while ([scanner scanHexInt:&i]) {
        if (i > 255) {
            return NO;
        }
        uint8_t b = i;
        [data appendBytes:&b length:1];
    }
    *obj = data.copy;
    return YES;
}
@end

@interface IFFChunk ()
- (instancetype)initWithChunk:(iff_chunk_s)chunk iff:(iff_file_c *)iff;
- (BOOL)writeToIFF:(iff_file_c *)iff;
@end

@interface IFFGroup ()
- (instancetype)initWithGroup:(iff_group_s)group iff:(iff_file_c *)iff;
- (BOOL)writeToIFF:(iff_file_c *)iff;
@end

@implementation IFFChunk
- (instancetype)initWithChunk:(iff_chunk_s)chunk iff:(iff_file_c *)iff {
    self = [super init];
    if (self) {
        char buf[5];
        iff_id_str(chunk.id, buf);
        _chunkID = [NSString stringWithCString:buf encoding:NSASCIIStringEncoding];
        NSMutableData *data = [NSMutableData dataWithLength:chunk.size];
        iff->read((uint8_t *)data.bytes, 1, chunk.size);
        _data = data.copy;
    }
    return self;
}
- (instancetype)init {
    self = [super init];
    if (self) {
        _chunkID = @"NULL";
        _data = [NSData new];
    }
    return self;
}
- (BOOL)writeToIFF:(iff_file_c *)iff {
    iff_chunk_s chunk;
    if (iff->begin(chunk, self.chunkID.UTF8String)) {
        if (iff->write((uint8_t *)_data.bytes, 1, _data.length)) {
            return iff->end(chunk);
        }
    }
    return NO;
}
- (NSUInteger)size {
    return _data.length;
}
- (NSString *)text {
    if ([_chunkID isEqualToString:@"TEXT"]) {
        return [[NSString alloc] initWithData:_data encoding:NSASCIIStringEncoding];
    }
    return nil;
}
- (void)setText:(NSString *)text {
    _chunkID = @"TEXT";
    NSMutableData *data = [[text dataUsingEncoding:NSASCIIStringEncoding] mutableCopy];
    uint8_t zero = 0;
    [data appendBytes:&zero length:1];
    _data = [data copy];
}
@end

@implementation IFFGroup
- (instancetype)initWithGroup:(iff_group_s)group iff:(iff_file_c *)iff {
    self = [super init];
    if (self) {
        char buf[5];
        iff_id_str(group.id, buf);
        self.chunkID = [NSString stringWithCString:buf encoding:NSASCIIStringEncoding];
        iff_id_str(group.subtype, buf);
        _subtypeID = [NSString stringWithCString:buf encoding:NSASCIIStringEncoding];
        _subChunks = [NSMutableArray array];
        
        iff_chunk_s chunk;
        while (iff->next(group, "*", chunk)) {
            if (chunk.id == IFF_FORM_ID || chunk.id == IFF_LIST_ID) {
                iff_group_s sub_group;
                iff->expand(chunk, sub_group);
                [_subChunks addObject:[[IFFGroup alloc] initWithGroup:sub_group iff:iff]];
            } else {
                [_subChunks addObject:[[IFFChunk alloc] initWithChunk:chunk iff:iff]];
            }
        }
    }
    return self;
}
- (instancetype)init {
    self = [super init];
    if (self) {
        self.chunkID = @"FORM";
        self.subtypeID = @"NULL";
        _subChunks = [NSMutableArray array];
    }
    return self;
}
- (BOOL)writeToIFF:(iff_file_c *)iff {
    iff_group_s group;
    if (iff->begin(group, self.chunkID.UTF8String)) {
        iff_id_t subtype = iff_id_make(self.subtypeID.UTF8String);
        if (iff->write(subtype)) {
            for (IFFChunk *chunk : _subChunks) {
                if (![chunk writeToIFF:iff]) {
                    return NO;
                }
            }
            return iff->end(group);
        }
    }
    return NO;
}
- (NSUInteger)size {
    NSUInteger size = 4;
    for (IFFChunk *chunk : _subChunks) {
        size += 8 + chunk.size;
    }
    return size;
}
@end

@interface Document ()
@end

@implementation Document

- (instancetype)init {
    self = [super init];
    if (self) {
        _rootGroup = [IFFGroup new];
        _rootGroup.chunkID = @"FORM";
        _rootGroup.subtypeID = @"NULL";
    }
    return self;
}

+ (BOOL)autosavesInPlace {
    return NO;
}

- (NSString *)windowNibName {
    return @"Document";
}

#pragma mark --- Read/write files

- (BOOL)writeToURL:(NSURL *)url ofType:(NSString *)typeName error:(NSError *__autoreleasing  _Nullable *)outError {
    iff_file_c iff(url.path.UTF8String, "w+");
    return [_rootGroup writeToIFF:&iff];
}

- (BOOL)readFromURL:(NSURL *)url ofType:(NSString *)typeName error:(NSError *__autoreleasing  _Nullable *)outError {
    iff_file_c iff(url.path.UTF8String, "r");
    if (iff.get_pos() >= 0) {
        iff_group_s group;
        if (iff.first("*", "*", group)) {
            _rootGroup = [[IFFGroup alloc] initWithGroup:group iff:&iff];
        }
    }
    return _rootGroup != nil;
}

#pragma mark --- OutlineView

- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item {
    if (item == nil) {
        return 1;
    } else if ([item isKindOfClass:[IFFGroup class]]) {
        return ((IFFGroup *)item).subChunks.count;
    } else {
        return 0;
    }
}

- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item {
    if (item == nil) {
        return _rootGroup;
    } else {
        return ((IFFGroup *)item).subChunks[index];
    }
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item {
    return [item isKindOfClass:[IFFGroup class]];
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item {
    if ([tableColumn.identifier isEqualToString:@"chunkID"]) {
        if ([item isKindOfClass:[IFFGroup class]]) {
            IFFGroup *group = item;
            return [NSString stringWithFormat:@"%@:%@", group.chunkID, group.subtypeID];
        } else {
            return ((IFFChunk *)item).chunkID;
        }
    } else if ([tableColumn.identifier isEqualToString:@"size"]) {
        return @(((IFFChunk *)item).size);
    } else if ([tableColumn.identifier isEqualToString:@"content"]) {
        if (![item isKindOfClass:[IFFGroup class]]) {
            IFFChunk *chunk = item;
            return chunk.text ?: chunk.data;
        }
    }
    return nil;
}

- (void)outlineView:(NSOutlineView *)outlineView willDisplayCell:(id)_cell forTableColumn:(NSTableColumn *)tableColumn item:(id)item {
    NSCell *cell = _cell;
    if ([tableColumn.identifier isEqualToString:@"content"] && ![item isKindOfClass:[IFFGroup class]] && [item text] == nil) {
        cell.formatter = [ContentFormatter new];
    } else {
        cell.formatter = nil;
    }
}

- (void)outlineView:(NSOutlineView *)outlineView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn byItem:(id)item {
    if ([tableColumn.identifier isEqualToString:@"chunkID"]) {
        if ([item isKindOfClass:[IFFGroup class]]) {
            IFFGroup *group = item;
            group.chunkID = [object componentsSeparatedByString:@":"].firstObject;
            group.subtypeID = [object componentsSeparatedByString:@":"].lastObject;
        } else {
            IFFChunk *chunk = item;
            chunk.chunkID = object;
        }
    } else if ([tableColumn.identifier isEqualToString:@"content"]) {
        IFFChunk *chunk = item;
        if ([object isKindOfClass:[NSString class]]) {
            chunk.text = object;
        } else {
            chunk.data = object;
        }
    }
    [self updateChangeCount:NSChangeDone];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldEditTableColumn:(NSTableColumn *)tableColumn item:(id)item {
    if ([tableColumn.identifier isEqualToString:@"size"]) {
        return NO;
    } else if ([tableColumn.identifier isEqualToString:@"content"]) {
        return ![item isKindOfClass:[IFFGroup class]];
    } else {
        return YES;
    }
}

- (void)_addChunk:(IFFChunk *)chunk {
    NSInteger row = self.outlineView.selectedRow;
    if (row >= 0 && row < self.outlineView.numberOfRows) {
        id item = [self.outlineView itemAtRow:row];
        IFFGroup *group = item;
        NSUInteger idx = -1;
        if (![group isKindOfClass:[IFFGroup class]]) {
            group = [self.outlineView parentForItem:item];
            idx = [group.subChunks indexOfObject:item];
        }
        if (idx >= 0) {
            [group.subChunks insertObject:chunk atIndex:idx + 1];
        } else {
            [group.subChunks addObject:chunk];
        }
        [self.outlineView reloadItem:_rootGroup reloadChildren:YES];
    }
}

- (void)addGroup:(id)sender {
    [self _addChunk:[IFFGroup new]];
}

- (void)addChunk:(id)sender {
    [self _addChunk:[IFFChunk new]];
}

- (void)removeChunk:(id)sender {
    NSInteger row = self.outlineView.selectedRow;
    if (row >= 0 && row < self.outlineView.numberOfRows) {
        id item = [self.outlineView itemAtRow:row];
        IFFGroup *group = [self.outlineView parentForItem:item];
        [group.subChunks removeObject:item];
        [self.outlineView reloadItem:_rootGroup reloadChildren:YES];
    }
}

@end
