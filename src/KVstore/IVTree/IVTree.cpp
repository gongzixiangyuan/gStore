/*=============================================================================
# Filename: IVTree.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:45
# Description: achieve functions in IVTree.h
=============================================================================*/

#include "IVTree.h"

using namespace std;

IVTree::IVTree()
{
	height = 0;
	mode = "";
	root = NULL;
	leaves_head = NULL;
	leaves_tail = NULL;
	TSM = NULL;
	storepath = "";
	filename = "";
	//transfer_size[0] = transfer_size[1] = transfer_size[2] = 0;
	//transfer_size = 0;
	this->stream = NULL;
	this->request = 0;
	this->value_list = NULL;
}

IVTree::IVTree(string _storepath, string _filename, string _mode, unsigned long long _buffer_size)
{
	storepath = _storepath;
	filename = _filename;
	this->height = 0;
	this->mode = string(_mode);
	string filepath = this->getFilePath();

	string vlist_file = filepath + "_vlist";
	this->value_list = new VList(vlist_file, this->mode, 1<<30);

	TSM = new IVStorage(filepath, this->mode, &this->height, _buffer_size, this->value_list);
	if (this->mode == "open")
	{
		this->TSM->preRead(this->root, this->leaves_head, this->leaves_tail);
		//cout << "IVTree: begin preLoad..." << endl;
		this->TSM->preLoad(this->root);
		//this->TSM->fullLoad(this->root);
		//cout << "IVTree: finish preLoad." << endl;
	}
	else
		this->root = NULL;

	//this->transfer[0].setStr((char*)malloc(Util::TRANSFER_SIZE));
	//this->transfer[1].setStr((char*)malloc(Util::TRANSFER_SIZE));
	//this->transfer[2].setStr((char*)malloc(Util::TRANSFER_SIZE));
	//this->transfer_size[0] = this->transfer_size[1] = this->transfer_size[2] = Util::TRANSFER_SIZE;		//initialied to 1M
	//this->transfer.setStr((char*)malloc(Util::TRANSFER_SIZE));

	this->stream = NULL;
	this->request = 0;
}

string
IVTree::getFilePath()
{
	return storepath + "/" + filename;
}

//void			//WARN: not check _str and _len
//IVTree::CopyToTransfer(const char* _str, unsigned _len, unsigned _index)
//{
	//if (_index > 2)
		//return;
	//[>
	//if(_str == NULL || _len == 0)
	//{
	//printf("error in CopyToTransfer: empty string\n");
	//return;
	//}
	//*/
	////unsigned length = _bstr->getLen();
	//unsigned length = _len;
	//if (length + 1 > this->transfer_size[_index])
	//{
		//transfer[_index].release();
		//transfer[_index].setStr((char*)malloc(length + 1));
		//this->transfer_size[_index] = length + 1;	//one more byte: convenient to add \0
	//}
	//memcpy(this->transfer[_index].getStr(), _str, length);
	//this->transfer[_index].getStr()[length] = '\0';	//set for string() in KVstore
	//this->transfer[_index].setLen(length);
//}

unsigned
IVTree::getHeight() const
{
	return this->height;
}

void
IVTree::setHeight(unsigned _h)
{
	this->height = _h;
}

IVNode*
IVTree::getRoot() const
{
	return this->root;
}

void
IVTree::prepare(IVNode* _np)
{
	//this->request = 0;
	bool flag = _np->inMem();
	if (!flag)
	{
		this->TSM->readNode(_np, &request);	//readNode deal with request
	}
}

bool
IVTree::search(unsigned _key, char*& _str, unsigned& _len)
{
	//if (_key < 0)
	//{
		//printf("error in IVTree-search: empty string\n");
		//return false;
	//}

	this->request = 0;
	int store = -1;
	IVNode* ret = this->find(_key, &store, false);
	if (ret == NULL || store == -1 || _key != ret->getKey(store))	//tree is empty or not found
	{
		return false;
	}

	ret->getValue(this->value_list, store, _str, _len);
	//const Bstr* val = ret->getValue(store);
	//this->CopyToTransfer(val->getStr(), val->getLen(), 0);		//not sum to request
	//_str = this->transfer[0].getStr();
	//_len = this->transfer[0].getLen();

	if (!VList::isLongList(_len))
	{
		char* debug = new char [_len];
		memcpy(debug, _str, _len);
		_str = debug;
	}

	this->TSM->request(request);
	return true;
}

