#include "heap.h"
#include "huffmantree.h"
struct charinfo
{
	charinfo()
	:_appearcount(0)
	{}
	charinfo(const charinfo&_chin)
		:_ch(_chin._ch), _appearcount(_chin._appearcount), strcode(_chin.strcode)
	{}
	bool operator <(const charinfo&_chin) const 
	{
		return _appearcount < _chin._appearcount;
	}
	bool operator <(const charinfo&_chin) 
	{
		return _appearcount < _chin._appearcount;
	}
	bool operator !=(const charinfo&_chin) const 
	{
		return _appearcount != _chin._appearcount;
	}
	bool operator !=(const charinfo&_chin) 
	{
		return _appearcount != _chin._appearcount;
	}
	charinfo operator + (const charinfo&_chin)
	{
		charinfo temp(*this);
		temp._appearcount += _chin._appearcount;
		return temp;
	}
	charinfo& operator +=(const charinfo&_chin)
	{
		_appearcount += _chin._appearcount;
		return *this;
	}
	unsigned char _ch;
	size_t _appearcount;
	std::string strcode;
};
class Filecompress
{
public:
	Filecompress()
	{
		for (size_t index = 0; index < 256; index++)
		{
			_fileinfo[index]._ch = index;
		}
	}
	void compress(std::string File_path)
	{
		FILE*fIn = fopen(File_path.c_str(), "r");
		if (NULL == fIn)
		{
			perror("fopen");
			return ;
		}
		size_t dotpos = File_path.find(".");
		char *WB=new char [1024]; //char WB[1024];
		char *RB = new char[1024];
		memset(WB, 0, sizeof(char)* 1024);
		memset(RB, 0, sizeof(char)* 1024);
		size_t readsize = 0;
		size_t Cap_count = 1;
		size_t Cap_idx = 0;
		while (!feof(fIn))  
		{  
			readsize = fread(RB+Cap_idx, sizeof(char), 1024, fIn); 
			if (readsize == 1024)
			{
				++Cap_count;
				char * del = new char [1024 * Cap_count];
				memset(del, 0, 1024 * Cap_count);
				memmove(del, RB, 1024 * (Cap_count-1));
				swap(del, RB);
				delete del;
				Cap_idx = (Cap_count-1) * 1024;
			}
		}  // 记录从 Fin 读到的有效字符个数.
		readsize = readsize + (Cap_count-1) * 1024;
		string Name =File_path.substr(0, dotpos);
		Name += ".hzp";
		FILE*fOut = fopen(Name.c_str(), "w"); // 让程序自己创建个压缩文件为打开 文件的名字+.hzp  Name.c_str()
		File_path += '\n';// 把 \n也写入 路径中方便后面的后续 处理 
		strcpy(WB, File_path.substr(dotpos).c_str());
		fwrite(WB, 1, File_path.size() - dotpos, fOut); // 把后缀已写入 .txt/n
		perror("fwrite");
		for (size_t idx = 0; idx < readsize; ++idx)
		{
			_fileinfo[RB[idx]]._appearcount++;     // 统计出现的每个字符的次数
		}
		string CodeInfo(1, RB[0]);
		for (size_t idx = 0; idx < readsize; ++idx)
		{
			if (string::npos == CodeInfo.find(RB[idx])) // find函数 没找到返回一个npos ，找到返回位置，这里 利用没找到就添加进去 ， 记录 有效字符的种类 
			{
				CodeInfo += RB[idx];
			}
		}
		char Codesize[5];//最多就256/n/0
		while (1)
		{
			size_t count = 0;
			itoa(CodeInfo.size(), Codesize, 10);
			for (size_t idx = 0; idx < 5; idx++)
			{
				if (Codesize[idx] == '\0')
				{
					break;
				}
				count++;
			}
			Codesize[count] = '\n';
			Codesize[count + 1] = '\0';
			fwrite(Codesize, 1, count + 1, fOut); // 把 编码个数 写进去 如列子 4/n
			break;
		}
		string ret;
		char appearcount[34];// 32位数字+ /n+/0
		HuffmanTree<charinfo> Ht(_fileinfo, 256, charinfo()); //压缩最后一步把 字符变成 数字并写入hzp
		HuffmanTreeNode<charinfo>*PCur = NULL;                  // 256为传过去的数组大小，真是huffman 树大小为有效编码 个数也 出现次数大于1的编码个数
		for (size_t idx = 0; idx < CodeInfo.size(); ++idx)  // 这个for俩个任务,1 是 把有效字符写入hzp 二是根据huffmantree 把每个有效字符的编码得到
		{
			int appear_count = 0;
			appear_count = _fileinfo[CodeInfo[idx]]._appearcount;
			itoa(appear_count,appearcount,10);
			ret += CodeInfo[idx]; // A
			ret += ',';  //,
			for (size_t j = 0; j < 32; j++)//把A出现的次数（上面刚通过itoa生成的这个字符串）的次数信息加到ret后面  
			{
				if (appearcount[j] == '\0')
				{
					break;  
				}
				ret += appearcount[j];
			}
			ret += '\n';
			strcpy(WB, ret.c_str());
			fwrite(WB, 1, ret.size(), fOut);
			PCur = (HuffmanTreeNode<charinfo>*)Getleefinfo(Ht.Root(), CodeInfo[idx]);
			while (PCur->_pParent!=NULL)
			{
				if (PCur->_pParent->_pLeft == PCur)
				{
					_fileinfo[CodeInfo[idx]].strcode += '0';
				}
				if (PCur->_pParent->_pRight == PCur)
				{
					_fileinfo[CodeInfo[idx]].strcode += '1';
				}
				PCur = PCur->_pParent;
			}
			std::reverse(_fileinfo[CodeInfo[idx]].strcode.begin(), _fileinfo[CodeInfo[idx]].strcode.end());
			ret.clear();
		}
		unsigned char value = 0;;
		char temp = 0;
		char sum_code = 0; // 有效字符编码的位数 
		size_t stridx = 0;//每个string对象的下标
		int print_i = 0;  // 为了进度条而设置的变量
		int print_j = 0;
		bool flag = true;
		for (size_t idx=0; idx < readsize; ++idx)//压缩最后一步把 字符变成 数字并写入hzp
		{
			//size_t count = 0;
			stridx = 0;
			while (stridx != _fileinfo[RB[idx]].strcode.size())
			{
				temp = _fileinfo[RB[idx]].strcode[stridx] - 48;
				value |= temp;
				if (sum_code < 7)  //sum_code已结移位的次数
				{
					value <<= 1;
					//++print_i;
				}
                sum_code++;
				if (idx!=0&&flag)      //每 移位3次 进来下 ， 看已经转换了多少个有效字符了 
				{
					print_j = ((idx)*100)/readsize;//进度大于 1%,调用进度条函数  这里这样写是出于 如果先除 小于1为0 ， 那么 我们先把除的结果 乘100 再除readsize一样的。 一个数字 先乘以一个数字再除以一个数字，因为是乘除同级运算 结果不变
                    if(print_j>=1) progress_bar(print_j,print_i);
					print_i = print_j;
					flag = false;
				}
				if (sum_code == 8)
				{
					itoa(value, WB, 16);
					if (WB[1] == '\0')
					{
						WB[1] = WB[0];
						WB[0] = '0';
					}
					fwrite(WB, 1, 2, fOut);
					value = 0;
					sum_code = 0;
				}
				++stridx;
			}
			flag = true;
		}
		progress_bar(100, print_j);
		if (sum_code > 0)
		{
			value<<=(7-sum_code); //  处理最后一个有效字符时 ， 可能 value中的有效值 还没有写入文件中，那么我们要把最低有效位 移到高位去，一个8位数总共可以移动7位，那么假设已经移1位，那么再移 7-1=6位即可把低位有效数移到高位去。
			itoa(value, WB, 16);
			if (WB[1] == '\0')
			{
				WB[1] = WB[0];
				WB[0] = '0';
			}
			fwrite(WB, 1,2, fOut);
		}
		progress_bar(100, print_j);
		printf("\n");
		system("pause");
		fclose(fIn);
		fclose(fOut);
		delete RB;
		delete WB;
	}
	void uncompress(std::string File_path)
	{
		FILE*fIn = fopen(File_path.c_str(), "r");
		if (NULL == fIn)
		{
			perror("fopen");
			return;
		}
		char *WB = new char[1024]; //char WB[1024];
		char *RB = new char[1024];
		memset(WB, 0, sizeof(char)* 1024);
		memset(RB, 0, sizeof(char)* 1024);
		size_t readsize = 0;
		size_t Cap_count = 1;
		size_t Cap_idx = 0;
		size_t dotpos = File_path.find(".");
		string Name = File_path.substr(0, dotpos);
        fgets(RB, 1024, fIn); // 记录从 Fin 读到的有效字符个数.
		Name += RB;
		Name.pop_back();
		FILE * fOut = fopen(Name.c_str(), "w");     // 从文件中读出后缀 创立新文件 用来存压缩结果 
		perror("fopen");
		fgets(RB,  1024, fIn);  //从hzp中读取压缩的 有效编码行数                         
		size_t  codecount = atoi(RB);              //count为有效编码行数 
		string CodeInfo;
		string temp;
		int appearcount = 0;
		for (size_t idx = 0; idx < codecount; ++idx)       //从 hzp 中 读取编码 信息 
		{
			 fgets(RB,1024, fIn);
			 CodeInfo += RB[0];                         //刚开始把CodeInfo放到 最后处理 那如果遇见\n的话 上面自己再Gets次 ，那么后面进行CodeInfo的时候 会出现 Codeinfo 记录的是 ',' 而不是\n因为前面已经换行了
			 if (!strcmp(RB, "\n")) fgets(RB, 1024, fIn); //所以等出现有效字符为 , 号 时 会再 记录次,那么后面 记录有效编码时 ,被记录俩次 ，当解析的时候就崩溃了
			 temp = RB;
			 temp = temp.substr(comma(RB) + 1);
			 appearcount = atoi(temp.c_str());
			 //CodeInfo += RB[0];
			 _fileinfo[CodeInfo[idx]]._appearcount = appearcount;	//_fileinfo[RB[0]]._appearcount = appearcount;
		}
		size_t Codesize = CodeInfo.size(); //保存每个 编码 
		HuffmanTree<charinfo> Ht(_fileinfo, 256, charinfo()); // 256为传过去的数组大小，真是huffman 树大小为有效编码 个数也 出现次数大于1的编码个数 ，构建完树后 解码 
		HuffmanTreeNode<charinfo>*PNode = (HuffmanTreeNode<charinfo>*)Ht.Root();
		HuffmanTreeNode<charinfo>*PLeef = (HuffmanTreeNode<charinfo>*)Ht.Root();
		for (size_t idx = 0; idx < Codesize; ++idx)  // 这个for 是为了 把有效编码 的 编码信息给出 
		{
			PNode = (HuffmanTreeNode<charinfo>*)Getleefinfo(Ht.Root(), CodeInfo[idx]);
			PLeef = PNode;
			while (PNode->_pParent != NULL)
			{
				if (PNode->_pParent->_pLeft == PNode)
				{
					_fileinfo[CodeInfo[idx]].strcode += '0';
				}
				if (PNode->_pParent->_pRight == PNode)
				{
					_fileinfo[CodeInfo[idx]].strcode += '1';
				}
				PNode = PNode->_pParent;
			}
			std::reverse(_fileinfo[CodeInfo[idx]].strcode.begin(), _fileinfo[CodeInfo[idx]].strcode.end());
			PLeef->_weight.strcode=_fileinfo[CodeInfo[idx]].strcode ;
		}
		//char RB_1[4096];
		//memset(RB_1, 0, sizeof(char)* 4096);
	   // fgets(RB_1, 4096, fIn);
		while (!feof(fIn))
		{
			readsize = fread(RB + Cap_idx, sizeof(char), 1024, fIn);
			if (readsize == 1024)
			{
				++Cap_count;
				char * del = new char[1024 * Cap_count];
				memset(del, 0, 1024 * Cap_count);
				memmove(del, RB, 1024 * (Cap_count-1));
				swap(del, RB);
				delete del;
				Cap_idx = (Cap_count - 1) * 1024;
			}
		}  // 记录从 Fin 读到的有效字符个数.
		size_t count_code = Ht.Root()->_weight._appearcount; //有效字符的个数
		char conven [9];//为每次的8位2进制码
		char conbase[3];//为保存每个char型数字的值的字符
		int  result;  //保存读到的char型值
		int i = 0;
		char pos = 0;
		int j = 0;
		readsize = readsize + (Cap_count - 1) * 1024;           //size_t readsize = strlen(RB); //RB_1
		HuffmanTreeNode<charinfo>*pRoot = (HuffmanTreeNode<charinfo>*)Ht.Root();
		HuffmanTreeNode<charinfo>*pCur = pRoot;
		string code;
		char Print2file[1];
		while (readsize)
		{
			conbase[2] = 0;
			memcpy(conbase, RB+j, sizeof(char)* 2);//RB_1
			result = _hexatoi(conbase);
			if (result != 0)
			{
				i_toa_2(result, conven); // itoa 如果遇见最高位为1而且 前面全是0，那将舍弃最高位1 前面 所有的0  // 在 有效字符后面写入
			} 
			else
			{
				strcpy(conven, "00000000");
			}
			result = 0;
			while (pos++ != 8)  // 之前写成 ++i 前置加加 少走一步 ，改成i++ 正确 可见 在while中, i=0 ,i++ ,i!=8 对等的是for(i=0;i<8;i++)
			{
				if (conven[i] == '0') // conven[i++]
				{
					pCur = pCur->_pLeft;
					code += '0';
				}
				else
				{
					pCur = pCur->_pRight;
					code += '1';
				}
				if (code == pCur->_weight.strcode)
				{
					Print2file[0] = pCur->_weight._ch;
					fwrite(Print2file,1,1, fOut);
					--count_code;
					code.clear();
					pCur = pRoot; // ********************** 之前这里没有把pCur再次 置为pRoot 那么出去pRoot 一走就为NULL 再走就崩了 
				}
				if (0 == count_code)  //防止最后次读RB 内容时 有效字符已经解析完了 又继续解析多于的位数对应的编码
				{
					break;
				}
				++i;
			}
			j += 2;
			readsize -=2;
			i = 0;
			pos = 0;
			strcpy(conven, "00000000");
		}
		fclose(fIn);
		fclose(fOut);
		delete RB;
		delete WB;
	}
	const HuffmanTreeNode<charinfo>*  Getleefinfo(const HuffmanTreeNode<charinfo>*pRoot, char ch)
	{
		HuffmanTreeNode<charinfo>* ret = NULL;
		if (pRoot == NULL)
		{
			return NULL;
		}
		if (pRoot->_pLeft == NULL&&pRoot->_pRight == NULL)
		{
			if (pRoot->_weight._ch == ch)
			{
				return pRoot;
			}
		}
		if (ret = (HuffmanTreeNode<charinfo>*)Getleefinfo(pRoot->_pLeft, ch))
		{
			return ret;
		}
		if (ret = (HuffmanTreeNode<charinfo>*)Getleefinfo(pRoot->_pRight, ch))
		{
			return ret;
		}
	}
private:
	size_t comma(const char * p)
	{
		size_t count=0;
		while (*p++ != ',') ++count;
		if (*p == ',')  ++count; //处理 有效字符为,
		return count;
	}
	int  _hexatoi(char _p[])
	{
		int ret = 0;
		if (_p[0] != '0')
		{
			if (_p[0] > 'a' || _p[0]=='a')
			{
				ret = (_p[0] - 87) * 16;
			}
			else
			{
				ret = (_p[0] - 48)*16;
			}
		}
		if (_p[1] > 'a'||_p[1] == 'a')
		{
			ret = ret+(_p[1] - 87);
		}
		else
		{
			ret = ret+(_p[1] - 48);
		}
		return ret;
	}
	char* i_toa(int num, char*str, int radix)
	{/*索引表*/
		char index[] = "0123456789ABCDEF";
		unsigned unum;/*中间变量*/
		int i = 0, j, k;
		/*确定unum的值*/
		if (radix == 10 && num<0)/*十进制负数*/
		{
			unum = (unsigned)-num;
			str[i++] = '-';
		}
		else unum = (unsigned)num;/*其他情况*/
		/*转换*/
		do{
			str[i++] = index[unum % (unsigned)radix];
			unum /= radix;
		} while (unum);
		str[i] = '\0';
		/*逆序*/
		if (str[0] == '-')k = 1;/*十进制负数*/
		else k = 0;
		char temp;
		for (j = k; j <= (i - 1) / 2; j++)
		{
			temp = str[j];
			str[j] = str[i - 1 + k - j];
			str[i - 1 + k - j] = temp;
		}
		return str;
	}
	char* i_toa_2(int num, char str[])
	{
		char index[] = "01";
		int i = 0, j;
		int pos = 0;
		do{
			str[i++] = index[num % 2]; //处理当前位
			num >>= 1;  //移位置下一位
			++pos;
		}while(pos != 8);
		str[i] = '\0';
		char temp;
		for (j = 0; j <= (i - 1) / 2; j++)
		{
			 temp=str[j];
			str[j] =str[i - 1 - j];
			str[i -1 -j] = temp;
		}
		return str;
	}
	void progress_bar(int lenth,int prev)
	{
		//static int temp = 0;
		int i = prev;
		char bar[102];
		memset(bar, '#', prev);
		const char *p = "|/-\\";
		do
		{
			bar[i] = '#';
			bar[i + 1] = '\0';
			printf("[%-100s]%d%% %c \r", bar, i, p[i % 4]);
			Sleep(20);
		} while (++i<= lenth);
		//temp = lenth;
	}
	charinfo _fileinfo[256];
};
