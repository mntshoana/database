#include "storage.h"
#include "mySQLDB.h"

uint32_t* getLeafCellCount(void* node){
    return node + LeafCellCountOffset;
}

void* getLeafCell(void* node, uint32_t index){
    return node + LeafHeaderSize + index * LeafCellSize;
}

uint32_t* getLeafKey(void* node, uint32_t index){
  return getLeafCell(node, index);
}

void* getLeafValue(void* node, uint32_t index){
  return getLeafCell(node, index) + LeafKeySize;
}

NodeType getNodeType(void* node){
    return *( (uint8_t*) node);
}

void setNodeType(void* node, NodeType type){
    *( (uint8_t*) node) =  type;
}

void initLeafNode(void* node){
    setNodeType(node, Leaf);
    *getLeafCellCount(node) = 0;
}

void insertLeaf(TableCursor* cursor, Row* content){
    uint32_t key = content->id;
    void* node = getPage(cursor->table->pager, cursor->pgNr);
   
    uint32_t cellNr = *getLeafCellCount(node);
    if (cellNr >= LeafMaxCells) {
       // Node full
        *splitLeaf(cursor, key, content);
        return;
    }
   
    if (cursor->cellNr <  cellNr) {
       // Make room for cell
        for (uint32_t i = cellNr; i > cursor->cellNr; i--) {
            memcpy(getLeafCell(node, i), getLeafCell(node, i-1), LeafCellSize);
        }
    }
   
    *(getLeafCellCount(node)) += 1;
    *(getLeafKey(node, cursor->cellNr)) = key;
    serializeRow(content, getLeafValue(node, cursor->cellNr));
}

TableCursor* findFromLeaf(Table* table, uint32_t pageNr, uint32_t key){
    void* node = getPage(table->pager, pageNr);
    uint32_t cellCount = *getLeafCellCount(node);
    
    TableCursor* cursor = malloc(sizeof(TableCursor));
    cursor->table = table;
    cursor->pgNr = pageNr;
    
    // Binary Search
    uint32_t start = 0;
    uint32_t end = cellCount;
    while (start != end){
        uint32_t index = (start + end) / 2;
        uint32_t currentKey = *getLeafKey(node, index);
        if (key == currentKey){ // found
            cursor->cellNr = index;
            return cursor; // return position
        }
        if (key < currentKey)
            end = index;
        else
            start = index +1;
    }
    
    cursor->cellNr = start;
    // position here could be:
    // the key which should be moved to insert new key
    // or
    // one past the last key
    return cursor;
}


void logConstants(){
    printf("Row size: %d\n", ROW_SIZE);
    printf("Size of common header in all Nodes: %d\n", BaseNodeSize);
    printf("Size of the header in a Leaf Node: %d\n", LeafHeaderSize);
    printf("Available usable space limit given to a single leaf node: %d\n", LeafAllocation);
    printf("Size of one cell (table row) in a Leaf Node: %d\n", LeafCellSize);
    printf("Maximum cells possible to allocate within the given space: %d\n", LeafMaxCells);
}
void logLeafNode(void* node){
    uint32_t cellCount = *getLeafCellCount(node);
    printf("Cell(s) counted: %d\n", cellCount);
    for (uint32_t i = 0; i < cellCount; i++){
        uint32_t primaryKey = *getLeafKey(node, i);
        printf(" - %d : %d\n", i, primaryKey);
    }
}
