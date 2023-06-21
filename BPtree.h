#pragma once
#include <iostream>
#include <vector>

namespace NCL::CSC8503 {
	template<typename keyType>
	class BPtree {
	public:

		struct BPT_Node {
			bool leaf;
			size_t numKeys;
			std::vector<keyType> Keys;
			BPT_Node* parent;
			std::vector<BPT_Node*> children;

			BPT_Node(bool isLeaf = false) {
				leaf = isLeaf;
				numKeys(0);
				parent(nullptr);
				children(Order, nullptr)
				Keys = {};
			}

			//update functions

			bool nodeFull() const {
				return numKeys >= BPT_Node::order;
			}

			bool toFew() const {
				return BPT_Node::numKeys < (order - 1) / 2;
			}

			bool isLeaf() {
				return BPT_Node::leaf;
			}
		};

	

		BPtree() {
			root = new BPT_Node();
		}

		~BPtree() {
			delete root;
		}

	protected:
		size_t order;

	private:
		BPT_Node* root;

		void insertInLeaf(BPT_Node* leaf,keyType &newKey) {
			unsigned int xi = leaf->numKeys;
			while (xi>0 && newKey < leaf->Keys[xi - 1])
			{
				leaf->Keys[xi] = leaf[xi - 1]; // move the values along as we are inserting a new one
				xi-- ;
			}
			leaf->Keys[xi] = newKey;
			leaf->numKeys += 1;
		}

		void InsertOpen(BPT_Node* node, keyType &newKey) {
			if (node->isLeaf()) { insertInLeaf(node,newKey); }
			else
			{
				unsigned int xi = leaf->numKeys;
				while (xi > 0 && newKey < node->Keys[xi - 1]) {
					xi--;
				}
				BPT_Node* childNode = node->children[xi]; //get the child to go too

				if (ChildNode->nodeFull()) {
					//fill in with node splitting operations 
				}
				InsertOpen(childNode, newKey);
			}
		}

		void splitNode(BPT_Node* parent,unsigned int childIndex) {
			BPT_Node* childNode = parent->children[childIndex];
			BPT_Node* newSplitNode = new BPT_Node();

			unsigned int middle = (order - 1) / 2;
			keyType middleKey = childNode->Keys[middle];

			newSplitNode->numKeys = middle;
			for (int q = 0; q < middle; q++) {
				newSplitNode->Keys[q] = childNode->Keys[q + middle + 1];//add keys to the newly created node
			}

			if (!(childNode->isLeaf())) {
				for (size_t i = 0; i <= midIndex; ++i) {
					newNode->children[i] = childNode->children[i + middle + 1];//add correct pointers
					childNode->children[i + middle + 1] = nullptr;//set pointers that have been moved to the split node
				}
			}
			childNode->numKeys = middle;

			for (int i = parent->numKeys; i > childIndex; --i) {
				parentNode->children[i + 1] = parentNode->children[i];
			}

			parent->children[childIndex + 1] = newNode;

			for (int xi = parent->numKeys; xi > childIndex; --xi) {
				parent->keys[xi] = parent->keys[xi - 1];
			}
			parent->Keys[childIndex] = middleKey;
			parent->numKeys += 1;
		}

	};

}