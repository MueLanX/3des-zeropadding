#include <iostream>
#include <fstream>	//	文件输入输出流
#include <bitset>	//	C++STL 类似数组，只能是0和1
#include <cstring>
#include <cmath>
#include <ctime>
#include <cstdio>

using namespace std;

bitset<64> key;	//	64位密钥
bitset<48> subKeys[16];	//	存放16轮子密钥

//	初始置换表：加密明文的第一步
int IP[] = { 58, 50, 42, 34, 26, 18, 10, 2,
			 60, 52, 44, 36, 28, 20, 12, 4,
			 62, 54, 46, 38, 30, 22, 14, 6,
			 64, 56, 48, 40, 32, 24, 16, 8,
			 57, 49, 41, 33, 25, 17,  9, 1,
			 59, 51, 43, 35, 27, 19, 11, 3,
			 61, 53, 45, 37, 29, 21, 13, 5,
			 63, 55, 47, 39, 31, 23, 15, 7 };

//	初始逆置换表：得到密文前的最后一步
int IP_1[] = { 40, 8, 48, 16, 56, 24, 64, 32,
			   39, 7, 47, 15, 55, 23, 63, 31,
			   38, 6, 46, 14, 54, 22, 62, 30,
			   37, 5, 45, 13, 53, 21, 61, 29,
			   36, 4, 44, 12, 52, 20, 60, 28,
			   35, 3, 43, 11, 51, 19, 59, 27,
			   34, 2, 42, 10, 50, 18, 58, 26,
			   33, 1, 41,  9, 49, 17, 57, 25 };

//	置换选择表1：将64位密钥变成56位
int PC_1[] = { 57, 49, 41, 33, 25, 17,  9,
			   1, 58, 50, 42, 34, 26, 18,
			   10,  2, 59, 51, 43, 35, 27,
			   19, 11,  3, 60, 52, 44, 36,
			   63, 55, 47, 39, 31, 23, 15,
			   7, 62, 54, 46, 38, 30, 22,
			   14,  6, 61, 53, 45, 37, 29,
			   21, 13,  5, 28, 20, 12,  4 };

//	置换选择表2：将56位密钥压缩成48位子密钥
int PC_2[] = { 14, 17, 11, 24,  1,  5,
				3, 28, 15,  6, 21, 10,
			   23, 19, 12,  4, 26,  8,
			   16,  7, 27, 20, 13,  2,
			   41, 52, 31, 37, 47, 55,
			   30, 40, 51, 45, 33, 48,
			   44, 49, 39, 56, 34, 53,
			   46, 42, 50, 36, 29, 32 };

//	每轮左移的位数
int shiftBits[] = { 1, 1, 2, 2, 2, 2, 2, 2,
					1, 2, 2, 2, 2, 2, 2, 1 };

//	扩展置换表：将 32位 扩展至 48位
int E[] = { 32,  1,  2,  3,  4,  5,
			 4,  5,  6,  7,  8,  9,
			 8,  9, 10, 11, 12, 13,
			12, 13, 14, 15, 16, 17,
			16, 17, 18, 19, 20, 21,
			20, 21, 22, 23, 24, 25,
			24, 25, 26, 27, 28, 29,
			28, 29, 30, 31, 32,  1 };


//	S盒：每个S盒是4x16的置换表，6位 -> 4位，8个S盒
int S_BOX[8][4][16] = {
		{
				{14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7},
				{0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8},
				{4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0},
				{15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13}
		},
		{
				{15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10},
				{3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5},
				{0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15},
				{13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9}
		},
		{
				{10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8},
				{13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1},
				{13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},
				{1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12}
		},
		{
				{7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15},
				{13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9},
				{10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},
				{3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14}
		},
		{
				{2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9},
				{14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6},
				{4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},
				{11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3}
		},
		{
				{12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},
				{10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8},
				{9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6},
				{4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13}
		},
		{
				{4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},
				{13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},
				{1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},
				{6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12}
		},
		{
				{13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7},
				{1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2},
				{7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8},
				{2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}
		}
};

//	P盒：32位 -> 32位
int P[] = { 16,  7, 20, 21,
			29, 12, 28, 17,
			 1, 15, 23, 26,
			 5, 18, 31, 10,
			 2,  8, 24, 14,
			32, 27,  3,  9,
			19, 13, 30,  6,
			22, 11,  4, 25 };


