#ifndef HEAP_H
#define HEAP_H
#include <iostream>
#include <vector>
#include <assert.h>
#include <windows.h>
using namespace std;
#pragma warning(disable:4996)
template<class T>
struct Less
{
	bool operator()(const T& left, const T& right)
	{
		return left < right;
	}
};

template<class T>
struct Greater
{
	bool operator()(const T& left, const T& right)
	{
		return left > right;
	}
};
template < class T, class compare = Less<T>>
class Heap
{
public:
	Heap()
	{}
	Heap(const T array[], size_t size)
	{
		_Heap.resize(size);
		for (size_t idx = 0; idx < size; idx++)
		{
			_Heap[idx] = array[idx];
		}
		int root = (_Heap.size() - 2) >> 1;
		for (; root < size; root--)
		{
			_adjustdown(root);
		}
	}
	void Insert(const T& data)
	{
		_Heap.push_back(data);
		if (_Heap.size()>1)
		{
			_adjustup();
		}
	}
	void Remove()
	{
		if (!Empty())
		{
			if (1 == _Heap.size())
			{
				_Heap.pop_back();
			}
			else
			{
				std::swap(_Heap[0], _Heap[_Heap.size() - 1]);
				_Heap.pop_back();
				_adjustdown(0);
			}
		}
	}

	size_t Size()const
	{
		return _Heap.size();
	}
	bool Empty()const
	{
		return _Heap.empty();
	}
	const T& Top()const
	{
		return _Heap[0];
	}
private:
	void _adjustdown(int Root) // 参数为每个元素的下标 ，把下标传过去让它去调整
	{
		size_t parent = Root;
		size_t child = 2 * parent + 1;
		size_t size = _Heap.size();
		while (child < size)
		{
			compare com;
			if (child + 1 < size&&com(_Heap[child + 1], _Heap[child]))
			{
				child +=  1;
			}
			if (com(_Heap[child], _Heap[parent]))
			{
				std::swap(_Heap[child], _Heap[parent]);
				parent = child;
				child = 2 * parent + 1;
			}
			else
			{
				return;
			}
		}
	}
	void _adjustup()
	{
		compare com;  // 直接用com 来比较 这样insert时 无论大堆还是小堆都处理了
		size_t child = _Heap.size() - 1;
		size_t parent = (child - 1) >> 1;
		while (child>0)
		{
			if (com(_Heap[child], _Heap[parent]))
			{
				std::swap(_Heap[child], _Heap[parent]);
				child = parent;
				parent = (child - 1) >> 1;
			}
			else
			{
				return;
			}
		}
	}
	vector<T> _Heap;
};
#endif
