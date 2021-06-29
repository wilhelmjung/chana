package main

import (
	"log"
)

// config logger...
func init() {
	prefix := "[DB] "
	log.SetFlags(log.Ldate | log.Ltime | log.Lmicroseconds | log.Llongfile)
	log.SetPrefix(prefix)
	log.Print("module db initialized.")
}

// Interface : xx
type Interface interface {
	Init()
	Insert(keyVal *Pair) (bool, error)
	Search(key int) *Pair
}

// DB : object
type DB struct {
	// btree root node
	root *Node
	// NumCell : num of cells in one page
	// 341 * 3 * 8 = 8184, ~ 8196
	//const NumCell = 340
	//const NumCell = 16
	NumCell int //= 1024
}

// NewDB : new DB object.
func NewDB() *DB {
	d := new(DB)
	d.Init()
	return d
}

// Init : d should be mutable!
func (d *DB) Init() {
	d.NumCell = 1024 // set num cell first!
	d.root = d.NewNode()
	//log.Printf("+d.root: %v\n", d.root)
}

// Pair : kv
type Pair struct {
	Key int    `json:"key,omitempty"`
	Val []byte `json:"val,omitempty"`
}

// Cell :
type Cell struct {
	Child  *Node `json:"child,omitempty"`
	KeyVal Pair  `json:"item,omitempty"`
}

// setNumCell :
func (d *DB) setNumCell(num int) {
	if num > d.NumCell {
		d.NumCell = num
	}
	log.Printf("NumCell set to %d\n", d.NumCell)
}

// Node :
// (child, {key, val}),
type Node struct {
	Parent *Node  `json:"parent,omitempty"`
	Cells  []Cell `json:"cells,omitempty"` // Num + 1 cells, // slice
	Used   int    `json:"used,omitempty"`
}

// Cursor : search or insert position
type Cursor struct {
	Node  *Node `json:"node,omitempty"`
	Index int   `json:"index,omitempty"`
}

// NewNode :
func (d *DB) NewNode() *Node {
	if d.NumCell == 0 {
		log.Panic("Num of cell can not be 0.")
	}
	// make one more cell for last right pointer
	cells := make([]Cell, d.NumCell+1, d.NumCell+1) // pre-allocated?
	node := &Node{Parent: nil, Cells: cells, Used: 0}
	node.Cells = cells
	//log.Printf("*num cell: %v\n", d.NumCell)
	//log.Printf("*root node: %v\n", node)
	return node
}

// check if node is full of cells;
func (d *DB) isFull(node *Node) bool {
	return node.Used == d.NumCell
}

// Search :
func (d *DB) Search(key int) *Pair {
	found, cursor := recursiveSearch(key, d.root)
	if found {
		return getKeyVal(cursor)
	}
	return nil
}

func getKeyVal(cursor Cursor) *Pair {
	return &cursor.Node.Cells[cursor.Index].KeyVal
}

func setKeyVal(cursor Cursor, keyVal *Pair) {
	cursor.Node.Cells[cursor.Index].KeyVal = *keyVal
}

func getChildNode(cursor Cursor) *Node {
	return cursor.Node.Cells[cursor.Index].Child
}

func recursiveSearch(key int, node *Node) (bool, Cursor) {
	if node == nil {
		log.Panic("try to search a NIL node!")
	}
	found, cursor := binarySearch(node, key)
	if found { // found in current node;
		return true, cursor
	}
	child := getChildNode(cursor)
	if child != nil {
		return recursiveSearch(key, child)
	}
	return false, cursor // return nearest insert position
}

