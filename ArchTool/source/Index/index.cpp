#include <iostream>
#include <fstream>
using namespace std;

int main(int argc,char * argv[])
{
	int i;
	char word[8],sentence[100];
//-------------------------write ascii	
	ofstream fout;
	fout.open("out.txt");
	fout << "hello world!\n" << flush;
//-------------------------read ascii
	ifstream fin("in.txt");
	fin >> word;
	cout << word << endl;

	fin.getline(sentence,100);
	cout << sentence << endl;
	fin.close();
	
	
//-------------------------write binary
	int number = 30;
	ofstream bfout("file.bin",ios::binary);
	bfout.write((char*)(&number),sizeof(number));
	bfout.close();
	//---------------------BETTER method, but have a error
	struct OBJECT
	{
		int	number;
		char letter;
	}obj;
	obj.number = 15;
	obj.letter = 'M';
	ofstream bbfout("bfile.bin",ios::binary);
	bbfout.write((char*)(&obj),sizeof(obj));
	bbfout.close();
	//---------------------BAD method
	ofstream bbbfout("bbfile.bin",ios::binary);
	bbbfout << (char*)(&number);
	bbbfout.close();
//-------------------------read binary
	int num;
	ifstream bfin("file.bin",ios::binary);
	bfin.read((char*)(&num),sizeof(num));
	bfin.close();
	cout << "output from file.bin:" << num << endl;

	cout << "参数列表:" << endl;
	for(i = 0; i < argc; i++)
	{
		cout << argv[i] << endl;
	}
	system("pause");
	return 0;
}