bool
IVTree::insert(unsigned _key, char* _str, unsigned _len)
{
	//if (_key < 0)
	//{
		//printf("error in IVTree-insert: empty string\n");
		//return false;
	//}

	//this->CopyToTransfer(_str, _len, 2);
	//const Bstr* val = &(this->transfer[2]);
	this->request = 0;
	IVNode* ret;
	if (this->root == NULL)	//tree is empty
	{
		leaves_tail = leaves_head = root = new IVLeafNode;
		request += IVNode::LEAF_SIZE;
		this->height = 1;
		root->setHeight(1);	//add to heap later
	}

	//this->prepare(this->root); //root must be in-mem
	if (root->getNum() == IVNode::MAX_KEY_NUM)
	{
		IVNode* father = new IVIntlNode;
		request += IVNode::INTL_SIZE;
		father->addChild(root, 0);
		ret = root->split(father, 0);
		if (ret->isLeaf() && ret->getNext() == NULL)
			this->leaves_tail = ret;
		if (ret->isLeaf())
			request += IVNode::LEAF_SIZE;
		else
			request += IVNode::INTL_SIZE;
		this->height++;		//height rises only when root splits
							//WARN: height area in Node: 4 bit!
		father->setHeight(this->height);	//add to heap later
		this->TSM->updateHeap(ret, ret->getRank(), false);
		this->root = father;
	}
	IVNode* p = this->root;
	IVNode* q;
	int i;
	while (!p->isLeaf())
	{
		//j = p->getNum();
		//for(i = 0; i < j; ++i)
		//if(bstr < *(p->getKey(i)))
		//break;
		//NOTICE: using binary search is better here
		i = p->searchKey_less(_key);

		q = p->getChild(i);
		this->prepare(q);
		if (q->getNum() == IVNode::MAX_KEY_NUM)
		{
			ret = q->split(p, i);
			if (ret->isLeaf() && ret->getNext() == NULL)
				this->leaves_tail = ret;
			if (ret->isLeaf())
				request += IVNode::LEAF_SIZE;
			else
				request += IVNode::INTL_SIZE;
			//BETTER: in loop may update multiple times
			this->TSM->updateHeap(ret, ret->getRank(), false);
			this->TSM->updateHeap(q, q->getRank(), true);
			this->TSM->updateHeap(p, p->getRank(), true);
			if (_key < p->getKey(i))
				p = q;
			else
				p = ret;
		}
		else
		{
			p->setDirty();
			this->TSM->updateHeap(p, p->getRank(), true);
			p = q;
		}
	}
	//j = p->getNum();
	//for(i = 0; i < j; ++i)
	//if(bstr < *(p->getKey(i)))
	//break;
	i = p->searchKey_less(_key);

	//insert existing key is ok, but not inserted in
	//however, the tree-shape may change due to possible split in former code
	bool ifexist = false;
	if (i > 0 && _key == p->getKey(i - 1))
		ifexist = true;
	else
	{
		p->addKey(_key, i);
		p->addValue(this->value_list, i, _str, _len, true);
		p->addNum();
		//NOTICE: is this is a vlist, then it will be freed, and should not be included in the request memory
		if(!VList::isLongList(_len))
		{
			request += _len;
		}
		//request += val->getLen();
		p->setDirty();
		this->TSM->updateHeap(p, p->getRank(), true);
		//_key->clear();
		//_value->clear();
	}
	this->TSM->request(request);
	return !ifexist;		//QUERY(which case:return false)
}

