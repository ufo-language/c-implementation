#include <stdio.h>

#include "mem.h"
#include "stacktrace.h"  /* TODO remove after debugging */
#include "vmem.h"

/* TODO these three variables should be static */
RawBlock _memFreeRoot;
int _memNFreeBlocks;
int _memNFreeWords;

RawBlock _prevAdjacentFreeBlock(RawBlock blk);
RawBlock _nextAdjacentFreeBlock(RawBlock blk);
void _remove(RawBlock blk);

void _setSize(RawBlock blk, Word size);
RawBlock _getPrev(RawBlock blk);
void _setPrev(RawBlock blk, RawBlock prev);

Block nullBlock = {0};
RawBlock nullRawBlock = {0};

/*------------------------------------------------------------------*/
Block memRawBlockToBlock(RawBlock rawBlock) {
  Block block = {rawBlock.a + MEMBLK_DATA_OFFSET};
  return block;
}

/*------------------------------------------------------------------*/
RawBlock memBlockToRawBlock(Block block) {
  RawBlock rawBlock = {block.a - MEMBLK_DATA_OFFSET};
  return rawBlock;
}

/*------------------------------------------------------------------*/
RawBlock memAddrToRawBlock(Address addr) {
  RawBlock rawBlock = {addr};
  return rawBlock;
}

/*------------------------------------------------------------------*/
Word memGetNBlocks(void) {
  return _memNFreeBlocks;
}

/*------------------------------------------------------------------*/
Word memGetNFreeWords(void) {
  return _memNFreeWords;
}

/*------------------------------------------------------------------*/
void memStart(void) {
  vmemStart();  /* sets all memory to 0 */
  _memFreeRoot.a = 0;
  _memNFreeBlocks = 0;
  _memNFreeWords = 0;
  int nBlocks = vmemGetNWords() / MEMBLK_GC_MARK;
  Word rawSize = MEMBLK_GC_MARK - 1;
  Word dataSize = rawSize - MEMBLK_OVERHEAD;
  Address addr = vmemGetNWords() - MEMBLK_GC_MARK;
  while (nBlocks--) {
    RawBlock rawBlock = memAddrToRawBlock(addr);
    _setSize(rawBlock, dataSize);
    memFreeRaw(rawBlock);
    addr -= rawSize;
  }
}

/*------------------------------------------------------------------*/
void memStop(void) {
  vmemStop();
}

/*------------------------------------------------------------------*/
/* TODO remove checkForCycle after debugging */

void exit(int);
extern int nMarkSweeps;

/* this checks for a cycle in the list of free blocks */
void checkForCycle(char* message) {
  Word addresses[8000];
  int index = 0;
  RawBlock blk = _memFreeRoot;
  while (blk.a) {
    for (int n=0; n<index; n++) {
      if (addresses[n] == blk.a) {
        printf("\nmem.c %s: [%s] found %d-block cycle at block %d\n", __func__, message, index, blk.a);
        printf("here's the stack trace:\n");
        stackTrace();
        printf("there have been %d mark-sweep cycles\n", nMarkSweeps);
        memDump();
        exit(1);
      }
    }
    addresses[index++] = blk.a;
    blk = memGetNext(blk);
  }
}

/*------------------------------------------------------------------*/
Block memAlloc(Word nWords) {
  RawBlock blk = _memFreeRoot;
  while (blk.a) {
    Word blkSize = memGetSize(blk);
    if (blkSize == nWords) {
      _remove(blk);
      _memNFreeBlocks--;
      _memNFreeWords -= memGetSize(blk);
      memSetStatus(blk, 0);
      return memRawBlockToBlock(blk);
    }
    if (blkSize > nWords + MEMBLK_OVERHEAD) {
      /* shrink the size of the block */
      blkSize -= nWords + MEMBLK_OVERHEAD;
      _setSize(blk, blkSize);
      /* create and return a block from the new space */
      RawBlock newBlk = memAddrToRawBlock(blk.a + blkSize + MEMBLK_OVERHEAD);
      _setSize(newBlk, nWords);
      memSetStatus(newBlk, 0);
      _memNFreeWords -= nWords + MEMBLK_OVERHEAD;
      return memRawBlockToBlock(newBlk);
    }
    blk = memGetNext(blk);
  }
  return nullBlock;
}

/*------------------------------------------------------------------*/
void memFree(Block blk) {
  memFreeRaw(memBlockToRawBlock(blk));
}

/*------------------------------------------------------------------*/
void memFreeRaw(RawBlock blk) {
  checkForCycle("memFree(1)");
  Word blkOrigSize = memGetSize(blk);
  int nJoins = 0;
  RawBlock prevBlk = _prevAdjacentFreeBlock(blk);
  if (prevBlk.a != nullRawBlock.a) {
    Word joinedSize = memGetSize(blk) + memGetSize(prevBlk) + MEMBLK_OVERHEAD;
    if (joinedSize < MEMBLK_GC_MARK) {
      blk = prevBlk;
      _setSize(blk, joinedSize);
      nJoins++;
    }
  }
  RawBlock nextBlk = _nextAdjacentFreeBlock(blk);
  if (nextBlk.a != nullRawBlock.a) {
    Word joinedSize = memGetSize(blk) + memGetSize(nextBlk) + MEMBLK_OVERHEAD;
    if (joinedSize < MEMBLK_GC_MARK) {
      _remove(nextBlk);
      _setSize(blk, joinedSize);
      nJoins++;
    }
  }
  if (nJoins == 0) {
    if (_memFreeRoot.a) _setPrev(_memFreeRoot, blk);
    memSetNext(blk, _memFreeRoot);
    _setPrev(blk, nullRawBlock);
    memSetStatus(blk, memGetSize(blk));
    _memFreeRoot = blk;
  }
  _memNFreeBlocks += (1 - nJoins);
  _memNFreeWords += blkOrigSize + nJoins * MEMBLK_OVERHEAD;
  checkForCycle("memFree(2)");
}

