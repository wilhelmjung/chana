package main

import (
    "fmt"
    "os"
    "io"
    "encoding/gob"
    //"reflect"
)

/*
  // solution I: index/data split
  index file:
  - size consideration:
  - segmented:
    one segment per file
    - size: 16MB
    - naming:
      index-${timestamp}-00000001.dat
  1. file header;
     1.1 magic word: "index___", 8 bytes
     1.2 file sequence: uint64
     1.3 file size: uint64
     1.3 offset: uint64, start of page address
  2. first page:
     1.1 magic word: "page____"
     1.2 page seq: uint64
     1.3 page size: uint64
     1.3 addr offset: uint64, start of page ptr address
     2.1 key array: N, uint64
     2.2 value ptr array: N, uint64, 
         pointed to the address space in tuple files
     2.3 page ptr array: N + 1, uint64
  3. second page;

  tuple file:
  - size consideration:
  - segmented:
    one segment per file
  - append only
    - size: 4GB
    - naming:
      tuple-${timestamp}-00000001.dat
  1. file header;
     1.1 magic word: "tuple___"
     1.2 file seq: uint64
     1.3 file size: uint64
     1.3 addr offset: uint64, start of value ptr address
  2. first row;
     2.1 magic word: "value___"
     2.2 value length
     2.3 value checksum
     2.4 value byte array: 
         need to be decode to KV pair
  3. second row; ......

  // solution II: index/data colocated.
     - huge B-Tree node page.
     - size: 16MB
*/

const (
    MAG = 1234567890
    IDX = 1
    DAT = 2
)

// TLV header
type FileHeader struct {
    MagicWord int
    TypeCode  int
    ValueLen  int
    ValueCrc  int
}

type FileHeader2 struct {
    FileHeader
    Value []byte
}

func check(err error) {
    if err != nil {
        fmt.Printf("err: %v\n", err)
        panic(err)
    }
}

func main() {
    hdr := FileHeader2{ FileHeader{ MAG, IDX, 66, 0xFFFFFFFF }, []byte{ 1, 2, 3, 4, 5 } }
    main_1(hdr)
}

func main_1(hdr interface{}) {
    f, err := os.OpenFile("./tmp.dat", os.O_RDWR | os.O_CREATE, 0755)
    check(err)
    defer f.Close()

    fmt.Printf("hdr: %v\n", hdr)
    encoder := gob.NewEncoder(f)
    encoder.Encode(hdr)
    f.Sync()
    f.Close()
    fmt.Println("write finished!")

    // decode
    f2, err := os.Open("./tmp.dat")
    check(err)
    defer f2.Close()

    var hdr2 FileHeader2
    decoder := gob.NewDecoder(f2)
    err = decoder.Decode(&hdr2)
    check(err)
    fmt.Printf("hdr2: %v\n", hdr2)
    fmt.Printf("byte: %v\n", hdr2.Value)

    offset, err := f2.Seek(0, io.SeekCurrent)
    fmt.Printf("offset: %v\n", offset)

    fmt.Println("read finished!")
}