bool
IVTree::modify(unsigned _key, char* _str, unsigned _len)
{
	//if (_key < 0)
	//{
		//printf("error in IVTree-modify: empty string\n");
		//return false;
	//}

	//this->CopyToTransfer(_str, _len, 2);	//not check value
	//const Bstr* val = &(this->transfer[2]);
	this->request = 0;
	int store;
	IVNode* ret = this->find(_key, &store, true);
	if (ret == NULL || store == -1 || _key != ret->getKey(store))	//tree is empty or not found
	{
		SLOG_ERROR("tree is empty or not found");
		return false;
	}
	//cout<<"IVTree::modify() - key is found, now to remove"<<endl;

	//NOTICE+DEBUG: if this value is a long list, then it is not saved in memory, here should return 0 in Bstr
	unsigned len = ret->getValue(store)->getLen();
	if(ret->getValue(store)->isBstrLongList())
	{
		len = 0;
	}
	ret->setValue(this->value_list, store, _str, _len, true);
	//ret->setValue(val, store, true);
	//cout<<"value reset"<<endl;
	//cout<<"newlen: "<<val->getLen()<<" oldlen: "<<len<<endl;

	//request += (val->getLen() - len);
	if(!VList::isLongList(_len))
	{
		this->request += _len;
	}
	//this->request = val->getLen();
	this->request -= len;
	ret->setDirty();
	//cout<<"to request"<<endl;
	this->TSM->request(request);
	//cout<<"memory requested"<<endl;

	return true;
}

//this function is useful for search and modify, and range-query 
IVNode*		//return the first key's position that >= *_key
IVTree::find(unsigned _key, int* _store, bool ifmodify)
{											//to assign value for this->bstr, function shouldn't be const!
	if (this->root == NULL)
		return NULL;						//IVTree Is Empty

	IVNode* p = root;
	int i, j;
	while (!p->isLeaf())
	{
		if (ifmodify)
			p->setDirty();
		//j = p->getNum();
		//for(i = 0; i < j; ++i)				//BETTER(Binary-Search)
		//if(bstr < *(p->getKey(i)))
		//break;
		i = p->searchKey_less(_key);

		p = p->getChild(i);
		this->prepare(p);
	}

	j = p->getNum();
	//for(i = 0; i < j; ++i)
	//if(bstr <= *(p->getKey(i)))
	//break;
	i = p->searchKey_lessEqual(_key);

	if (i == j)
		*_store = -1;	//Not Found
	else
		*_store = i;

	return p;
}

/*
Node*
IVTree::find(unsigned _len, const char* _str, int* store) const
{
}
*/

bool
IVTree::remove(unsigned _key)
{
	//if (_key < 0)
	//{
		//printf("error in IVTree-remove: empty string\n");
		//return false;
	//}

	this->request = 0;
	IVNode* ret;
	if (this->root == NULL)	//tree is empty
		return false;

	IVNode* p = this->root;
	IVNode* q;
	int i, j;
	while (!p->isLeaf())
	{
		j = p->getNum();
		//for(i = 0; i < j; ++i)
		//if(bstr < *(p->getKey(i)))
		//break;
		i = p->searchKey_less(_key);

		q = p->getChild(i);
		this->prepare(q);
		if (q->getNum() < IVNode::MIN_CHILD_NUM)	//==MIN_KEY_NUM
		{
			if (i > 0)
				this->prepare(p->getChild(i - 1));
			if (i < j)
				this->prepare(p->getChild(i + 1));
			ret = q->coalesce(p, i);
			if (ret != NULL)
				this->TSM->updateHeap(ret, 0, true);//non-sense node
			this->TSM->updateHeap(q, q->getRank(), true);
			if (q->isLeaf())
			{
				if (q->getPrev() == NULL)
					this->leaves_head = q;
				if (q->getNext() == NULL)
					this->leaves_tail = q;
			}
			if (p->getNum() == 0)		//root shrinks
			{
				//this->leaves_head = q;
				this->root = q;
				this->TSM->updateHeap(p, 0, true);	//instead of delete p				
				this->height--;
			}
		}
		else
			p->setDirty();
		this->TSM->updateHeap(p, p->getRank(), true);
		p = q;
	}

	bool flag = false;
	//j = p->getNum();		//LeafNode(maybe root)
	//for(i = 0; i < j; ++i)
	//	if(bstr == *(p->getKey(i)))
	//	{
	//		request -= p->getKey(i)->getLen();
	//		request -= p->getValue(i)->getLen();
	//		p->subKey(i, true);		//to release
	//		p->subValue(i, true);	//to release
	//		p->subNum();
	//		if(p->getNum() == 0)	//root leaf 0 key
	//		{
	//			this->root = NULL;
	//			this->leaves_head = NULL;
	//			this->leaves_tail = NULL;
	//			this->height = 0;
	//			this->TSM->updateHeap(p, 0, true);	//instead of delete p
	//		}
	//		p->setDirty();
	//		flag = true;
	//		break;
	//	}
	i = p->searchKey_equal(_key);
	//WARN+NOTICE:here must check, because the key to remove maybe not exist
	if (i != (int)p->getNum())
	{
		if(!p->getValue(i)->isBstrLongList())
		{
			request -= p->getValue(i)->getLen();
		}
		p->subKey(i);		//to release
		p->subValue(this->value_list, i, true);	//to release
		p->subNum();
		if (p->getNum() == 0)	//root leaf 0 key
		{
			this->root = NULL;
			this->leaves_head = NULL;
			this->leaves_tail = NULL;
			this->height = 0;
			this->TSM->updateHeap(p, 0, true);	//instead of delete p
		}
		p->setDirty();
		flag = true;
	}

	this->TSM->request(request);
	return flag;		//i == j, not found		
}

