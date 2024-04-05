//
//  Document.h
//  IFFEdit
//
//  Created by Fredrik on 2024-04-05.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface IFFChunk : NSObject
@property (strong) NSString *chunkID;
@property (readonly) NSUInteger size;
@property (strong) NSData *data;
@property (strong, nullable) NSString *text;
@end

@interface IFFGroup : IFFChunk
@property (strong) NSString *subtypeID;
@property (readonly) NSMutableArray<IFFChunk *> *subChunks;
@end

@interface Document : NSDocument <NSOutlineViewDataSource, NSOutlineViewDelegate>

@property (strong) IBOutlet NSOutlineView *outlineView;

@property (strong) IFFGroup *rootGroup;

- (IBAction)addGroup:(id)sender;
- (IBAction)addChunk:(id)sender;
- (IBAction)removeChunk:(id)sender;

@end

NS_ASSUME_NONNULL_END
