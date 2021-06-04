
#ifndef STORAGE_HEADER
#define STORAGE_HEADER

#include "mySQLDB.h"

typedef enum {
    Internal,
    Leaf
}NodeType;

// Base Node
static const uint32_t NodeTypeSize = sizeof(uint8_t);
static const uint32_t IsRootSize = sizeof(uint8_t);
static const uint32_t ParentPtrSize = sizeof(uint32_t);
static const uint8_t BaseNodeSize = NodeTypeSize + IsRootSize + ParentPtrSize;

static const uint32_t IsRootOffset = NodeTypeSize;
static const uint32_t ParentPointOffset = NodeTypeSize + IsRootSize;


// Leaf Node
static const uint32_t LeafCellCountSize = sizeof(uint32_t);
static const uint32_t LeafHeaderSize =  BaseNodeSize + LeafCellCountSize;
// Leaf Node Body
static const uint32_t LeafKeySize = sizeof(uint32_t);
static const uint32_t LeafValueSize = ROW_SIZE;
static const uint32_t LeafCellSize = LeafKeySize + LeafValueSize;

static const uint32_t LeafCellCountOffset = BaseNodeSize;
static const uint32_t LeafValueOffset = LeafKeySize;
 
static const uint32_t LeafAllocation = PAGE_SIZE - LeafHeaderSize;
static const uint32_t LeafMaxCells = LeafAllocation / LeafCellSize;

// Note: B = byte
// Leaf node structure will involve...
//
//---nodeType[1B] ---|---isRoot[1B]---|---parentPtr[4B]---|...
//---cellCount[4B]---|...
//------key[4B]------|---value[66B]---|... // change value to 293B in future
//----... more keys and values ...----|...
//-- Total 58 keys and values[4060B]--|
//------------------------------------| // all packed into 4096 bytes (one page size)
//-- wasted space: 4096 - (1+1+4+4+4060)
//--             : 4096 - 4070
//--             : 26 bytes wasted from page size

uint32_t* getLeafCellCount(void* node);
void*     getLeafCell(void* node, uint32_t index);
uint32_t* getLeafKey(void* node, uint32_t index);
void*     getLeafValue(void* node, uint32_t index);

NodeType getNodeType(void* node);
void setNodeType(void* node, NodeType type);

void      initLeafNode(void* node); // reset cell count to zero

void      insertLeaf(TableCursor* cursor, Row* content);
TableCursor* findFromLeaf(Table* table, uint32_t pageNr, uint32_t key);

// Log info
#define SHOW_INFO_LOGS 0
void logConstants();
void logLeafNode(void* node);
#endif
