#include "StdAfx.h"
#include "USkeletonNode.h"
#include "Utility.h"


USkeletonNode::USkeletonNode(void)
{
	count = 1;
}

USkeletonNode::USkeletonNode(int _id, CVector3 _point, USkeletonNode* _parent) {
	id = _id;
	count = 1;
	point = _point;
	parent = _parent;
	parentDist = Length(point - parent->point);
	parent->nodes.push_back(this);
}

USkeletonNode::USkeletonNode(int _id, CVector3 _point, float _parentDist, USkeletonNode* _parent) {
	id = _id;
	count = 1;
	point = _point;
	parent = _parent;
	parentDist = _parentDist;
	//parent->nodes.push_back(this);
}

USkeletonNode::USkeletonNode(USkeletonNode* root, USkeletonNode* addRoot, float _parentDist) {
	id = root->id;
	count = 1;
	point = addRoot->point;
	parent = root;
	parentDist = _parentDist;
	parent->nodes.push_back(this);

	for (int i = 0; i < addRoot->nodes.size(); i++) {
		USkeletonNode* node = new USkeletonNode(this, addRoot->nodes[i], addRoot->nodes[i]->parentDist);
	}
}

/*USkeletonNode::USkeletonNode(USkeletonNode* root, USkeletonNode* addRoot) {
	id = root->id;
	point = addRoot->point;
	parent = root;
	parentDist = Length(point - parent->point);
	parent->nodes.push_back(this);

	for (int i = 0; i < addRoot->nodes.size(); i++) {
		USkeletonNode* node = new USkeletonNode(this, addRoot->nodes[i]);
	}
}*/

USkeletonNode::USkeletonNode(SkeletonGraph* G, int _id) {
	if (_id == -1) _id = G->branchNodes[0]->id;
	parent = NULL;
	id = _id;
	point = G->nodes[_id]->point;
	parentDist = 0;
	count = 1;

	for (int i = 0; i < G->nodes[_id]->edges.size(); i++) {
		USkeletonNode* node = SkeletonNodesFromEdge(G->nodes[_id]->edges[i], this);
		new USkeletonNode(G, node, G->nodes[_id]->neighborhood[i]->id, id);
	}
}

USkeletonNode::USkeletonNode(SkeletonGraph* G, USkeletonNode* root, int gid, int skipId) {
	parent = root;
	root->nodes.push_back(this);
	id = gid;
	point = G->nodes[gid]->point;
	parentDist = Length(point - parent->point);
	count = 1;

	for (int i = 0; i < G->nodes[gid]->edges.size(); i++) {
		if (G->nodes[gid]->neighborhood[i]->id == skipId) continue;
		USkeletonNode* node = SkeletonNodesFromEdge(G->nodes[gid]->edges[i], this);
		new USkeletonNode(G, node, G->nodes[gid]->neighborhood[i]->id, id);
	}
}

USkeletonNode* USkeletonNode::SkeletonNodesFromEdge(GraphEdge ge, USkeletonNode* root) {
	bool reverse = (ge.fromId != root->id);
	int toId = reverse ? ge.fromId : ge.toId;
	USkeletonNode* croot = root;
	count = 1;

	for (int i = 0; i < ge.positions.size(); i++) {
		int idx = reverse ? (ge.positions.size() - i - 1) : i;
		USkeletonNode* node = new USkeletonNode();
		node->id = toId;
		node->point = ge.positions[idx];
		node->parent = croot;
		node->parentDist = Length(node->point - croot->point);

		croot->nodes.push_back(node);

		croot = node;
	}

	return croot;
}
//other is intersection skeleton that has all nodes mapped to other nodes
USkeletonNode::USkeletonNode(SkeletonGraph* G, USkeletonNode* other, vector<int> mapping) {
	id = mapping[other->id];
	parent = NULL;
	parentDist = 0;
	point = G->nodes[id]->point;
	count = 1;

	vector<int> used;
	for (int i = 0; i < other->nodes.size(); i++) {
		USkeletonNode* ochild = other->nodes[i];
		int idx = mapping[ochild->id];
		GraphEdge ge;
		if (G->AreNeighbors(id, idx, ge)) {
			used.push_back(idx);
			USkeletonNode* node = SkeletonNodesFromEdge(ge, this);
			new USkeletonNode(G, node, SkipSameIds(other->nodes[i]), mapping, id);
		}
	}
	for (int i = 0; i < G->nodes[id]->neighborhood.size(); i++) {
		if (!inArray(used, G->nodes[id]->neighborhood[i]->id)) {
			USkeletonNode* node = SkeletonNodesFromEdge(G->nodes[id]->edges[i], this);
			new USkeletonNode(G, node, G->nodes[id]->neighborhood[i]->id, id);
		}
	}
}

