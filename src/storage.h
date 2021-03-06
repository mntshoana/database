
#ifndef STORAGE_HEADER
#define STORAGE_HEADER

#include "mySQLDB.h"

// Node info
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

// Internal Node
// Internal node structure will involve...
// Note: B = byte
//
//START of internal node structure
//---nodeType[1B] ----|---isRoot[1B]---|---parentPtr[4B]---|...
//internalKeyCount[4B]|...
//-rightChildPtr[4B]--|
//----childPtr[4B]----|---key[4B]---|...
//----... more keys and values ...----|...
//-- Total 510 childPtrs and keys [4080B]--|
//END of Structure
//
//  Packed into 4096 bytes (one page size)
//   wasted space: 4096 - (1+1+4+4+4+4080)
//               : 4096 - 4094
//               : 2 bytes wasted from page size

static const uint32_t InternalKeyCountSize = sizeof(uint32_t);
static const uint32_t InternalRighChildSize = sizeof(uint32_t);
static const uint32_t InternalHeaderSize = BaseNodeSize + InternalKeyCountSize + InternalRighChildSize;

static const uint32_t InternalKeySizeOffset = BaseNodeSize;
static const uint32_t InternalRighChildSizeOffset = InternalKeySizeOffset + InternalKeyCountSize;
// Internal Node Body
static const uint32_t InternalKeySize = sizeof(uint32_t);
static const uint32_t InternalChildSize = sizeof(uint32_t);
static const uint32_t InternalCellSize = InternalKeySize + InternalChildSize;

// This is small for testing
static const uint32_t InternalNodeMaxCells = 3;

// End of Internal Node Sructure
//-------------------------------

// Leaf Node
// Leaf node structure will involve...
// Note: B = byte
//
//START of Leaf node structure
//---nodeType[1B] ---|---isRoot[1B]---|---parentPtr[4B]---|...
//---cellCount[4B]---|--nextNode[4B]--|...
//------key[4B]------|---value[66B]---|... // change value to 293B in future
//----... more keys and values ...----|...
//-- Total 58 keys and values[4060B]--|
//END of Structure
//
//   All packed into 4096 bytes (one page size)
//   wasted space: 4096 - (1+1+4+4+4+4060)
//               : 4096 - 4074
//               : 22 bytes wasted from page size

static const uint32_t LeafCellCountSize = sizeof(uint32_t);
static const uint32_t LeafNextNodeSize = sizeof(uint32_t);
static const uint32_t LeafHeaderSize =  BaseNodeSize + LeafCellCountSize + LeafNextNodeSize;
// Leaf Node Body
static const uint32_t LeafKeySize = sizeof(uint32_t);
static const uint32_t LeafValueSize = ROW_SIZE;
static const uint32_t LeafCellSize = LeafKeySize + LeafValueSize;

static const uint32_t LeafCellCountOffset = BaseNodeSize;
static const uint32_t LeafNextNodeOffset = LeafCellCountOffset + LeafCellCountSize;
static const uint32_t LeafValueOffset = LeafKeySize;
 
static const uint32_t LeafAllocation = PAGE_SIZE - LeafHeaderSize;
static const uint32_t LeafMaxCells = LeafAllocation / LeafCellSize;

static const uint32_t leafSplitCountForRight = (LeafMaxCells +1) / 2;
static const uint32_t leafSplitCountForLeft = (LeafMaxCells +1) - leafSplitCountForRight;
// End of Leaf Node Sructure
//--------------------------

//Node functions
void createNewRoot(Table* table, uint32_t rightChildPageNumber);

uint32_t* getParentNode(void* node);

NodeType getNodeType(void* node);
void setNodeType(void* node, NodeType type);

bool isRootNode(void* node);
void setIsRootNode(void* node, bool isRootNode);

uint32_t* getLeafCellCount(void* node);
uint32_t* getLeafNextNode(void* node);
void*     getLeafCell(void* node, uint32_t index);
uint32_t* getLeafKey(void* node, uint32_t index);
void*     getLeafValue(void* node, uint32_t index);

void      initLeafNode(void* node); // reset cell count to zero
void      insertLeaf(TableCursor* cursor, Row* content);
void      splitLeaf(TableCursor* cursor, uint32_t key, Row* content); // creates a new node and creates/updates the parent node


TableCursor* findFromLeaf(Table* table, uint32_t pageNr, uint32_t key);

uint32_t* getInternalNodeKeyCount(void* node);
uint32_t* getInternalNodeCell(void* node, uint32_t index);
uint32_t* getInternalNodeRightChild(void* node);
uint32_t* getInternalNodeChildAt(void* node, uint32_t childNr);
uint32_t* getInternalNodeKeyAt(void* node, uint32_t keyNr);

uint32_t* getNodeMaxKey(void* node);
void initInternalNode(void* node);
TableCursor* nodeFind(Table* table, uint32_t pageNr, uint32_t key);
uint32_t* nodeFindChild(void* node, uint32_t key);

void updateInternalNodeKey(void* node, uint32_t oldKeyValue, uint32_t newKeyValue);
void insertInternalNodeChild(Table* table, uint32_t parentPage, uint32_t childPage);
// End of Node Functions
//--------------------------------------------------------

// Log info
#define SHOW_INFO_LOGS 0
void logConstants();

void spaceIndent(uint32_t level);
void logTree(void* pager, uint32_t pgNr, uint32_t level);
#endif
