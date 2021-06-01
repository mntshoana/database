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

void initLeafNode(void* node){
    *getLeafCellCount(node) = 0;
}

void insertLeaf(TableCursor* cursor, uint32_t key, Row* content){
    void* node = getPage(cursor->table->pager, cursor->pgNr);
   
    uint32_t cellNr = *getLeafCellCount(node);
    if (cellNr >= LeafMaxCells) {
       // Node full
         printf("No splitting of a leaf node is implemented yet.\n");
         exit(EXIT_FAILURE);
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


void logConstants(){
    printf("Row size: %d\n", ROW_SIZE);
    printf("Size of common header in all Nodes: %d\n", BaseNodeSize);
    printf("Size of the header in a Leaf Node: %d\n", LeafHeaderSize);
    printf("Available usable space limit given to a single leaf node: %d\n", LeafAllocation);
    printf("Size of one cell (table row) in a Leaf Node: %d\n", LeafCellSize);
    printf("Maximum cells possible to allocate within the given space: %d\n", LeafMaxCells);
}
