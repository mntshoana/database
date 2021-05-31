#ifndef STORAGE_HEADER
#define STORAGE_HEADER

#include "mySQLDB.h"

typedef enum {
    Internal,
    Leaf
}NodeType;

// Base Node
const uint32_t NodeTypeSize = sizeof(uint8_t);
const uint32_t IsRootSize = sizeof(uint8_t);
const uint32_t ParentPtrSize = sizeof(uint32_t);
const uint8_t BaseNodeSize = NodeTypeSize + IsRootSize + ParentPtrSize;

const uint32_t IsRootOffset = NodeTypeSize;
const uint32_t ParentPointOffset = NodeTypeSize + IsRootSize;


// Leaf Node
const uint32_t LeafCellCountSize = sizeof(uint32_t);
const uint32_t LeafHeaderSize =  BaseNodeSize + LeafCellCountSize;
// Leaf Node Body
const uint32_t LeafKeySize = sizeof(uint32_t);
const uint32_t LeafValueSize = LeafKeySize;
const uint32_t LeafCellSize = LeafKeySize + LeafValueSize;

const uint32_t LeafCellCountOffset = BaseNodeSize;
const uint32_t LeafValueOffset = LeafKeySize;

const uint32_t LeafAllocation = PAGE_SIZE - LeafHeaderSize;
const uint32_t LeafMaxCells = LeafAllocation / LeafCellSize;

// Note: B = byte
//
//---nodeType[1B] ---|---isRoot[1B]---|---parentPtr[4B]---|
//---cellCount[4B]---|
//------key[4B]------|---value[293B]---|
//----... more keys and values ...-----|
//-- Total 13 keys and values[3861B] --|
//-- wasted space: 4096 - (1+1+4+4+3861)
//--             : 4096 - 3871
//--             : 225 bytes wasted from page size

uint32_t* leafNode;
#endif
