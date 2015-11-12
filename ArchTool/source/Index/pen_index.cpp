#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <vector>
#include <algorithm>
#include <Windows.h>
#include <WinBase.h>		//WIN32_FIND_DATA
#include <vector>
#include <algorithm>
using namespace std;
#pragma warning(disable:4996)
#define cout log

#define CODESTART 0x10000 + 4	//4为总资源的大小
#define INDEXSTART (0x10000 + 4 + (0x1869f + 10)*5)		//后面所加为所支持的码值数

void create_64K();
void analyse_conf();
void analyse_line(char * str);
void print_all();
void analyse_audio_file();
int write2file();
void process_struct();
int IsHaveError();

typedef struct tagCODEINDEX			//code2index的结构
{
	int code;
	char link_num;
	int index_addr;
	char link_name[100][50];			//按照顺序来罗列link_num个音频名字
}CODEINDEX, *PCODEINDEX;

typedef struct tagAUDIOINDEX		//索引N后面的结构
{
	int audio_addr;					// = 音频首地址+ AUDIO.audio_offset;
	int audio_size;
}AUDIOINDEX, *PAUDIOINDEX;

typedef struct tagAUDIO				//存储音频的机构
{
	char audio_name[50];
	int audio_offset;
	int audio_size;
}AUDIO, *PAUDIO;

CODEINDEX	code2index[0x19000];	//code结构数组[中为code值]
int var_code = 0;					//第几个code
AUDIO		audio[10000];			//音频文件结构数组
int audio_num_all = 0;				//出现的总的音频个数
int audio_num_repeat=0;				//音频重复的个数
//AUDIOINDEX audioindex[3000];		//音频索引的结构

int source_size = 0;				//资源总的大小


char NoThisAudio[100][50] = {0};
int	 iNoThisAudio = 0;
int  RepeatCode[100] = {0};
int	 iRepeatCode = 0;
char ErrorAudio[100][50] = {0};
int	 iErrorAudio = 0;