// if found, return cell and index;
// if not found, return nil and the position for insertion.
func binarySearch(node *Node, key int) (bool, Cursor) {
	if node == nil {
		log.Panic("try to search a NIL node.")
	}
	if node.Used == 0 {
		return false, Cursor{node, 0}
	}
	//fmt.Printf("search %v in node: %v\n", key, *node)
	l, r := 0, node.Used-1
	if key < node.Cells[l].KeyVal.Key {
		return false, Cursor{node, 0} // not found: Cells[l] move afterward
	}
	if key > node.Cells[r].KeyVal.Key {
		return false, Cursor{node, node.Used} // not found: just append
	}
	// binary search
	for l <= r {
		m := (l + r) / 2
		mKey := node.Cells[m].KeyVal.Key
		if mKey == key {
			return true, Cursor{node, m} // found
		} else if mKey < key {
			l = m + 1
		} else if mKey > key {
			r = m - 1
		}
	}
	// not found, in between r and l, insert at l;
	return false, Cursor{node, l}
}

var noDup = false

// Insert : a kv pair
func (d *DB) Insert(keyVal *Pair) (bool, error) {
	// find insert position
	found, cursor := recursiveSearch(keyVal.Key, d.root)
	if found && noDup { // found dup key
		kv := getKeyVal(cursor)
		log.Printf("found dup key: %v -> %v\n", *keyVal, *kv)
		setKeyVal(cursor, keyVal) // overwrite dup key
		return true, nil
	}
	if cursor.Node == nil {
		log.Panic("found invalid cursor!")
	}
	// insert this kv pair first to make it really full;
	ok, err := d.insertIntoNode(cursor, keyVal)
	if !ok {
		log.Printf("failed insertIntoNode - cursor:%v, kv:%v", cursor, keyVal)
		return false, err
	}
	return true, nil
}

// split on middle kv pair;
func (d *DB) split(node *Node) {
	if node.Used < 3 {
		log.Panicf("node - %v to be split should have at least 3 kv pairs.\n", node)
	}
	// split on middle kv pair
	mid := node.Used / 2
	keyVal := node.Cells[mid].KeyVal
	//node.Cells[mid].KeyVal = Pair{0, nil} // clear kv
	lNode := node
	rNode := d.NewNode()
	// copy right half of node to rNode
	j := 0
	for i := mid + 1; i <= node.Used; i++ { // include last ptr;
		rNode.Cells[j] = lNode.Cells[i]
		//lNode.Cells[i] = Cell{nil, Pair{0, nil}}
		j++
	}
	// update used
	lNode.Used -= j + 1 // include mid
	rNode.Used += j
	if node.Parent == nil { // split root node
		newRoot := d.NewNode()
		newRoot.Used = 1
		// set children and kv
		newRoot.Cells[0].Child = lNode   // left child
		newRoot.Cells[0].KeyVal = keyVal // key val
		newRoot.Cells[1].Child = rNode   // right child
		// set parent
		lNode.Parent, rNode.Parent, d.root = newRoot, newRoot, newRoot
		return
	}
	// insert kv into its parent node
	pNode := node.Parent
	// to find the exact cell that points to current node
	found, cursor := binarySearch(pNode, keyVal.Key)
	if !found && (cursor.Index == 0 && cursor.Index == cursor.Node.Used) {
		log.Panicf("key is not within range of node - %v, key - %v.\n", pNode, keyVal.Key)
	}
	ok, err := d.insertIntoNode(cursor, &keyVal)
	if !ok {
		log.Panicf("insertIntoNode failed, err: %v\n", err)
	}
	return
}

// insert kv into node
func (d *DB) insertIntoNode(cursor Cursor, kv *Pair) (bool, error) {
	node := cursor.Node
	if node == nil {
		log.Panic("try to insert into nil node!")
	}
	if d.isFull(node) {
		log.Printf("try to insert into a full node: %v, kv: %v", node, kv)
		log.Panic("insert into a full node.")
	}
	idx := cursor.Index
	// TODO	 check node.Cells[idx].Child == nil
	// move cells
	//log.Printf("+ node: %v\n", node)
	for i := node.Used + 1; i > idx; i-- {
		node.Cells[i] = node.Cells[i-1]
	}
	// set cell
	node.Cells[idx].KeyVal = *kv
	// set used
	node.Used++
	// check full
	if d.isFull(node) {
		d.split(node)
	}
	return true, nil
}