/*------------------------------------------------------------------*/
void memShowBlock(RawBlock blk) {
  printf("@=%d, Sz=%d, Prv=%d, Nxt=%d, Status=%d, nextAdj=%d\n",
    blk.a,
    memGetSize(blk),
    _getPrev(blk).a,
    memGetNext(blk).a,
    memGetStatus(blk),
    blk.a + memGetSize(blk) + MEMBLK_OVERHEAD);
}

/*------------------------------------------------------------------*/
void memDump(void) {
  printf("  +----------------\n");
  printf("  | memDump:\n");
  printf("  |  blocks     %d\n", _memNFreeBlocks);
  printf("  |  free words %d\n", _memNFreeWords);
  int n = 0;
  RawBlock blk = _memFreeRoot;
  while (blk.a && n < 15) { /* shows a maximum of 15 blocks */
    printf("  | %d. ", n++);
    memShowBlock(blk);
    blk = memGetNext(blk);
  }
  if (blk.a) printf("  | there are more blocks...\n");
  printf("  +----------------\n");
}

/*------------------------------------------------------------------*/
void _remove(RawBlock blk) {
  RawBlock prev = _getPrev(blk);
  RawBlock next = memGetNext(blk);
  if (_memFreeRoot.a == blk.a) _memFreeRoot = next;
  if (prev.a) memSetNext(prev, next);
  if (next.a) _setPrev(next, prev);
}

/*------------------------------------------------------------------*/
/* Returns the previous adjacent block, but only if it is a free block. */
RawBlock _prevAdjacentFreeBlock(RawBlock blk) {
  /* the status word of the previously adjacent block is just the current
     block's address - 1 */
  Word prevBlkStatus = vmemGet(blk.a - 1);
  if (prevBlkStatus > 0 && prevBlkStatus < MEMBLK_GC_MARK) {
    return memAddrToRawBlock(blk.a - prevBlkStatus - MEMBLK_OVERHEAD);
  }
  return nullRawBlock;
}

/*------------------------------------------------------------------*/
/* Returns the next adjacent block, but only if it is a free block. */
RawBlock _nextAdjacentFreeBlock(RawBlock blk) {
  RawBlock nextBlk = memAddrToRawBlock(blk.a + memGetSize(blk) + MEMBLK_OVERHEAD);
  if (nextBlk.a > MAX_ADDRESS - MEMBLK_OVERHEAD) {
    return nullRawBlock;
  }
  Word nextBlkStatus = memGetStatus(nextBlk);
  if (nextBlkStatus > 0 && nextBlkStatus < MEMBLK_GC_MARK) {
    return nextBlk;
  }
  return nullRawBlock;
}

/*------------------------------------------------------------------*/
extern bool globalDebug;
Word memGetStatus(RawBlock blk) {
  Word size = vmemGet(blk.a + MEMBLK_SIZE_OFFSET);
  Address statusAddr = blk.a + MEMBLK_DATA_OFFSET + size;
  Word status = vmemGet(statusAddr);
  return status;
}

/*------------------------------------------------------------------*/
void memSetStatus(RawBlock blk, Word status) {
  Word size = vmemGet(blk.a + MEMBLK_SIZE_OFFSET);
  Address statusAddr = blk.a + MEMBLK_DATA_OFFSET + size;
  vmemSet(statusAddr, status);
}

/*------------------------------------------------------------------*/
/* This returns the size in words of the payload area of the block */
Word memGetSize(RawBlock blk) {
  Word size = vmemGet(blk.a + MEMBLK_SIZE_OFFSET);
  return size;
}

/*------------------------------------------------------------------*/
/* This sets the size and status fields to indicate the size in words
   of the payload area of the block */
void _setSize(RawBlock blk, Word size) {
  vmemSet(blk.a + MEMBLK_SIZE_OFFSET, size);
  /* TODO instead of calling memSetStatus, inline the call to memSetStatus here */
  /* TODO or make memSetStatus a macro? */
  memSetStatus(blk, size);
}

/*------------------------------------------------------------------*/
RawBlock memGetNext(RawBlock blk) {
  return memAddrToRawBlock(vmemGet(blk.a + MEMBLK_NEXT_OFFSET));
}

/*------------------------------------------------------------------*/
void memSetNext(RawBlock blk, RawBlock next) {
  vmemSet(blk.a + MEMBLK_NEXT_OFFSET, next.a);
}

/*------------------------------------------------------------------*/
RawBlock _getPrev(RawBlock blk) {
  return memAddrToRawBlock(vmemGet(blk.a + MEMBLK_PREV_OFFSET));
}

/*------------------------------------------------------------------*/
void _setPrev(RawBlock blk, RawBlock prev) {
  vmemSet(blk.a + MEMBLK_PREV_OFFSET, prev.a);
}
