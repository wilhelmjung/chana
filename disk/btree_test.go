// test with: go test btree
package main

import (
	"flag"
	"fmt"
	"log"
	"strconv"
	"testing"

	"github.com/stretchr/testify/assert"
)

func beforeTestInsert() {
}

func afterTestInsert() {

}

func TestInsert(t *testing.T) {
	beforeTestInsert()
	defer afterTestInsert()

	type args struct {
		keyVal *Pair
	}
	tests := []struct {
		name    string
		args    args
		want    bool
		wantErr bool
	}{
		{"test1", args{&Pair{2, []byte{'f', 'o', 'o'}}}, true, false}, // do not want error
		{"test2", args{&Pair{1, []byte{'b', 'a', 'r'}}}, true, false}, // do not want error
		{"test3", args{&Pair{5, []byte{'b', 'a', 'z'}}}, true, false}, // do not want error
		{"test4", args{&Pair{3, []byte{'z', 'o', 'e'}}}, true, false}, // do not want error
		{"test5", args{&Pair{4, []byte{'d', 'o', 't'}}}, true, false}, // do not want error
	}
	d := NewDB()
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got, err := d.Insert(tt.args.keyVal)
			if (err != nil) != tt.wantErr {
				t.Errorf("Insert() error = %v, wantEr %v", err, tt.wantErr)
				return
			}
			if got != tt.want {
				t.Errorf("Insert() = %v, want %v", got, tt.want)
			}
		})
	}
	// dump root node
	t.Logf("root: %+v", d.root) //TODO too much logs.
}

func SetUpInsertIntoNode() {
}

func TearDownInsertIntoNode() {
	//TODO //deleteDB
}

func Test_insertIntoNode(t *testing.T) {
	// setup and teardown
	SetUpInsertIntoNode()
	defer TearDownInsertIntoNode()

	db := NewDB()

	type args struct {
		cursor Cursor
		kv     *Pair
	}
	tests := []struct {
		name    string
		args    args
		want    bool
		wantErr bool
	}{
		{
			"test_insert_into_empty_node",
			args{
				Cursor{db.NewNode(), 0},
				&Pair{1, []byte{'a', 'b', 'c'}},
			},
			true, false,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got, err := db.insertIntoNode(tt.args.cursor, tt.args.kv)
			if (err != nil) != tt.wantErr {
				t.Errorf("insertIntoNode() error = %v, wantErr %v", err, tt.wantErr)
				return
			}
			if got != tt.want {
				t.Errorf("insertIntoNode() = %v, want %v", got, tt.want)
			}
		})
		// dump root node
		log.Printf("node: %+v", *tt.args.cursor.Node) //TODO too much logs.
	}
}
func TestInsertAndSearch1(t *testing.T) {
	db := NewDB()
	for i := 1; i <= 10; i++ {
		str := fmt.Sprintf("foo_%v", i)
		db.Insert(&Pair{i, []byte(str)})
	}
	p := db.Search(4)
	//log.Printf("found p: %v, root: %v", string(p.Val), root)
	log.Printf("found p: %v, root: %v", p, db.root)
}

func TestInsertAndSearch0(t *testing.T) {
	db := NewDB()
	kv1 := &Pair{10, []byte("foo")}
	kv2 := &Pair{20, []byte("bar")}
	kv3 := &Pair{30, []byte("baz")}

	var (
		ok  bool
		err error
		p   *Pair
	)
	ok, err = db.Insert(kv2)
	log.Printf("ok:%v, err:%v", ok, err)
	ok, err = db.Insert(kv1)
	log.Printf("ok:%v, err:%v", ok, err)
	ok, err = db.Insert(kv3)
	log.Printf("ok:%v, err:%v", ok, err)

	p = db.Search(30)
	log.Printf("found p: %v, root: %v", string(p.Val), db.root)
	p = db.Search(10)
	log.Printf("found p: %v, root: %v", string(p.Val), db.root)
	p = db.Search(20)
	log.Printf("found p: %v, root: %v", string(p.Val), db.root)

}

// compare performance with hash map;
// HashMap impl
type Hash struct {
	Map map[int][]byte
}

func NewHash() *Hash {
	var h Hash
	h.Map = make(map[int][]byte)
	return &h
}

func (hash *Hash) Init() {
	fmt.Println("using hashmap!")
	hash.Map[42] = []byte("101")
}

func (hash *Hash) Insert(kv *Pair) (bool, error) {
	hash.Map[kv.Key] = kv.Val
	return true, nil
}

func (hash *Hash) Search(key int) *Pair {
	val := hash.Map[key]
	return &Pair{key, val}
}

func genVal(i int) []byte {
	s := fmt.Sprintf("#%d", i)
	return []byte(s)
}

func BenchmarkInsert1M(b *testing.B) {
	db := NewDB()
	for i := 1; i <= b.N; i++ {
		kv := &Pair{i, genVal(i)}
		ok, err := db.Insert(kv)
		if !ok || err != nil {
			b.Error("Insert failed!")
		}
	}
}

func BenchmarkSearch1M(b *testing.B) {
	db := NewDB()
	for i := 1; i <= b.N; i++ {
		kv := &Pair{i, genVal(i)}
		ok, err := db.Insert(kv)
		if !ok || err != nil {
			b.Error("Insert failed!")
		}
	}
	b.ResetTimer()
	for i := b.N; i <= 1; i-- {
		p := db.Search(i)
		if p == nil {
			b.Error("Search failed!")
		}
	}
}

// command line:
//   using hashmap:   go test db -run ^TestDB -v -args 100000000 1
//   using simple DB: go test db -run ^TestDB -v -args 100000000 2
// TestDB :
func TestDB(t *testing.T) {
	a := assert.New(t)

	flag.Parse()
	//num := 1000 * 1000
	num := 1000
	args := flag.Args()
	var db Interface
	if len(args) >= 1 {
		num, _ = strconv.Atoi(args[0])
	}
	db = new(DB)
	if len(args) >= 2 {
		use, _ := strconv.Atoi(args[1])
		if use == 1 {
			db = NewHash()
			t.Logf("use hash\n")
		} else {
			db = new(DB)
			t.Logf("use DB\n")
		}
	}

	t.Logf("num: %d\n", num)

	db.Init()

	genVal := func(i int) []byte {
		s := fmt.Sprintf("#%d", i)
		return []byte(s)
	}

	for i := 1; i <= num; i++ {
		kv := &Pair{i, genVal(i)}
		ok, err := db.Insert(kv)
		a.True(ok)
		a.Nil(err)
	}

	for i := num; i <= 1; i-- {
		p := db.Search(i)
		a.NotNil(p)
		a.Equal(p.Key, i)
		a.Equal(p.Val, genVal(i))
	}

}

// test mmap btree index file.
func TestDiskBTree(t *testing.T) {

    fmt.Printf("testing...")
}
