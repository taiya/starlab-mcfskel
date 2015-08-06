#pragma once

#include <QList>
#include "VertexRecord.h"

class PriorityQueue{

public:
	QList<VertexRecord*> data;

	bool IsEmpty()
	{
		return (data.size()==0);
	}
	void Clear()
	{
		data.clear();
	}

	void Insert(VertexRecord* obj)
	{
		int hole = data.size();
		data.push_back(obj);
		while (hole>0 && obj->CompareTo(*data[(hole-1)/2])<0)
		{
			data[hole] = data[(hole-1)/2];
			data[hole]->PQIndex = hole;
			hole = (hole-1) / 2;
		}
		data[hole] = obj;
		obj->PQIndex = hole;
	}
	VertexRecord* DeleteMin()
	{
		VertexRecord* obj = data[0];

		data[0] = data[data.size()-1];
		data.removeAt(data.size()-1);
		if (data.size() > 0)
		{
			data[0]->PQIndex = 0;
			PercolateDown(0);
		}

		return obj;
	}
	VertexRecord* GetMin()
	{
		return data[0];
	}

	void Update(VertexRecord* obj)
	{
		PercolateUp(obj->PQIndex);
		PercolateDown(obj->PQIndex);
	}

	void PercolateUp(int hole)
	{
		VertexRecord* obj = data[hole];
		while (hole > 0 && obj->CompareTo(*data[(hole - 1) / 2]) < 0)
		{
			int parent = (hole - 1) / 2;
			VertexRecord* parentObj = data[parent];
			data[hole] = data[parent];
			parentObj->PQIndex = hole;
			hole = parent;
		}
		data[hole] = obj;
		obj->PQIndex = hole;
	}
	void PercolateDown(int hole)
	{
		VertexRecord* obj = data[hole];
		while ((hole*2+1) < data.size())
		{
			int child = hole*2 + 1;
			if (child != data.size()-1)
			{
				if (data[child+1]->CompareTo(*data[child])<0)
					child++;
			}
			VertexRecord* childObj = data[child];
			if (childObj->CompareTo(*obj) < 0)
			{
				data[hole] = childObj;
				childObj->PQIndex = hole;
				hole = child;
			}
			else break;
		}
		data[hole] = obj;
		obj->PQIndex = hole;
	}
};