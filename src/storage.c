#include "storage.h"
#include "mySQLDB.h"

void createNewRoot(Table* table, uint32_t rightChildPageNumber){
    // Splits the root
    void* root = getPage(table->pager, table->rootPage);
    void* rightChild = getPage(table->pager, rightChildPageNumber);
    // create new page for left child
    uint32_t newPage = getEmptyPage(table->pager);
    void* leftChild = getPage(table->pager, newPage);
    
    // Update parent page number
    *getParentNode(leftChild) = table->rootPage;
    *getParentNode(rightChild) = table->rootPage;
    
    // The old root is copied to the left child
    memcpy(leftChild, root, PAGE_SIZE);
    setIsRootNode(leftChild, false);
    
    // Initialize root as internal node
    initInternalNode(root);
    setIsRootNode(root, true);
    *getInternalNodeKeyCount(root) = 1; // with one key
    // add two children
    *getInternalNodeChildAt(root, 0) = newPage; // left node
    *getInternalNodeKeyAt(root, 0) = getNodeMaxKey(leftChild);
    *getInternalNodeRightChild(root) = rightChildPageNumber;
    
}

uint32_t* getParentNode(void* node){
    return node + ParentPointOffset;
}

NodeType getNodeType(void* node){
    return *( (uint8_t*) node);
}

void setNodeType(void* node, NodeType type){
    *( (uint8_t*) node) =  type;
}

bool isRootNode(void* node){
    uint8_t isRootState = *(uint8_t*)(node + IsRootOffset);
    return (bool)isRootState;
}
void setIsRootNode(void* node, bool isRootNodeState){
    *(uint8_t*)(node + IsRootOffset) = isRootNodeState;
}

uint32_t* getLeafCellCount(void* node){
    return node + LeafCellCountOffset;
}

uint32_t* getLeafNextNode(void* node){
    return node + LeafNextNodeOffset;
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
    setNodeType(node, Leaf);
    *getLeafCellCount(node) = 0;
    setIsRootNode(node, false);
    *getLeafNextNode(node) = 0; // here, zero represents no siblings
}

