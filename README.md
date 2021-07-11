# Design

## What is ChanaDB?
   - Store data on layered hardware storages, using LSM-Tree data structure.
   - A key-value database, compatible with Redis protocol.
   - Cloud Native, using S3 or other object storage as cold data storage (backend).
 
## Goal:
   Not designed for CAP, but:
   - Consistency
   - High Availability(Not just availability);

## Non-Goal:
   - Transaction is not supported.
   - SQL is not supported.

## Data structures

### LSM-Tree
    In memory:
    - Memtable 
    - Immutable Memtable 
      B-Tree(Applend Only/Copy On Write)/RB-Tree/SkipList

    On disk(files):
    - WAL(Write Ahead Log file) 
      FIFO on disk(or distributed storage/file system)
    - SSTable
      - Level[0-6]:
        - max size:
          1M, 1G
        - segment:

    Compaction strategy:
    - Mixed compaction
      - memtable/imutable_memtable:
        max_memtable_size: 128M
      - level 0:
        max_sstable_number: 4 
      - level 1:
        max_sstable_number: 40 
      - level 2:
        max_sstable_number: 400 
      - level 3:
        max_sstable_number: 4000 
      - total size of database:
        4444 * 128M = 555.5G


## TODO
   - Disk-based B-Tree
