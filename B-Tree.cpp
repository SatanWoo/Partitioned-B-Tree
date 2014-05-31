#include<iostream>
#include<vector>
using namespace std;

#definde BTreeDegree 3
typedef enum BTreeErrorCode 
{
	BTreeErrorCodeKeyNotFound = -1  
};

/************** Helper **************/
typedef struct 
{
	int key;
	int data;

	BTreeData(int k, int d)
	{
		key = k;
		data = d;
	}

	BTreeData()
	{
		key = -1;
		data = -1;
	}

	BTreeData clone()
	{
		return BTreeData(this->key， this->data);
	}

} BTreeData;

typedef struct 
{
	int start;
	int end;

	BTreeKeyRange(int s, int e)
	{
		start = s;
		end = e;
	}

	bool isInRange(int key)
	{
		return key >= start && key <= end;
	}
} BTreeKeyRange;
/************** End of Helper **************/


typedef struct
{
	int keyCount;
	bool isLeaf;

	BTreeData *indexes[2 * BTreeDegree - 1];
	BTreeNode **childs[2 * BTreeDegree];
	BTreeNode *parent;

	BTreeNode()
	{
		keyCount = 0;
		isLeaf = true;

		for (int i = 0; i < 2 * BTreeDegree - 1; i++)
		{
			indexes[i] = NULL;
		}

		for (int i = 0; i < 2 * BTreeDegree; i++)
		{
			childs[i] = NULL;
		}

		parent = NULL;
	}

	~BTreeNode()
	{
		keyCount = 0;
		parent = NULL;

		for (int i = 0; i < 2 * BTreeDegree - 1; i++)
		{
			if (indexes[i]) delete indexes[i];
			indexes = NULL;
		}

		for (int i = 0; i < 2 * BTreeDegree; i++)
		{
			if (childs[i]) delete childs[i];
			childs[i] = NULL;
		}
	}

	void addChild(BTreeNode *child, int pos)
	{
		if (this == NULL || child == NULL) return;
		if (pos >= 2 * BTreeDegree - 1 || pos < 0) return;
		if (this->keyCount >= 2 * BTreeDegree - 1) return;

		this->childs[pos] = child;
		child->parent = this;
		this->isLeaf = false;
	}

	int findDataOffset(int key, bool &isEqual)
	{
		for (int i = 0; i < keyCount; i++)
		{
			BTreeData *temp = indexes[i];
			if (temp->m_key == key) 
			{
				isEqual = true;
				return i;
			}
			else if (key < temp->m_key)
			{
				return i;
			}
		}
		return keyCount;
	}

} BTreeNode;

class BTree
{
public:
	BTree();
	~BTree();

	vector<BTreeData> getRange(BTreeKeyRange range);
	void put(int key ,int value);
	int get(int key);

	//void printBTree(); // Just for debug
private:
	BTreeNode *root;

	void splitNode(BTreeNode *nodeToSplit);
	void insertNode(int key, int value);
	void recursiveInsertNode(BTree *node, int key, int value);

	BTreeData *searchDataWithKey(BTreeNode *node, int key, int value);
};

/************** Public API **************/
BTree::BTree():root(new BTreeNode())
{}

BTree::~BTree()
{
	if (root == NULL) return;

	delete root; 
	root = NULL;
}

vector<BTreeData> BTree::getRange(BTreeKeyRange range)
{
	vector<BTreeData> result;
	for (int i = range.start; i <= range.end; i++)
	{
		BTreeData *existData = searchDataWithKey(root, i);
		if (existData != NULL) 
		{
			result.push_back(existData->clone());
		}
	}
	return result;
}

void BTree::put(int key, int value)
{
	BTreeData *existData = searchDataWithKey(root, key);
	if (existData != NULL) existData->data = value;

	insertNode(key, value);
}

int BTree::get(int key)
{
	BTreeData *existData = searchDataWithKey(root, key);
	if (existData != NULL) return existData->data;

	return BTreeErrorCodeKeyNotFound;
}
/************** End of Public API **************/