const Bstr*
IVTree::getRangeValue()
{
	if (this->stream == NULL)
	{
		fprintf(stderr, "IVTree::getRangeValue(): no results now!\n");
		return NULL;
	}
	if (this->stream->isEnd())
	{
		fprintf(stderr, "IVTree::getRangeValue(): read till end now!\n");
		return NULL;
	}
	//NOTICE:this is one record, and donot free the memory!
	//NOTICE:Bstr[] but only one element, used as Bstr*
	return this->stream->read();
}

void
IVTree::resetStream()
{
	if (this->stream == NULL)
	{
		fprintf(stderr, "no results now!\n");
		return;
	}
	this->stream->setEnd();
}

//TODO: change to using value list, getValue() maybe not get real long list
bool	//special case: not exist, one-edge-case
IVTree::range_query(unsigned _key1, unsigned _key2)
{		//the range is: *_key1 <= x < *_key2 	
		//if(_key1 <0 && _key2 <0)
		//return false;
		//ok to search one-edge, requiring only one be negative
		//find and write value
	int store1, store2;
	IVNode *p1, *p2;
	if (_key1 >= 0)
	{
		request = 0;
		p1 = this->find(_key1, &store1, false);
		if (p1 == NULL || store1 == -1)
			return false;	//no element
		this->TSM->request(request);
	}
	else
	{
		p1 = this->leaves_head;
		store1 = 0;
	}
	if (_key2 >= 0)
	{		//QUERY: another strategy is to getnext and compare every time to tell end
		request = 0;
		p2 = this->find(_key2, &store2, false);
		if (p2 == NULL)
			return false;
		else if (store2 == -1)
			store2 = p2->getNum();
		else if (store2 == 0)
		{
			p2 = p2->getPrev();
			if (p2 == NULL)
				return false;		//no element
			store2 = p2->getNum();
		}
		this->TSM->request(request);
	}
	else
	{
		p2 = this->leaves_tail;
		store2 = p2->getNum();
	}

	IVNode* p = p1;
	unsigned i, l, r;
	//get the num of answers first, not need to prepare the node
	unsigned ansNum = 0;
	while (true)
	{
		//request = 0;
		//this->prepare(p);
		if (p == p1)
			l = store1;
		else
			l = 0;
		if (p == p2)
			r = store2;
		else
			r = p->getNum();
		ansNum += (r - l);
		//this->TSM->request(request);
		if (p != p2)
			p = p->getNext();
		else
			break;
	}

	if (this->stream != NULL)
	{
		delete this->stream;
		this->stream = NULL;
	}
	vector<unsigned> keys;
	vector<bool> desc;
	this->stream = new Stream(keys, desc, ansNum, 1, false);

	p = p1;
	while (1)
	{
		request = 0;
		this->prepare(p);
		if (p == p1)
			l = store1;
		else
			l = 0;
		if (p == p2)
			r = store2;
		else
			r = p->getNum();
		for (i = l; i < r; ++i)
		{
			//NOTICE:Bstr* in an array, used as Bstr[]
			//DEBUG+TODO: if long list?? clean
			this->stream->write(p->getValue(i));
		}
		this->TSM->request(request);
		if (p != p2)
			p = p->getNext();
		else
			break;
	}

	this->stream->setEnd();

	return true;
}

