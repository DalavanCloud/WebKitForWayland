/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#import <WebKit/WKFoundation.h>

#if WK_API_ENABLED

#import <WebKit/WKWebsiteDataRecord.h>

NS_ASSUME_NONNULL_BEGIN

/*! A WKWebsiteDataStore represents various types of data that a website might
 make use of. This includes cookies, disk and memory caches, and persistent data such as WebSQL,
 IndexedDB databases, and local storage.
 */
WK_CLASS_AVAILABLE(10_11, 9_0)
@interface WKWebsiteDataStore : NSObject <NSCoding>

/* @abstract Returns the default data store. */
+ (WKWebsiteDataStore *)defaultDataStore;

/** @abstract Returns a new non-persistent data store.
 @discussion If a WKWebView is associated with a non-persistent data store, no data will
 be written to the file system. This is useful for implementing "private browsing" in a web view.
*/
+ (WKWebsiteDataStore *)nonPersistentDataStore;

- (instancetype)init NS_UNAVAILABLE;

/*! @abstract Whether the data store is persistent or not. */
@property (nonatomic, readonly, getter=isPersistent) BOOL persistent;

/*! @abstract Returns a set of all available website data types. */
+ (WK_SET(NSString *) *)allWebsiteDataTypes;

/*! @abstract Fetches data records containing the given website data types.
  @param dataTypes The website data types to fetch records for.
  @param completionHandler A block to invoke when the data records have been fetched.
*/
- (void)fetchDataRecordsOfTypes:(WK_SET(NSString *) *)dataTypes completionHandler:(void (^)(WK_ARRAY(WKWebsiteDataRecord *) *))completionHandler;

/*! @abstract Removes website data of the given types for the given data records.
 @param dataTypes The website data types that should be removed.
 @param dataRecords The website data records to delete website data for.
 @param completionHandler A block to invoke when the website data for the records has been removed.
*/
- (void)removeDataOfTypes:(WK_SET(NSString *) *)dataTypes forDataRecords:(WK_ARRAY(WKWebsiteDataRecord *) *)dataRecords completionHandler:(void (^)(void))completionHandler;

/*! @abstract Removes all website data of the given types that has been modified since the given date.
 @param dataTypes The website data types that should be removed.
 @param date A date. All website data modified after this date will be removed.
 @param completionHandler A block to invoke when the website data has been removed.
*/
- (void)removeDataOfTypes:(WK_SET(NSString *) *)websiteDataTypes modifiedSince:(NSDate *)date completionHandler:(void (^)(void))completionHandler;

@end

NS_ASSUME_NONNULL_END

#endif