//	轮函数f
bitset<32> f(bitset<32> R, bitset<48> K) {
	bitset<48> expandR;	//	32位输入 扩展置换后得到的48位

	//	第一步：扩展置换
	for (int i = 0; i < 48; ++i)
		expandR[47 - i] = R[32 - E[i]];

	//	第二步：异或运算
	expandR = expandR ^ K;

	//	第三步：S盒代换
	bitset<32> afterS;	//	48位输入 S盒代换后得到的32位

	//	48位扩展置换后的密钥，分成八组，每组6位
	for (int i = 0; i < 8; i++) {
		int row = expandR[47 - i * 6] * 2 + expandR[47 - i * 6 - 5];	//	第一位和第六位二进制数转为十进制，设为行
		int col = expandR[47 - i * 6 - 1] * 8 + expandR[47 - i * 6 - 2] * 4
			+ expandR[47 - i * 6 - 3] * 2 + expandR[47 - i * 6 - 4]; 	//	中间四位二进制转为十进制，设为列
		int num = S_BOX[i / 6][row][col];   //  得到S盒中的十进制数

		bitset<4> binary(num); 	//	转成四位二进制数
		afterS[31 - i * 4] = binary[3];
		afterS[31 - i * 4 - 1] = binary[2];
		afterS[31 - i * 4 - 2] = binary[1];
		afterS[31 - i * 4 - 3] = binary[0];
	}
	//	第四步：P盒置换
	bitset<32> output;	//	P盒置换得到的32位
	for (int i = 0; i < 32; ++i)
		output[31 - i] = afterS[32 - P[i]];
	return output;
}


//	对56位密钥的前后部分进行左移
bitset<28> leftShift(bitset<28> k, int shift) {
	bitset<28> tmp = k;
	for (int i = 27; i >= 0; i--) {
		if (i - shift < 0)
			k[i] = tmp[i - shift + 28]; //	处理左移溢出高位，移到最低位
		else
			k[i] = tmp[i - shift];
	}
	return k;
}

//	生成subkey： 共计16个48位的子密钥
void generateKeys() {
	bitset<56> realKey;
	bitset<28> left;    //	高位密钥
	bitset<28> right;   //	低位密钥
	bitset<48> compressKey;
	//	置换选择1，去掉奇偶标记位，将64位密钥变成56位
	for (int i = 0; i < 56; ++i)
		realKey[55 - i] = key[64 - PC_1[i]];

	//	生成子密钥，保存在 subKey[16] 中
	for (int round = 0; round < 16; round++) {

		//	前28位与后28位
		for (int i = 28; i < 56; i++)
			left[i - 28] = realKey[i];
		for (int i = 0; i < 28; i++)
			right[i] = realKey[i];

		//	左移
		left = leftShift(left, shiftBits[round]);
		right = leftShift(right, shiftBits[round]);

		//	组合成56位
		for (int i = 28; i < 56; i++)
			realKey[i] = left[i - 28];
		for (int i = 0; i < 28; i++)
			realKey[i] = right[i];

		//	置换选择2，将56位密钥变成48位
		for (int i = 0; i < 48; i++)
			compressKey[47 - i] = realKey[56 - PC_2[i]];
		subKeys[round] = compressKey;
	}
}

//	工具函数：将char字符数组转为二进制
bitset<64> charToBitset(const char s[8]) {
	bitset<64> bits;
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			bits[i * 8 + j] = ((s[i] >> j) & 1);
	return bits;
}


//	加密过程
bitset<64> encrypt(bitset<64>& originBits) {
	bitset<64> enBits;
	bitset<64> currentBits;
	bitset<32> left;
	bitset<32> right;
	bitset<32> newLeft;

	//	第一步：初始置换IP
	for (int i = 0; i < 64; i++)
		currentBits[63 - i] = originBits[64 - IP[i]];

	//	第二步：获取Li和Ri
	for (int i = 32; i < 64; i++)
		left[i - 32] = currentBits[i];
	for (int i = 0; i < 32; i++)
		right[i] = currentBits[i];

	//	第三步：共16轮迭代
	for (int round = 0; round < 16; round++) {
		newLeft = right;
		right = left ^ f(right, subKeys[round]);
		left = newLeft;
	}

	//	第四步：合并L16和R16，注意合并为 R16L16
	for (int i = 0; i < 32; i++)
		enBits[i] = left[i];
	for (int i = 32; i < 64; i++)
		enBits[i] = right[i - 32];
	
	//	第五步：结尾置换IP-1
	currentBits = enBits;
	for (int i = 0; i < 64; ++i)
		enBits[63 - i] = currentBits[64 - IP_1[i]];
	
	//	返回密文
	return enBits;
}


