#include "AnalyseConf.h"
#include <iostream>
#include <fstream>
using namespace std;
void analyse_conf()
{
	ofstream log("log.txt");

	char str[100];
	ifstream fin("conf.ini");
	fin.getline(str,1000);
	if(!strcmp(str,"[code2index]"))
	{
		fin.getline(str,sizeof(str));
		while(fin)
		{
			analyse_line(str);
			fin.getline(str,sizeof(str));
		}
	}

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
	for(int i=0; i<var_code; i++)
		if(code2index[i].code == code2index[var_code].code)
		{
			cout << "error:不能重复出现\"" << code2index[var_code].code << "\"作为code值！" << endl;
			cerr << "error:不能重复出现\"" << code2index[var_code].code << "\"作为code值！" << endl;
			system("pause");
			ShellExecute(NULL,"open","clean.bat",NULL,NULL,SW_HIDE);
			exit(1);
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