USkeletonNode::USkeletonNode(SkeletonGraph* G, USkeletonNode* root, USkeletonNode* other, vector<int> mapping, int skipId) {
	id = mapping[other->id];
	parent = root;
	root->nodes.push_back(this);
	point = G->nodes[id]->point;
	parentDist = Length(root->point - point);
	count = 1;

	vector<int> used;
	for (int i = 0; i < other->nodes.size(); i++) {
		USkeletonNode* ochild = other->nodes[i];
		int idx = mapping[ochild->id];
		GraphEdge ge;
		if (G->AreNeighbors(id, idx, ge)) {
			used.push_back(idx);
			USkeletonNode* node = SkeletonNodesFromEdge(ge, this);
			new USkeletonNode(G, node, SkipSameIds(other->nodes[i]), mapping, id);
		}
	}
	for (int i = 0; i < G->nodes[id]->neighborhood.size(); i++) {
		if (!inArray(used, G->nodes[id]->neighborhood[i]->id) && G->nodes[id]->neighborhood[i]->id != skipId) {
			USkeletonNode* node = SkeletonNodesFromEdge(G->nodes[id]->edges[i], this);
			new USkeletonNode(G, node, G->nodes[id]->neighborhood[i]->id, id);
		}
	}
}

USkeletonNode::~USkeletonNode(void)
{
	for (int i = 0; i < nodes.size(); i++) {
		delete nodes[i];
	}
}

void USkeletonNode::SetParent(USkeletonNode* node) {
	parent->RemoveChild(this);
	parent = node;
	parent->nodes.push_back(this);
	parentDist = Length(point - parent->point);
}

void USkeletonNode::RemoveChild(USkeletonNode* node) {
	for (vector<USkeletonNode *>::iterator it = nodes.begin(); it != nodes.end(); it++) {
		if ((*it) == node) {
			it = nodes.erase(it);
			return;
		}
	}
}

bool USkeletonNode::ReplaceChild(USkeletonNode* child, USkeletonNode* node) {
	for (int i = 0; i < nodes.size(); i++) {
		if (nodes[i] == child) {
			nodes[i] = node;
			return true;
		}
	}

	return false;
}

SkeletonNode* USkeletonNode::ToSkeletonNode() {
	SkeletonNode* skl = new SkeletonNode();
	skl->id = id;
	skl->point = point;
	skl->cyclic = count;

	for (int i = 0; i < nodes.size(); i++) {
		SkeletonNode* nskl = nodes[i]->ToSkeletonNode();
		nskl->father = skl;
		skl->nodes.push_back(nskl);
	}

	return skl;
}

void USkeletonNode::CalculateCorrespondingDoF(USkeletonNode* bind, float threshold) {
	axisAngles.push_back(CVector4(0, 0, 0, 1));
	for (int i = 0; i < nodes.size(); i++) {
		if (i < bind->nodes.size()) {
			nodes[i]->CalculateCorrespondingDoF(bind->nodes[i], glm::mat4(1.0), threshold);
		}
	}
}

void USkeletonNode::CalculateCorrespondingDoF(USkeletonNode* bind, glm::mat4 M, float threshold) {
	//have to be the same skeleton just posed differently
	//this is in bind position
	CVector4 axisAngle = CVector4(0, 0, 0, 1);

	//rotation from bind to this skeleton
	CVector3 bPoint = TransformCPoint(bind->point, M);
	CVector3 bpPoint = TransformCPoint(bind->parent->point, M);
	CVector3 u = Normalize(bPoint - bpPoint);
	CVector3 v = Normalize(point - parent->point);

	Quaternion q = MPQuaternionBetweenVectors(u, v);
	axisAngle = QuaternionToAxisAngle(q);
	axisAngle.s = axisAngle.s*180.0f/M_PI;

	//if (axisAngle.s*180.0f/M_PI > threshold) {
	if (axisAngle.s > threshold) {
		AddRotation(M, axisAngle, parent->point);
	} else {
		axisAngle = CVector4(0, 0, 0, 1);
	}
	axisAngles.push_back(axisAngle);

	for (int i = 0; i < nodes.size(); i++) {
		if (i < bind->nodes.size()) {
			nodes[i]->CalculateCorrespondingDoF(bind->nodes[i], M, threshold);
		}
	}
}

USkeletonNode* SkipSameIds(USkeletonNode* node) {
	int id = node->id;
	while (node->nodes.size() == 1 && node->nodes[0]->id == id) {
		node = node->nodes[0];
	}

	return node;
}

