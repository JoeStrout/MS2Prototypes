// CS_String.cpp - non-inline implementations for CS_String.h

#include "CS_String.h"

#include "layer_defs.h"
#if LAYER_3B_HIGHER
#error "CS_String.cpp (Layer 3B) cannot depend on higher layers (4)"
#endif
#if LAYER_3B_ASIDE
#error "CS_String.cpp (Layer 3B - host) cannot depend on A-side layers (2A, 3A)"
#endif


StringStorageSPtr ss_createShared(int byteLen) {
    if (byteLen <= 0) return nullptr;	// (empty strings represented as null
    
    size_t totalSize = sizeof(StringStorage) + byteLen + 1;
    StringStorage* rawPtr = (StringStorage*)malloc(totalSize);
    if (!rawPtr) return NULL;
    
    rawPtr->lenB = byteLen;
    rawPtr->lenC = -1;  // Will be computed when needed
    rawPtr->hash = 0;   // Will be computed when needed
    rawPtr->data[byteLen] = '\0';  // Ensure null termination
    
    return std::shared_ptr<StringStorage>(rawPtr, free);
}

StringStorageSPtr ss_createShared(const char *cstr, int byteLen) {
	if (byteLen < 0) byteLen = strlen(cstr);
    if (byteLen <= 0) return nullptr;
    
    size_t totalSize = sizeof(StringStorage) + byteLen + 1;
    StringStorage* rawPtr = (StringStorage*)malloc(totalSize);
    if (!rawPtr) return NULL;
    
    rawPtr->lenB = byteLen;
    rawPtr->lenC = -1;  // Will be computed when needed
    rawPtr->hash = 0;   // Will be computed when needed
    memcpy(rawPtr->data, cstr, byteLen);
    rawPtr->data[byteLen] = '\0';  // Ensure null termination
    
    return std::shared_ptr<StringStorage>(rawPtr, free);
}

StringStorageSPtr String::FindOrCreate(const char *cstr, int byteLen) {
	if (byteLen < 0) byteLen = strlen(cstr);
	if (byteLen < 128) {
		// ToDo: look for this string already interned, or else intern it now
	}
	return ss_createShared(cstr, byteLen);
}