//	解密过程
bitset<64> decrypt(bitset<64>& enBits) {
	bitset<64> deBits;  //	最终密文
	bitset<64> currentBits;
	bitset<32> left;
	bitset<32> right;
	bitset<32> newLeft;
	//	第一步：初始置换IP
	for (int i = 0; i < 64; ++i)
		currentBits[63 - i] = enBits[64 - IP[i]];
	
	//	第二步：获取 Li 和 Ri
	for (int i = 32; i < 64; ++i)
		left[i - 32] = currentBits[i];
	for (int i = 0; i < 32; ++i)
		right[i] = currentBits[i];
	
	//	第三步：共16轮迭代（子密钥逆序应用）
	for (int round = 0; round < 16; ++round) {
		newLeft = right;
		right = left ^ f(right, subKeys[15 - round]);
		left = newLeft;
	}

	//	第四步：合并L16和R16，注意合并为 R16L16
	for (int i = 0; i < 32; ++i)
		deBits[i] = left[i];
	for (int i = 32; i < 64; ++i)
		deBits[i] = right[i - 32];
	
	//	第五步：结尾置换IP-1
	currentBits = deBits;
	for (int i = 0; i < 64; ++i)
		deBits[63 - i] = currentBits[64 - IP_1[i]];
	
	//	返回明文
	return deBits;
}


void en(char* sKey, const char* file1, const char* file2) {
	key = charToBitset(sKey);
	generateKeys();
	bitset<64> temp;
	bitset<64> tempen;
	fstream f1;	//	输入流
	fstream f2;	//	输出流
	//	打开需要加密的文件
	f1.open(file1, ios::binary | ios::in);
	f1.seekg(0, ios::end);
	//	计算加密次数
	int times;
	times = ((int)f1.tellg() + sizeof(temp) - 1) / sizeof(temp);
	//	清空文件
	f2.open(file2, ios::out);
	f2.close();

	//	不断读取并写入加密文件
	f2.open(file2, ios::binary | ios::app);
	for (int i = 0; i < times; i++) {
		temp.reset();	//	清空缓存
		tempen.reset();	//	清空解密缓存（ZeroPadding）
		f1.seekg(i * sizeof(temp), ios::beg);
		f1.read((char*)&temp, sizeof(temp));
		tempen = encrypt(temp);
		f2.write((char*)&tempen, sizeof(tempen));
	}
	f2.close();
	f1.close();

}


void de(char* sKey, const char* file1, const char* file2) {
	key = charToBitset(sKey);
	generateKeys();
	bitset<64> temp;
	bitset<64> tempde;
	fstream f1;	//	输入流
	fstream f2;	//	输出流
	//	打开加密后的文件
	f1.open(file1, ios::binary | ios::in);
	f1.seekg(0, ios::end);
	//	计算解密次数
	int times;
	times = ((int)f1.tellg() + sizeof(temp) - 1) / sizeof(temp);
	//	清空文件
	f2.open(file2, ios::out);
	f2.close();

	//	不断读取并写入解密文件
	f2.open(file2, ios::binary | ios::app);
	for (int i = 0; i < times; i++) {
		temp.reset();	//	清空缓存
		tempde.reset();	//	清空解密缓存（ZeroPadding）
		f1.seekg(i * sizeof(temp), ios::beg);
		f1.read((char*)&temp, sizeof(temp));
		tempde = decrypt(temp);
		f2.write((char*)&tempde, sizeof(tempde));
	}
	f2.close();
	f1.close();
}


//  程序入口
int main(int argc, char* argv[]) {
	//	argv[1]：加解密参数
	//	argv[2]：key Value
	//  argv[3]：源文件
	//  argv[4]：目标文件
	clock_t start, end;	//	定义clock_t变量
	start = clock();	//	开始时间

	if (argc != 5) {
		cout << "********Param error! Please check the parameter!********" << endl;
		return 0;
	}

	if (strcmp(argv[1], "en") == 0) {
		cout << "********3-DES encrypting********" << endl;
		//	3-DES加密过程
		en(argv[2], argv[3], "temp1");
		en(argv[2], "temp1", "temp2");
		en(argv[2], "temp2", argv[4]);
		remove("temp1");
		remove("temp2");
		cout << "********Finsh!********" << endl;
	}
	else if (strcmp(argv[1], "de") == 0) {
		cout << "********3-DES decrypting********" << endl;
		//	3-DES解密过程
		de(argv[2], argv[3], "temp2");
		de(argv[2], "temp2", "temp1");
		de(argv[2], "temp1", argv[4]);
		remove("temp1");
		remove("temp2");
		cout << "********Finsh!********" << endl;
	}                                                                    
	else cout << "********Param error! Please check the parameter!********" << endl;

	end = clock();	//	结束时间
	cout << "********Time:	" <<
	 double(end - start) / CLOCKS_PER_SEC << "s********" << endl;	//	输出时间
	return 0;
}
