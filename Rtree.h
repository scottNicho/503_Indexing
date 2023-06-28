#pragma once
#include "NPC.h"
#include <vector>
#include <utility>
#include <algorithm>
#include "BPtree.h"

namespace NCL::CSC8503 {

	struct BoundingBox
	{
		BoundingBox(Vector2& BLeft, Vector2& TRight):minLeftPoint(BLeft), topRightPoint(TRight) {}

		const Vector2& getBottomLeft() {
			return minLeftPoint;
		}

		const Vector2& getTopRight() {
			return minLeftPoint;
		}

		Vector2 GetCentre() {
			return(minLeftPoint + topRightPoint) * 0.5;
		}

		float MinimumSizeIncrease(BoundingBox newBox) {
			float increaseSize = 0.0f;
			if (newBox.topRightPoint.x > topRightPoint.x) {
				increaseSize += abs(newBox.topRightPoint.x - topRightPoint.x);
			}
			else if(newBox.minLeftPoint.x < minLeftPoint.x){
				increaseSize += abs(newBox.minLeftPoint.x - minLeftPoint.x);
			}

			if (newBox.topRightPoint.y > topRightPoint.y) {
				increaseSize += abs(newBox.topRightPoint.y - topRightPoint.y);
			}
			else if (newBox.minLeftPoint.y < minLeftPoint.y) {
				increaseSize += abs(newBox.minLeftPoint.y - minLeftPoint.y);
			}

			return increaseSize;
		}

	private:

	Vector2 minLeftPoint;
	Vector2 topRightPoint;

	};

	struct RTreeDataPoint {
		BoundingBox bounds;
		int npc_ID;
		RTreeNode* childNode;
		RTreeDataPoint(BoundingBox& box,int ID,RTreeNode* child = nullptr):bounds(box),childNode(child), npc_ID(ID) {}
	};

	struct RTreeNode
	{
		bool isLeaf;
		std::vector<RTreeDataPoint> points;
		RTreeNode* parent;
		std::vector<RTreeNode*> childNodes;

		RTreeNode(bool leaf = false): isLeaf(leaf), parent(nullptr) {}
	};

	class Rtree {
	public:
		Rtree(int maxPerNode):maxE(maxPerNode){
			root = new RTreeNode(true);
		}

		~Rtree(){}

		void Insert(NPC* npc) {
			BoundingBox box = npc->setBoundingBox();
			InsertE(root, box, npc->GetID());
		}

	private:
		int maxE;
		RTreeNode* root;
		BPtree<NPC> bPlusTree;

		void InsertE(RTreeNode* node, BoundingBox box, int npc_id) {
			if (node->isLeaf) {
				node->points.emplace_back(box,npc_id);
				if (node->points.size() > maxE) {
					//insert split npde function
				}
			}
			else
			{
				RTreeNode* child = nullptr;
				float minE = std::numeric_limits<float>::max();
				for (auto aln : node->points) {
					float increase = aln.bounds.MinimumSizeIncrease(box);
					if (increase < minE) {
						minE = increase;
						child = aln.childNode;
					}
				}
				if (child) {
					InsertE(child, box, npc_id);
				}
				else
				{
					RTreeNode* newNode = new RTreeNode(true);
					node->points.emplace_back(box, npc_id, newNode);
					newNode->parent = node;
					node->childNodes.push_back(newNode);

					if (node->points.size() > maxE) {
						//SplitLeafNode(node);
					}
				}
			}
		}

		void splitLeaf(RTreeNode* node) {
			RTreeNode* newNode = new RTreeNode(true);

			int midP = node->points.size() / 2;
			for (int i = midP; i < node->points.size(); ++i) {
				newNode->points.push_back(node->points[i]);
			}

			node->points.erase(node->points.begin() + midP, node->points.end());

			if (node == root) {
				RTreeNode* newRoot = new RTreeNode(false);
				for (const auto& xi : node->points) {
					newRoot->points.emplace_back(xi.bounds, nullptr, xi.childNode);
					xi.childNode->parent = newRoot;
				}

				for (const auto& xi : newNode->points) {
					newRoot->points.emplace_back(xi.bounds, nullptr, xi.childNode);
					xi.childNode->parent = newRoot;
				}

				root = newRoot;
			}
			else
			{
				RTreeNode* parent = node->parent;

				for (const auto& xi : newNode->points) {
					parent->points.emplace_back(xi.bounds, nullptr, xi.childNode);
					xi.childNode->parent = parent;
					parent->childNodes.push_back(xi.childNode);
				}
			}
		}

	};

}