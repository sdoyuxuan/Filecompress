#include "heap.h"

template<class T>
struct HuffmanTreeNode
{
	HuffmanTreeNode(const T& weight)
	: _weight(weight)
	, _pLeft(NULL)
	, _pRight(NULL)
	, _pParent(NULL)
	{}

	T _weight;         // Ȩֵ
	HuffmanTreeNode<T>* _pLeft;
	HuffmanTreeNode<T>* _pRight;
	HuffmanTreeNode<T>* _pParent;
};
template<class T>
class HuffmanTree
{
	typedef HuffmanTreeNode<T> Node;
public:
	HuffmanTree()
		: _pRoot(NULL)
	{}

	HuffmanTree(const T array[], size_t size, const T& invalid)
	{
		_Create(array, size, invalid);
	}
	~HuffmanTree()
	{
		_Destroy(_pRoot);
	}
	const Node* Root()const
	{
		return _pRoot;
	}
private:
	void _Create(const T array[], size_t size, const T& invalid)
	{
		struct compare
		{
			bool operator()(const Node * left, const Node* right)
			{
				return left->_weight < right->_weight;
			}
		};
		Heap <Node*, compare>hp;
		for (size_t idx = 0; idx < size; ++idx)
		{
              if(array[idx]!=invalid)hp.Insert(new Node(array[idx]));
		}
		while (hp.Size()>1)
		{
			Node * left = hp.Top();
			hp.Remove();
			Node * Right = hp.Top();
			hp.Remove();
			Node* parent = new Node(left->_weight + Right->_weight);
			left->_pParent = parent;
			Right->_pParent = parent;
			parent->_pLeft = left;
			parent->_pRight = Right;
			hp.Insert(parent);
		}
		_pRoot = hp.Top();
	}
	void _Destroy(Node* & pRoot)
	{
		if (pRoot)
		{
			_Destroy(pRoot->_pLeft);
			_Destroy(pRoot->_pRight);
			delete pRoot;
			pRoot = NULL;
		}
	}

protected:
	Node* _pRoot;
};