bool
IVTree::save()	//save the whole tree to disk
{
#ifdef DEBUG_KVSTORE
	printf("now to save tree!\n");
#endif
	this->value_list->release_cache();
	if (TSM->writeTree(this->root))
		return true;
	else
		return false;
}

void
IVTree::release(IVNode* _np) const
{
	if (_np == NULL)	return;
	if (_np->isLeaf())
	{
		delete _np;
		return;
	}
	int cnt = _np->getNum();
	for (; cnt >= 0; --cnt)
		release(_np->getChild(cnt));
	delete _np;
}

void
IVTree::AddIntoCache(TYPE_PREDICATE_ID _id)
{
	char* _tmp = NULL;
	unsigned _len;
	this->search(_id, _tmp, _len);
//	cout << "len is " << len << endl;
	this->value_list->AddIntoCache(_id, _tmp, _len);
	delete [] _tmp;
}

void
IVTree::AddIntoCache(TYPE_ENTITY_LITERAL_ID _id)
{
	char* _tmp = NULL;
	unsigned _len = 0;
	this->search(_id, _tmp, _len);
//	cout << "len is " << len << endl;
	this->value_list->AddIntoCache(_id, _tmp, _len);
	delete [] _tmp;
}

IVTree::~IVTree()
{
	//cout << "IVTree" << endl;
	//cout << "delete Vlist" << endl;
	delete this->value_list;
	value_list = NULL;
	//cout << "delete stream" << endl;
	delete this->stream;//maybe NULL
	this->stream = NULL;
	//cout << "delete TSM" << endl;
	delete TSM;
	TSM = NULL;
#ifdef DEBUG_KVSTORE
	//printf("already empty the buffer, now to delete all nodes in tree!\n");
#endif
	//recursively delete each Node
	//cout << "release" << endl;
	release(root);
	//cout << "~IVTree done" << endl;
}

void
IVTree::print(string s)
{
#ifdef DEBUG_KVSTORE
	fputs(Util::showtime().c_str(), Util::debug_kvstore);
	fputs("Class IVTree\n", Util::debug_kvstore);
	fputs("Message: ", Util::debug_kvstore);
	fputs(s.c_str(), Util::debug_kvstore);
	fputs("\n", Util::debug_kvstore);
	fprintf(Util::debug_kvstore, "Height: %d\n", this->height);
	if (s == "tree" || s == "TREE")
	{
		if (this->root == NULL)
		{
			fputs("Null IVTree\n", Util::debug_kvstore);
			return;
		}
		IVNode** ns = new IVNode*[this->height];
		int* ni = new int[this->height];
		IVNode* np;
		int i, pos = 0;
		ns[pos] = this->root;
		ni[pos] = this->root->getNum();
		pos++;
		while (pos > 0)
		{
			np = ns[pos - 1];
			i = ni[pos - 1];
			this->prepare(np);
			if (np->isLeaf() || i < 0)	//LeafNode or ready IntlNode
			{							//child-num ranges: 0~num
				if (s == "tree")
					np->print("node");
				else
					np->print("NODE");	//print full node-information
				pos--;
				continue;
			}
			else
			{
				ns[pos] = np->getChild(i);
				ni[pos - 1]--;
				ni[pos] = ns[pos]->getNum();
				pos++;
			}
		}
		delete[] ns;
		delete[] ni;
	}
	else if (s == "LEAVES" || s == "leaves")
	{
		IVNode* np;
		for (np = this->leaves_head; np != NULL; np = np->getNext())
		{
			this->prepare(np);
			if (s == "leaves")
				np->print("node");
			else
				np->print("NODE");
		}
	}
	else if (s == "check tree")
	{
		//check the tree, if satisfy B+ definition
		//TODO	
	}
	else;
#endif
}

