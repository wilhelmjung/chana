package main

import (
    "fmt"
    "os"
    "io"
    "encoding/gob"
    //"reflect"
)

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