int main(int argc, char* argv[])
{
	for(int i=0; i< 0x1900; i++)
		memset(code2index[i].link_name,NULL,50*100);
	//点读校准使用
	create_64K();
	//------------解析ini文件，把相应的信息放到对应的结构体中
	analyse_conf();
	//------------获得ini文件中音频的具体信息
	analyse_audio_file();
	//如果有错误的音频就直接退出
	if(IsHaveError()){
		return 0;
	}
	//------------处理结构体中的信息，比如对code进行排序，支持code乱序输入
	process_struct();
	//------------直接打印出结构体，方便检查
	print_all();
	//------------把上述结构中的数据对应着写入bin文件
	write2file();
	cerr << "clean.bat" << endl;
	ShellExecute(NULL,"open","clean.bat",NULL,NULL,SW_HIDE);
	return 0;
}
int IsHaveError()
{
	ofstream ReadError("..\\ErrorReport.txt");
	ReadError << "------------------check over:" << endl;
	cerr << "------------------check over:" << endl;
	if(iErrorAudio){
		ReadError << "------------------error: please check those audio:  _O.O_" << endl;
		cerr << "------------------error: please check those audio:  _O.O_" << endl;
		for(int z = 0; z < iErrorAudio; z++){
			ReadError << "------------------" << (z+1) << ":" << ErrorAudio[z] << endl;
			cerr << "------------------" << (z+1) << ":" << ErrorAudio[z] << endl;
		}
	}
	if(iRepeatCode){
		ReadError << "------------------error: Have repeat code: _O.O_" << endl;
		cerr << "------------------error: Have repeat code: _O.O_" << endl;
		for(int z = 0; z < iRepeatCode; z++){
			ReadError << "------------------" << (z+1) << ":" << RepeatCode[z] << endl;
			cerr << "------------------" << (z+1) << ":" << RepeatCode[z] << endl;
		}
	}
	if(iRepeatCode || iErrorAudio){
		cerr << "******************PLEASE READ ErrorReport.txt TO CHECK THOSE AUDIO!" << endl;
		std::system("pause");
		ShellExecute(NULL,"open","clean.bat",NULL,NULL,SW_HIDE);
		return 1;
	}
	ReadError << "------------------No error _O.O_" << endl;
	cerr << "------------------No error _O.O_" << endl;
	return 0;
}
void process_struct()
{
	vector<int> sort_code;
	//vector<int>::iterator iter_begin = sort_code.begin();
	//vector<int>::iterator iter_end = sort_code.end();

	for(int i=0; i<var_code; i++)			//放入数据
	{
		sort_code.push_back(code2index[i].code);
	}
	sort(sort_code.begin(),sort_code.end());	//排序
	for(int i=0; i<var_code; i++)				//修改顺序
	{
		for(int j=0; j<var_code; j++)
			if(sort_code[i] == code2index[j].code)
			{
				code2index[var_code] = code2index[i];
				code2index[i] = code2index[j];
				code2index[j] = code2index[var_code];
			}
	}
	for(int i=0; i<var_code; i++)			//计算音频片段开始地址
	{
		if(i == 0)
			code2index[i].index_addr = INDEXSTART;
		else
		{
			code2index[i].index_addr = code2index[i-1].index_addr + code2index[i-1].link_num*8;
		}
	}
}
bool less_filename(const string& s1,const string& s2)
{
	return (strcmp(s1.c_str(),s2.c_str()) < 0 );
}
void analyse_conf()
{
	ofstream log("log.txt");
	ofstream conf_all("..\\conf\\000.conf.ini");
	ifstream conf;
	
	//合并conf.ini文件
	char szFile[MAX_PATH] = "..\\conf\\*.ini";
	char str[1000];
	vector<string> vect; 
	string TempStr;
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = ::FindFirstFileA(szFile,&FindFileData);
	if(INVALID_HANDLE_VALUE == hFind){
		cerr << "No configuration file in conf file _0.0_" << endl;
		return ;
	}
	while(TRUE){
		//if(!strcmp(FindFileData.cFileName,"conf.ini"))	continue;
		if(!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
			vect.push_back(FindFileData.cFileName);
		}
		if(!FindNextFile(hFind,&FindFileData))	break;
	}
	sort(vect.begin(),vect.end(),less_filename);
	for(int i=1; i < vect.size(); i++){
		TempStr = "..\\conf\\";
		TempStr += vect.at(i);
		conf.open(TempStr.c_str());
		cerr << TempStr.c_str() << endl;
		while(conf){
			conf.getline(str,1000);
			cerr << str << endl;
			conf_all << str << endl;
		}
		conf.close();
	}
	conf_all.close();

	CopyFileA("..\\conf\\000.conf.ini","conf.ini",0);

	system("pause");

	ifstream fin("conf.ini");
	//fin.getline(str,1000);
	//if(!strcmp(str,"[code2index]"))
	//{
		//fin.getline(str,sizeof(str));
		while(fin)
		{
			
			fin.getline(str,sizeof(str));
			analyse_line(str);
		}
	//}

	log.close();
}
void analyse_line(char * str)
{
	ofstream log("log.txt",ios::app);

	if(strlen(str) == 0){
		cout << endl;
		return;
	}
	if( *str == '[' ){
		for(int i=0; i<strlen(str); i++)
			cout << *(str+i);
		cout << endl;
		return;
	}

	int i = 0, num = 0, audio_num_line = 0, name_i, repeat_flag=0;
	char temp_name[50];							//临时存储名字信息，方便查重

	while( *(str+i) == ' ' )	i++;		//找到code值，允许存在空格	
	code2index[var_code].code = atoi(str);	//把char型的数值转换为int
	//if(var_code > 0 && code2index[var_code].code < code2index[var_code-1].code){
	//	RepeatCode[iRepeatCode++] = code2index[i].code;
	//}
	for(int i=0; i<var_code; i++){
		if(code2index[i].code == code2index[var_code].code)
		{
			RepeatCode[iRepeatCode++] = code2index[i].code;
		}
	}

	cout << "--------------------------------------code2index 's code:" << code2index[var_code].code << endl;
	int j=0;
	num = code2index[var_code].code;
	while( num / 10)									//判断code的位数(j+1)
	{
		j++;
		num /= 10;
	}
	i += (j+1);
	cout << "音频名字分别为:" << endl;

	int break_flag = 0;
	while( i < strlen(str) )
	{
		while( *(str+i) == '=' || *(str+i) == ' ' || *(str+i) == '+'){ 		//找到文件名
			i++;
			if(i >= strlen(str)){		//检测到结尾
				break_flag = 1;
			}
		}
		if(break_flag)
			break;

		j = 0;
		memset(temp_name,NULL,sizeof(temp_name));
		while( (*(str+i) != ' ') && (*(str+i) != '+') && (i < strlen(str)))		//取出文件名
		{
			temp_name[j] = *(str+i);
			i++;
			j++;
		}

		int z = 0;
		while(temp_name[z] != '.')
		{
			code2index[var_code].link_name[audio_num_line][z] = temp_name[z];
			z++;
		}
		strcat(code2index[var_code].link_name[audio_num_line],".s7");		//替换文件名为s7格式

		cout << "第" << var_code << "个code的第" << audio_num_line << "个音频是：" << temp_name << endl;
		for(name_i=0; name_i < audio_num_all; name_i++)
		{
			if(!strcmp(audio[name_i].audio_name,temp_name))
			{
				repeat_flag = 1;
				audio_num_repeat++;
				cout << "---已经存在的音频" << audio[name_i].audio_name << endl;
				break;
			}
		}
		if(repeat_flag == 0)
		{
			strcpy(audio[audio_num_all].audio_name,temp_name);
			cout << "-----------新加的音频" << audio[audio_num_all].audio_name << endl;
			audio_num_all++;
			cout << "总的音频个数" << audio_num_all << endl;
		}else{
			repeat_flag = 0;
		}
		audio_num_line++;
	}
	cout << "------------------------------------一行中的音频个数共：" << audio_num_line << endl;
	code2index[var_code].link_num = audio_num_line;
	var_code++;	
	log.close();
}
void analyse_audio_file()
{
	WIN32_FIND_DATA FindFileData;
	SYSTEMTIME  SystemTime;
	char str[100];
	char Tranfer[50];

	ofstream log("log.txt",ios::app);
	ofstream AudioInfo1("data.y");
	for(int i=0; i<audio_num_all; i++)		//压缩
	{
		string par;
		string Temp = "";
		char temp_name[50],storage_name[50];

		memset(storage_name,NULL,sizeof(storage_name));
		strcpy(storage_name,audio[i].audio_name);
		memset(temp_name,NULL,sizeof(temp_name));
	
		par = "-e -7 ";
		par += audio[i].audio_name;
		par += " ";
		int j = 0;
		while(audio[i].audio_name[j] != '.')
		{
			temp_name[j] = audio[i].audio_name[j];
			j++;
		}
		strcat(temp_name,".s7");
		memset(audio[i].audio_name,NULL,sizeof(audio[i].audio_name));
		strcpy(audio[i].audio_name,temp_name);
		par += temp_name;

		//ifstream s7_file;
		//s7_file.open(temp_name,ios::binary);
		//s7_file.seekg(0,ios::end);
//-------------------------------------
		
		FindClose(FindFirstFile(storage_name,&FindFileData));
		if(!FileTimeToSystemTime(&FindFileData.ftLastWriteTime,&SystemTime)){
			cout << "tranfer failure!!!!" << endl;
		}
		Temp = audio[i].audio_name;
		Temp += '|';
		itoa(SystemTime.wYear,Tranfer,10);			Temp += Tranfer;
		itoa(SystemTime.wMonth,Tranfer,10);			Temp += Tranfer;
		itoa(SystemTime.wDay,Tranfer,10);			Temp += Tranfer;
		itoa((SystemTime.wHour+8),Tranfer,10);			Temp += Tranfer;
		itoa(SystemTime.wMinute,Tranfer,10);		Temp += Tranfer;
		itoa(SystemTime.wSecond,Tranfer,10);		Temp += Tranfer;
		itoa(SystemTime.wMilliseconds,Tranfer,10);	Temp += Tranfer;
		AudioInfo1 << Temp << endl;

		//cerr << SystemTime.wYear << "|" << SystemTime.wMonth << "|" << SystemTime.wDay << "|" << SystemTime.wHour << "|" << SystemTime.wMinute << "|" << SystemTime.wMilliseconds << endl;
		//cerr << Temp << endl;
		//system("pause");
		int bHadPressFlag = 0;
		ifstream AudioInfo("data.n");
		while(AudioInfo){
			AudioInfo.getline(str,100);
			//cerr << str << '\t' << Temp.c_str() << strlen(str) << '|' << strlen(Temp.c_str()) << endl;
			if(!strcmp(Temp.c_str(),str)){			//说明没有压缩过，或者改动过
				bHadPressFlag = 1;
				//cerr << "Find it!!" << endl;
				break;
			}
		}
		AudioInfo.close();
		
		if(bHadPressFlag == 0){
			cerr << "compressing " << storage_name << endl;
			SHELLEXECUTEINFO sei; 
			memset(&sei, 0, sizeof(SHELLEXECUTEINFO)); 

			sei.cbSize = sizeof(SHELLEXECUTEINFO); 
			sei.fMask = SEE_MASK_NOCLOSEPROCESS; 
			sei.hwnd = NULL;
			sei.lpVerb = NULL;
			sei.lpVerb = "open"; 
			sei.lpFile = "siren7.exe"; 
			sei.lpDirectory = NULL;
			sei.nShow = SW_HIDE; 
			sei.hInstApp = NULL;
			sei.lpParameters = par.c_str();
			ShellExecuteEx(&sei); 
			WaitForSingleObject(sei.hProcess, INFINITE);
			CloseHandle(sei.hProcess);

			cerr << "compress " << storage_name << " OK !" << endl;
		}
	}
	/*std::system("pause");*/
	for(int i=0; i<audio_num_all; i++)		//分析
	{
		ifstream audio_file;
		audio_file.open(audio[i].audio_name,ios::binary);
		audio_file.seekg(0,ios::end);
		if(0 >= (audio[i].audio_size = audio_file.tellg()))
		{
			strcpy(ErrorAudio[iErrorAudio++],audio[i].audio_name);
		}
		//cerr << audio[i].audio_size << "\t" << endl;
		audio_file.close();
		if(i == 0)
			audio[i].audio_offset = 0;
		else
		{
			audio[i].audio_offset = audio[i-1].audio_offset + audio[i-1].audio_size;
		}
	}
	AudioInfo1.close();
	log.close();
}
void create_64K()
{
	int i;
	char letter = 0x00;
	char str[] = {0x56,0x00,0x00,0x00,0x44,0x00,0x00,0x00,0x56,0x00,0x00,0x00,0x37,0x00,0x00,0x00,0xAA,0x00,0x00,0x00};

	//------------点读笔校准头
	ofstream fout("AudioData.bin",ios::binary);
	fout.write(str,sizeof(str));

	//------------点读笔校准擦除区域
	for(i=0x14; i <= 0xFF; i++)
	{
		fout.write(&letter,sizeof(letter));
	}
	letter = 0xFF;
	for( i=0xFF+1; i <= 0xFFFF; i++)
	{
		fout.write(&letter,sizeof(letter));
	}
	fout.close();
}
void print_all()
{
	ofstream log("log.txt",ios::app);

	int i,j=0,audio_start = 0;
	//---------------conf.ini文件信息
	cout << endl << "conf.ini文件信息：" << endl << "conf.ini文件当中共有"<< var_code << "个点读码，每个点读码对应的音频文件如下：" << endl;
	for(i=0; i<var_code; i++)
	{
		cout << code2index[i].code << " = ";
		for(j=0; j<code2index[i].link_num; j++)
		{
			cout << code2index[i].link_name[j] << " ";
		}
		cout << endl;
	}
	//---------------CODEINDEX中的信息存储
	cout << endl << "CODEINDEX中的信息存储：" << endl << "总共存在"<< var_code << "个点读码,陈列如下：" << endl;
	
	for(i=0; i<var_code; i++)
	{
		cout << "点读码:"<< code2index[i].code << "\t绝对地址：0x" << hex << code2index[i].code * 5 + CODESTART << dec;
		cout << "\t片段个数"  << (int)code2index[i].link_num << "\t片段初始地址:0x" << hex << code2index[i].index_addr << dec << endl; 
	}
	//--------------AUDIOINDEX结构的信息存储
	cout << endl << "AUDIOINDEX结构的首地址为:" << INDEXSTART << ",依次填写的内容为：" << endl;
	audio_start = INDEXSTART + (audio_num_all + audio_num_repeat) * 8;
	for(i=0; i<var_code; i++)
	{
		for(j=0; j<code2index[i].link_num; j++)
		{
			for(int z=0; z<audio_num_all; z++)
				if(!strcmp(code2index[i].link_name[j],audio[z].audio_name))
					cout << "音频名字：" << code2index[i].link_name[j] << "\t绝对地址:0x" << hex << audio_start + audio[z].audio_offset << "\t大小:0x" << audio[z].audio_size << dec << endl;
		}
		cout << endl;
	}

	//---------------AUDIO中的信息存储
	cout << endl << "AUDIO中的信息存储：" << endl << "总共存在"<< audio_num_all << "个音频文件,陈列如下：" << endl;
	cout << "首地址为:0x" << hex << audio_start << dec << endl;		//AUDIO结构的首地址为:总的音频数（包括重复）* 8
	for(i=0; i<audio_num_all; i++)
	{
		cout << "音频名字："<< audio[i].audio_name << "\t偏移地址：0x" << hex << audio[i].audio_offset << "\t绝对地址：0x" << audio_start + audio[i].audio_offset << "\t大小:0x" << audio[i].audio_size << dec << endl;
	}

	//--------------整个资源的大小
	source_size = audio_start + audio[audio_num_all-1].audio_offset + audio[audio_num_all-1].audio_size;
	cout << "整个资源的大小：0x" << hex << source_size << dec << endl;

	log.close();
}
int write2file()
{
	int i,j=0,audio_start = 0, code_end_addr;
	char letter=0x00;
	ofstream fout("AudioData.bin",ios::binary | ios::app);
	//---------------存储CODEINDEX信息
	fout.write((char *)&source_size,sizeof(source_size));
	for(i=0; i<var_code; i++)
	{
		if(i==0)
			for(j=0; j<code2index[i].code*5; j++)
			{
				fout.write(&letter,sizeof(letter));
			}
		else
			for(j=0; j<(code2index[i].code-code2index[i-1].code-1)*5; j++)
			{
				fout.write(&letter,sizeof(letter));
			}
		fout.write(&code2index[i].link_num,sizeof(code2index[i].link_num)); 
		fout.write((char *)&code2index[i].index_addr,sizeof(code2index[i].index_addr));
	}
	code_end_addr = CODESTART + (code2index[var_code-1].code + 1) * 5;
	for(; code_end_addr<INDEXSTART; code_end_addr++)
		fout.write(&letter,sizeof(letter));

	//---------------存储AUDIOINDEX信息
	audio_start = INDEXSTART + (audio_num_all + audio_num_repeat) * 8;
	for(i=0; i<var_code; i++)
	{
		for(j=0; j<code2index[i].link_num; j++)
		{
			for(int z=0; z<audio_num_all; z++)
				if(!strcmp(code2index[i].link_name[j],audio[z].audio_name))
				{
					int audio_addr_temp = 0;
					audio_addr_temp = audio_start + audio[z].audio_offset;
					fout.write( (char *)&audio_addr_temp, sizeof(int) );
					fout.write((char *)&audio[z].audio_size,sizeof(int));
				}
		}
	}

	//---------------AUDIO中的信息存储
	ifstream fin;
	for(i=0; i<audio_num_all; i++)
	{
		fin.open(audio[i].audio_name,ios::binary);
		for(j=0; j<audio[i].audio_size; j++)
		{
			fin.get(letter);
			fout.put(letter);
		}
		fin.close();
	}
	fout.close();
	return 1;
}