void insertLeaf(TableCursor* cursor, Row* content){
    uint32_t key = content->id;
    void* node = getPage(cursor->table->pager, cursor->pgNr);
   
    uint32_t cellNr = *getLeafCellCount(node);
    if (cellNr >= LeafMaxCells) {
       // Node full
        splitLeaf(cursor, key, content);
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

void      splitLeaf(TableCursor* cursor, uint32_t key, Row* content){
    uint32_t pageNr = getEmptyPage(cursor->table->pager);
    
    void* leftNode = getPage(cursor->table->pager, cursor->pgNr);
    uint32_t leftMax = getNodeMaxKey(leftNode);
    void* rightNode = getPage(cursor->table->pager, pageNr);
    initLeafNode(rightNode);
    
    *getParentNode(rightNode) = *getParentNode(leftNode);
    *getLeafNextNode(rightNode) = *getLeafNextNode(leftNode);
    *getLeafNextNode(leftNode) = pageNr;
    
    // Divide keys into correct position
    for (uint32_t i = LeafMaxCells; i >= 0; i--){
        void* correctNode;
        if (i >= leafSplitCountForLeft)
            correctNode = rightNode;
        else
            correctNode = leftNode;
        
        uint32_t index = i % leafSplitCountForLeft;
        void* targetCell = getLeafCell(correctNode, index);
        
        if (i == cursor->cellNr) {
            serializeRow(content, getLeafValue(targetCell, index));
            *getLeafKey(targetCell, index) = key;
        }
        else if (i > cursor->cellNr)
            memcpy(targetCell, getLeafCell(leftNode, i-1), LeafCellSize);
        else
            memcpy(targetCell, getLeafCell(leftNode, i), LeafCellSize);
        
    }
    
    // Update header
    *getLeafCellCount(leftNode)  = leafSplitCountForLeft;
    *getLeafCellCount(rightNode) = leafSplitCountForRight;
    
    // Create or Update Parent
    if (isRootNode(leftNode))
        return createNewRoot(cursor->table, pageNr);
    else {
        uint32_t parentPage = *getParentNode(leftNode);
        uint32_t updatedMax = *getNodeMaxKey(leftNode);
        void* parent = getPage(cursor->table->pager, parentPage);
        updateInternalNodeKey(parent, leftMax, updatedMax);
        insertInternalNodeChild(cursor->table, parentPage, pageNr);
        return;
    }
    
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

uint32_t* getInternalNodeKeyCount(void* node){
    return node + InternalKeySizeOffset;
}

uint32_t* getInternalNodeCell(void* node, uint32_t index){
    return node + InternalHeaderSize + index * InternalCellSize;
}

uint32_t* getInternalNodeRightChild(void* node){
    return node + InternalRighChildSizeOffset;
}

uint32_t* getInternalNodeChildAt(void* node, uint32_t childNr){
    uint32_t keyCount = *getInternalNodeKeyCount(node);
    if (childNr > keyCount){
        printf("Error: Attempt to access child number %d, which is greater than the number of keys %d\n",
               childNr, keyCount);
        exit(EXIT_FAILURE);
    } else if (childNr == keyCount)
        return getInternalNodeRightChild(node);
    else
        return getInternalNodeCell(node, childNr);
}

uint32_t* getInternalNodeKeyAt(void* node, uint32_t keyNr){
    return (void*)getInternalNodeCell(node, keyNr) + InternalChildSize;
    // getInternalNodeCell returns uint32_t* (4 bytes)
    // intention is to add 4 bytes, not 4*4 bytes, hence the (void*) cast
}

uint32_t* getNodeMaxKey(void* node){
    switch (getNodeType(node)){
        case Internal:
            return *getInternalNodeKeyAt(node, *getInternalNodeKeyCount(node) - 1);
        case Leaf:
            return *getLeafKey(node, *getLeafCellCount(node) -1);
    }
}

void initInternalNode(void* node){
    setNodeType(node, Internal);
    setIsRootNode(node, false);
    *getInternalNodeKeyCount(node) = 0;
}

TableCursor* nodeFind(Table* table, uint32_t pageNr, uint32_t key){
    void* node = getPage(table->pager, pageNr);
    
    uint32_t childIndex = nodeFindChild(node, key);
    uint32_t childNr = *getInternalNodeChildAt(node, childIndex);
    void* child = getPage(table->pager, childIndex);
    switch (getNodeType(child)){
        case Leaf:
            return findFromLeaf(table, childIndex, key);
        case Internal:
            return nodeFind(table, childIndex, key);
    }
    
}


uint32_t* nodeFindChild(void* node, uint32_t key){
    uint32_t keyCount = *getInternalNodeKeyCount(node);
    
    // Binary Search for child index
    uint32_t min = 0;
    uint32_t max = keyCount;
    
    while (min != max){
        uint32_t index = (min+max) / 2;
        uint32_t keyToRight = *getInternalNodeKeyAt(node, index);
        if (keyToRight >= key)
            max = index;
        else
            min = index + 1;
    }
    
    return min; // index
}

void updateInternalNodeKey(void* node, uint32_t oldKeyValue, uint32_t newKeyValue){
    uint32_t oldChildLastValue = nodeFindChild(node, oldKeyValue);
    *getInternalNodeKeyAt(node, oldChildLastValue) = newKeyValue;
}

void insertInternalNodeChild(Table* table, uint32_t parentPage, uint32_t childPage){
    void* parent = getPage(table->pager, parentPage);
    void* child = getPage(table->pager, childPage);
    uint32_t childMax = getNodeMaxKey(child);
    uint32_t index = nodeFindChild(parent, childMax);

    uint32_t originalKeyCount = *getInternalNodeKeyCount(parent);
    *getInternalNodeKeyCount(parent) = originalKeyCount + 1;

    if (originalKeyCount >= InternalNodeMaxCells) {
        printf("Error: Have not implemented splitting of internal node\n");
        exit(EXIT_FAILURE);
    }
    
    uint32_t rightChildPage = *getInternalNodeRightChild(parent);
    void* rightChild = getPage(table->pager, rightChildPage);

    if (childMax > getNodeMaxKey(rightChild)) {
        // Replace
        *getInternalNodeChildAt(parent, originalKeyCount) = rightChildPage;
        *getInternalNodeKeyAt(parent, originalKeyCount) = getNodeMaxKey(rightChild);
        *getInternalNodeRightChild(parent) = childPage;
    } else {
        // Make room for the new cell
        for (uint32_t i = originalKeyCount; i > index; i--) {
            void* targetCell = getInternalNodeCell(parent, i);
            void* sourceCell = getInternalNodeCell(parent, i - 1);
            memcpy(targetCell, sourceCell, InternalCellSize);
        }
        *getInternalNodeChildAt(parent, index) = childPage;
        *getInternalNodeKeyAt(parent, index) = childMax;
    }
}

void logConstants(){
    printf("Row size: %d\n", ROW_SIZE);
    printf("Size of common header in all Nodes: %d\n", BaseNodeSize);
    printf("Size of the header in a Leaf Node: %d\n", LeafHeaderSize);
    printf("Available usable space limit given to a single leaf node: %d\n", LeafAllocation);
    printf("Size of one cell (table row) in a Leaf Node: %d\n", LeafCellSize);
    printf("Maximum cells possible to allocate within the given space: %d\n", LeafMaxCells);
}

void spaceIndent(uint32_t level){
    for (uint32_t i = 0; i < level; i++)
        printf("  ");
}
void logTree(void* pager, uint32_t pgNr, uint32_t level){
    void* node = getPage(pager, pgNr);
    uint32_t keyCount, child;
    
    switch (getNodeType(node)){
        case Leaf:
            keyCount = *getLeafCellCount(node);
            spaceIndent(level);
            printf("- leaf [size %d]\n", keyCount);
            for (uint32_t i = 0; i < keyCount; i++){
                spaceIndent(level +1);
                printf("- %d\n", *getLeafKey(node, i));
            }
            break;
        case Internal:
            keyCount = *getInternalNodeKeyCount(node);
            spaceIndent(level);
            printf("- internal [size %d]\n", keyCount);
            for (uint32_t i = 0; i < keyCount; i++){
                child = *getInternalNodeChildAt(node, i);
                logTree(pager, child, level+1);
                
                spaceIndent(level+1);
                printf("- key %d\n", *getInternalNodeKeyAt(node, i));
            }
            child = *getInternalNodeRightChild(node);
            logTree(pager, child, level+1);
            break;
    }
}
