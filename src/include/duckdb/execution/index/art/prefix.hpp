//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/execution/index/art/prefix.hpp
//
//
//===----------------------------------------------------------------------===//
#pragma once

#include "duckdb/execution/index/art/art.hpp"
#include "duckdb/execution/index/art/node.hpp"

namespace duckdb {

// classes
class ARTKey;

//! The Prefix is a special node type that contains up to PREFIX_SIZE bytes, one byte for the count,
//! and a Node pointer. This pointer either points to another prefix
//! node or the 'actual' ART node.
class Prefix {
public:
	//! Up to seven bytes of prefix data and the count
	uint8_t data[Node::PREFIX_SIZE + 1];
	//! A pointer to the next ART node
	Node ptr;

public:
	//! Get a new empty prefix node, might cause a new buffer allocation
	static Prefix &New(ART &art, Node &node);
	//! Create a new prefix node containing a single byte and a pointer to a next node
	static Prefix &New(ART &art, Node &node, uint8_t byte, Node next);
	//! Get a new chain of prefix nodes, might cause new buffer allocations,
	//! with the node parameter holding the tail of the chain
	static void New(ART &art, reference<Node> &node, const ARTKey &key, const uint32_t depth, uint32_t count);
	//! Free the node (and its subtree)
	static void Free(ART &art, Node &node);
	//! Get a reference to the prefix
	static inline Prefix &Get(const ART &art, const Node ptr) {
		return *Node::GetAllocator(art, NType::PREFIX).Get<Prefix>(ptr);
	}

	//! Initializes a merge by incrementing the buffer ID of the child node
	inline void InitializeMerge(ART &art, const ARTFlags &flags) {
		ptr.InitializeMerge(art, flags);
	}

	//! Appends a byte and a child_prefix to prefix. If there is no prefix, than it pushes the
	//! byte on top of child_prefix. If there is no child_prefix, then it creates a new
	//! prefix node containing that byte
	static void Concatenate(ART &art, Node &prefix_node, const uint8_t byte, Node &child_prefix_node);
	//! Traverse prefixes until (1) encountering two non-prefix nodes,
	//! (2) encountering one non-prefix node, (3) encountering a mismatching byte.
	//! Also frees all fully traversed r_node prefixes
	static idx_t Traverse(ART &art, reference<Node> &l_node, reference<Node> &r_node);
	//! Traverse a prefix and a key until (1) encountering a non-prefix node, or (2) encountering
	//! a mismatching byte, in which case depth indexes the mismatching byte in the key
	static idx_t Traverse(const ART &art, reference<Node> &prefix_node, const ARTKey &key, idx_t &depth);
	//! Returns the byte at position
	static inline uint8_t GetByte(const ART &art, const Node &prefix_node, const idx_t position) {
		auto prefix = Prefix::Get(art, prefix_node);
		D_ASSERT(position < Node::PREFIX_SIZE);
		D_ASSERT(position < prefix.data[Node::PREFIX_SIZE]);
		return prefix.data[position];
	}
	//! Removes the first n bytes from the prefix and shifts all subsequent bytes in the
	//! prefix node(s) by n. Frees empty prefix nodes
	static void Reduce(ART &art, Node &prefix_node, const idx_t n);
	//! Splits the prefix at position. prefix then references the ptr (if any bytes left before
	//! the split), or stays unchanged (no bytes left before the split). child references
	//! the node after the split, which is either a new Prefix node, or ptr
	static void Split(ART &art, reference<Node> &prefix_node, Node &child_node, idx_t position);

	//! Serialize this node
	BlockPointer Serialize(ART &art, MetaBlockWriter &writer);
	//! Deserialize this node
	void Deserialize(MetaBlockReader &reader);

	//! Vacuum the child of the node
	inline void Vacuum(ART &art, const ARTFlags &flags) {
		Node::Vacuum(art, ptr, flags);
	}

private:
	//! Appends the byte to this prefix node, or creates a subsequent prefix node,
	//! if this node is full
	Prefix &Append(ART &art, const uint8_t byte);
	//! Appends the other_prefix_node and all its subsequent prefix nodes to this prefix node.
	//! Also frees all copied/appended nodes
	Prefix &Append(ART &art, Node &other_prefix_node);
};

} // namespace duckdb