/************** Private API **************/
void BTree::splitNode(BTreeNode *nodeToSplit)
{
	BTreeNode *newNode = new BTreeNode();
	newNode->isLeaf = nodeToSplit->isLeaf;
	newNode->keyCount = BTreeDegree - 1;

	// Copy Indexed Data from right half of NodeToSplit
	for (int i = 0; i < newNode->keyCount; i++)
	{
		newNode->indexes[i] = nodeToSplit->indexes[i + BTreeDegree];
	}

	if (!nodeToSplit->isLeaf)
	{
		for (int i = 0; i < newNode->keyCount + 1; i++)
		{
			newNode->childs[i] = nodeToSplit->childs[i + BTreeDegree];
		}
	}

	BTreeData *dataToLift = nodeToSplit->indexes[BTreeDegree - 1];
	nodeToSplit->keyCount = BTreeDegree - 1；
	BTreeNode *parentNode = nodeToSplit->parent;

	int insertPos = 0;
	for (int i = 0; i < parentNode->keyCount; i++)
	{
		int key = parentNode->indexes[i]->key;
		if (dataToLift->key < key) break;

		insertPos ++;
	}

	// Move Data after pos by 1 offset
	for (int i = parentNode->keyCount - 1; i >= insertPos; i--)
	{
		parentNode->indexes[i + 1] = parentNode->indexes[i];
	}

	// Move Pointer after post by 1 offset
	for (int i = parentNode->keyCount; i > insertPos; i--)
	{
		parentNode->childs[i + 1] = parentNode->childs[i];
	}

	parentNode->indexes[insertPos] = dataToLift;
	parentNode->addChild(insertPos + 1, newNode);
	parentNode->keyCount += 1;
}

BTreeData* BTree::searchDataWithKey(BTreeNode *start, int key))
{
	if (start == NULL) return NULL;

	bool isEqual = false;
	int offset = start->findDataOffset(key, isEqual));

	if (isEqual)
	{
		return start->indexes[offset];
	} 
	else if (start->isLeaf)
	{
		return NULL;
	}
	else
	{
		return searchDataWithKey(start->childs[offset], key);
	}
}

void BTree::insertNode(int key, int value)
{
	if (node == NULL) return;

	if (node == root && node->keyCount >= 2 * BTreeDegree - 1)
	{
		BTreeNode *newRoot = new BTreeNode();
		newRoot->isLeaf = false;
		newRoot->keyCount = 0;
		newRoot->addChild(0, root);
		splitNode(root);
		root = newRoot;
	}

	recursiveInsertNode(root, key, value);
}

void BTree::recursiveInsertNode(BTreeNode *node ,int key, int value)
{
	// We can assure that the node for insert must be not full with key
	if (node->isLeaf)
	{
		int insertPos = 0;
		for (int i = 0; i < node->keyCount; i++)
		{
			int k = node->indexes[i]->key;
			if (key < k) break;
			insertPos ++;
		}

		// Move Data after pos by 1 offset
		for (int i = node->keyCount - 1; i >= insertPos; i--)
		{
			node->indexes[i + 1] = node->indexes[i];
		}

		BTreeData *newData = new BTreeData(key, value);
		node->indexes[insertPos] = newData;
		node->keyCount += 1;
	}
	else
	{
		int lookPos = 0;
		for (int i = 0; i < node->keyCount; i++)
		{
			int k = node->indexes[i]->key;
			if (key < k) break;
			lookPos ++;
		}

		BTreeNode *nextLookUpNode = node->childs[lookPos];
		if (nextLookUpNode->keyCount == 2 * BTreeDegree - 1)
		{
			splitNode(nextLookUpNode);
			if (lookPos > nextLookUpNode->keyCount || key > nextLookUpNode->indexes[lookPos]->key)
			{
				nextLookUpNode = node->childs[lookPos + 1];
			}
		}

		recursiveInsertNode(nextLookUpNode, key, value);
	}
}

int main()
{
	return 0;
}