/*void AddSkeleton(USkeletonNode* oNode, USkeletonNode* aNode, USkeletonNode* root, vector<int> mapping, float lthreshold) {
	//no more to add
	if (aNode == NULL) return;
	//only nodes to add remain
	if (oNode == NULL) {
		USkeletonNode* node = new USkeletonNode(root, aNode);
		return;
	}
	//add closer node
	float len = Length(aNode->point - root->point);
	//if within threshold don't add and skip
	if (fabs(oNode->parentDist - len) < lthreshold) {
		//move both
		for (int i = 0; i < aNode->nodes.size(); i++) {
			if (i < oNode->nodes.size()) {
				AddSkeleton(oNode->nodes[i], aNode->nodes[i], oNode, mapping, lthreshold);
			} else {
				AddSkeleton(NULL, aNode->nodes[i], oNode, mapping, lthreshold);
			}
		}
	} else if (oNode->parentDist < len) {//if oNode is closer add just skip
		//move oNode
		if (oNode->nodes.size() > 0) {
			AddSkeleton(oNode->nodes[0], aNode, oNode, mapping, lthreshold);
		} else {
			AddSkeleton(NULL, aNode, oNode, mapping, lthreshold);
		}
	} else {//if aNode is closer add new node
		vector<USkeletonNode *> nodes = root->nodes;
		root->nodes.clear();
		int idx = nodes.size() == 0 ? root->id : nodes[0]->id;
		USkeletonNode* node = new USkeletonNode(idx, aNode->point, root);
		node->nodes = nodes;
		//move aNode
		if (aNode->nodes.size() == 0) {
			AddSkeleton(oNode, NULL, node, mapping, lthreshold);
		} else {
			AddSkeleton(oNode, aNode->nodes[0], node, mapping, lthreshold);
			for (int i = 1; i < aNode->nodes.size(); i++) {
				AddSkeleton(NULL, aNode->nodes[i], node, mapping, lthreshold);
			}
		}
	}

}*/

void AddSkeleton(USkeletonNode* oNode, USkeletonNode* aNode, vector<int> mapping, float lthreshold) {
	//move both
	for (int i = 0; i < aNode->nodes.size(); i++) {
		if (i < oNode->nodes.size()) {
			AddSkeleton(oNode->nodes[i], oNode->nodes[i]->parentDist, aNode->nodes[i], aNode->nodes[i]->parentDist, oNode, mapping, lthreshold);
		} else {
			AddSkeleton(NULL, 0, aNode->nodes[i], aNode->nodes[i]->parentDist, oNode, mapping, lthreshold);
		}
	}
}

void AddSkeleton(USkeletonNode* oNode, float oDist, USkeletonNode* aNode, float aDist, USkeletonNode* root, vector<int> mapping, float lthreshold) {
	//no more to add
	if (aNode == NULL) return;
	//only nodes to add remain
	if (oNode == NULL) {
		USkeletonNode* node = new USkeletonNode(root, aNode, aDist);
		return;
	}
	//add closer node
	//if within threshold don't add and skip
	if (fabs(oDist - aDist) < lthreshold) {
		//move both
		aNode->count++;
		for (int i = 0; i < aNode->nodes.size(); i++) {
			if (i < oNode->nodes.size()) {
				AddSkeleton(oNode->nodes[i], oNode->nodes[i]->parentDist, aNode->nodes[i], aNode->nodes[i]->parentDist, oNode, mapping, lthreshold);
			} else {
				AddSkeleton(NULL, 0, aNode->nodes[i], aNode->nodes[i]->parentDist, oNode, mapping, lthreshold);
			}
		}
	} else if (oDist < aDist) {//if oNode is closer add just skip
		//move oNode
		if (oNode->nodes.size() > 0) {
			AddSkeleton(oNode->nodes[0], oNode->nodes[0]->parentDist, aNode, aDist - oDist, oNode, mapping, lthreshold);
		} else {
			AddSkeleton(NULL, 0, aNode, aDist - oDist, oNode, mapping, lthreshold);
		}
	} else {//if aNode is closer add new node
		int idx = oNode->id;
		CVector3 dir = Normalize(oNode->point - root->point);
		//USkeletonNode* node = new USkeletonNode(idx, aNode->point, aDist, root);
		USkeletonNode* node = new USkeletonNode(idx, root->point + dir*aDist, aDist, root);
		root->ReplaceChild(oNode, node);
		node->nodes.push_back(oNode);
		oNode->parent = node;
		oNode->parentDist -= aDist;
		//move aNode
		if (aNode->nodes.size() == 0) {
			AddSkeleton(oNode, oDist - aDist, NULL, 0, node, mapping, lthreshold);
		} else {
			AddSkeleton(oNode, oDist - aDist, aNode->nodes[0], aNode->nodes[0]->parentDist, node, mapping, lthreshold);
			for (int i = 1; i < aNode->nodes.size(); i++) {
				AddSkeleton(NULL, 0, aNode->nodes[i], aNode->nodes[i]->parentDist, node, mapping, lthreshold);
			}
		}
	}
}
