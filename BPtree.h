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
				numKeys = 0;
				parent = nullptr;
				children = std::vector<BPT_Node*>(order, nullptr);
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

		void InsertIntoTree(keyType& key) {
			BPT_Node current = root;
			if (current.nodeFull()) {
				BPT_Node* newNode = new BPT_Node();
				newNode->children[0] = current;
				root = newNode;

				splitNode(newNode,0);
				InsertOpen(newNode,key);
			}
			else
			{
				InsertOpen(current,key);
			}
		}

		void TotalClear() {
			Clear(root);
			root->numKeys = 0;
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
				unsigned int xi = node->numKeys;
				while (xi > 0 && newKey < node->Keys[xi - 1]) {
					xi--;
				}
				BPT_Node* childNode = node->children[xi]; //get the child to go too

				if (childNode->nodeFull()) {
					splitNode(node,xi);
					if (newKey > node->Keys[xi]) {
						xi++;
						childNode = node->children[xi];
					}
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
				for (size_t i = 0; i <= middle; ++i) {
					newSplitNode->children[i] = childNode->children[i + middle + 1];//add correct pointers
					childNode->children[i + middle + 1] = nullptr;//set pointers that have been moved to the split node
				}
			}
			childNode->numKeys = middle;

			for (int i = parent->numKeys; i > childIndex; --i) {
				parent->children[i + 1] = parent->children[i];
			}

			parent->children[childIndex + 1] = newSplitNode;

			for (int xi = parent->numKeys; xi > childIndex; --xi) {
				parent->keys[xi] = parent->keys[xi - 1];
			}
			parent->Keys[childIndex] = middleKey;
			parent->numKeys += 1;
		}

		void Clear(BPT_Node* node) {
			if (node.isLeaf) { return;}
			for (int h = 0; h <= node->numKeys ; h++) {
				Clear(node->children[h]);
				delete node->children[h];
			}
				
		}

	};

}