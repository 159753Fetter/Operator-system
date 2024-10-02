#include <bits/stdc++.h>
#include <dirent.h>
using namespace std;

#define BUFFER_SIZE 1024
/*
	struct dirent {
	      ino_t          d_ino;       inode number 索引节点号
	      off_t          d_off;       not an offset; see NOTES 在目录文件中的偏移
	      unsigned short d_reclen;    length of this record 文件名长
	      unsigned char  d_type;      type of file; not supported by all  filesystem types 文件类型                               
	      char           d_name[256]; filename 文件名，最长255字符
           }
      dirent节点可以度出文件分配表的属性 
*/	
void Pmaps(int argc,char **argv){
    dirent* Load=NULL;
    long long pid,pid1; // pid用来记录进程号参数,pid1用于下面查找部分
    if(argc == 3) 
        pid = atoi(argv[2]);
    else 
        pid = atoi(argv[1]);
    if(pid <= 0){
        printf("The process pid is invalid\n");
        exit(-1);
    }

    DIR* dp = opendir("/proc");
    if(dp != NULL){
        if(argc == 3 && (strcmp(argv[1], "-d") == 0)){
            printf("地址                  Kbytes    Mode      偏移量                    设备                  Mapping\n");
        }
        else if(argc == 2 || (argc == 3 && (strcmp(argv[1], "-p") == 0))){
            if((argc == 3 && (strcmp(argv[1], "-p") == 0)))
                printf("地址                  Kbytes    Mode      Mapping(Absolute Path)\n");
            else
                printf("地址                  Kbytes    Mode      Mapping\n");
        }
        else if(argc == 3 && (strcmp(argv[1], "-c") == 0)){
            printf("地址\n");
        }
        int size = 0;
        while((Load = readdir(dp))){ // readdir(文件描述符)打开一个目录文件，并不停的读取一个目录项
            pid1 = strtol(Load->d_name, NULL ,10);
            if(Load->d_type == DT_DIR && pid1 == pid){ // 如果是目录文件且进程号是目标进程号
                char path[50] = "/proc/";
                DIR* dir;
                if(argc == 3)//得到完整的目录表
                    strcat(path,argv[2]);
                else
                    strcat(path,argv[1]);
                dir = opendir(path);
                if(dir == NULL){
                    printf("PID is wrong\n");
                    closedir(dir);
                    closedir(dp);
                    exit(-1);
                } 
                else{
                    ifstream in; // 打开文件
                    string line; // 一次性读入一行，在从一行内输入
                    string temp; // 用来记录进程号
                    if(argc == 3)
                        temp = argv[2];
                    else 
                        temp = argv[1];
                    string file = "/proc/" + temp + "/maps";
                    in.open(file.c_str());
                    getline(in,line);
                    while(!in.eof()){
                        stringstream buf(line);
					    string str;
                        int num = 0;
                        vector<string> vec;
                        while(!buf.eof()){
                            buf>>str;
                            if(num == 0){
                                string start,end;
                                int place = str.find('-');
                                if(place == 0){
                                    cout<<"Address is wrong\n";
                                    exit(-1);
                                }
                                else{
                                    start = str.substr(0,place);
                                    end = str.substr(place + 1, str.length() - place);
                                    vec.push_back(start);
                                    long long startplace,endplace,sizeofall;
                                    startplace=strtol(start.c_str(),NULL,16);
								    endplace=strtol(end.c_str(),NULL,16);
								    sizeofall=(endplace-startplace)/1024;//计算差值
                                    if(start.length()==16) sizeofall = 4;
								    size+=sizeofall;
                                    string tsize = to_string(sizeofall) + "K";
                                    vec.push_back(tsize);
                                }
                            }

                            else if(strcmp(argv[1], "-c") == 0){
                                break;
                            }
                            else if(argc == 3 && num < 4 && (strcmp(argv[1], "-d") == 0))
                                vec.push_back(str);
                            else if(argc == 3 && num == 5){
                                if(strcmp(argv[1], "-p") == 0){
                                    string str1 = str;
                                    while(!buf.eof()){
                                        buf>>str;
                                        str1 += " " + str;
                                    }
                                    vec.push_back(str1);
                                }
                                else{
                                    int place;
                                    string str1;
                                    if(str[0]=='/'){
                                        while(!buf.eof())
                                            buf>>str;
                                        for(int i = str.length()-1; i > 0; i--){
                                            if(str[i]=='/'){
                                                place = i;
                                                break;
                                            }
                                        }
                                   for(int i = place + 1; i < str.length(); i++)
											str1+=str[i];
									str=str1;
                                    } 
                                    vec.push_back(str);
                                }
                            }
                            else if((argc == 2 && num == 1)||(argc == 3 && ((strcmp(argv[1], "-p") == 0)||(strcmp(argv[1], "-q") == 0)) && num == 1))
								vec.push_back(str);
							else if(argc == 2 && num == 5){
								if(str[0]=='/'){
									while(!buf.eof())
										buf>>str;
									string str1;
									int place=0;
									for(int i = str.length() - 1 ; i > 0; i--)
										if(str[i]=='/'){
											place=i;
											break;
										}
									for(int i = place + 1;i < str.length(); i++)
										str1+=str[i];
									str=str1;
								}
								vec.push_back(str);
							}
							num++;
                        }
                        for(int i = 0; i < vec.size(); i++){
                            if(strcmp(argv[1], "-d") == 0){
                                if(i == 0){
                                    if(vec[0].length()!=16) cout <<"0000" << setfill(' ') << setw(18) << left << vec[i];
                                    else cout << setfill(' ') << setw(22) << left << vec[i];
                                }
                                else if(i == 3){
                                    cout <<"00000000" << setfill(' ') << setw(18) << left << vec[i];
                                }
                                else if(i == 4){
                                    string temp = vec[i];
                                    string temp1 = vec[i].substr(0,2);
                                    string temp2 = vec[i].substr(3,2);
                                    temp1 = "00" + temp1;
                                    temp2 =  "000" + temp2;
                                    vec[i] = temp1 +":"+ temp2;
                                   cout << setfill(' ') << setw(22) << left << vec[i];
                                }
							    else
								    cout << setfill(' ') << setw(10) << left << vec[i];  
                            }

                            else{
                                if(i==0){
                                    if(vec[0].length()!=16) cout <<"0000" << setfill(' ') << setw(18) << left << vec[i];
                                    else cout << setfill(' ') << setw(22) << left << vec[i];
                                }
							    else
								    cout << setfill(' ') << setw(10) << left << vec[i];
                            }
                        }
						cout<<endl;
						getline(in,line);
                    }
                    in.close();
                }
                closedir(dir);
                if((strcmp(argv[1], "-q") != 0) && (strcmp(argv[1], "-c") != 0))
                    printf("Total:          %lldK\n",size);
                return ;
            }     
        }   
    }
    else{
        printf("Unable to open file\n");
        return ;
    }
}

void Pmapx(int argc,char **argv){
    dirent* Load=NULL;
	long long pid, pid1;
	pid = atoi(argv[2]);//得到目标进程号
	
    if(pid <= 0){
		cout<<"PID is illeagal"<<endl;
		exit(-1);
	}

	int mode = 0;
	DIR* dp = opendir("/proc");//打开proc目录 某个进程号x在proc对应/proc/x目录 返回一个dir类型的指针，类似文件描述符

	if(strcmp(argv[1], "-x") == 0) // 代表-XX
		mode = 0;
	else if(strcmp(argv[1], "-X") == 0) // 代表-x
		mode = 1;
	else // 代表-X
		mode = 2;

	//proc文件存储的是当前内核运行状态的一系列特殊文件，用户可以通过这些文件查看有关系统硬件及当前正在运行进程的信息

	if(dp != NULL){
		if(mode == 0){
            cout << "地址              Kbytes       RSS       Dirty       Mode        Mapping"<<endl;	
		}
		else if(mode == 1){   
            cout<<"地址      Perm  Offset    Dev    Inode   Size    Rss   Pss  Pss_Dirty  Refe Anon  LazF  SPMD   FPMD  Sh_H   Pr_H   Swap   SPss   Lock   THPe   Mapping"<<endl;
		} 
		else if(mode == 2){
			cout << setfill(' ') << setw(13) << left << "地址" << "  perm " << "offset   DEV   INODE   SIZE  KPS   MMUS  RSS   PSS   P_D   SH_C  SH_D  PR_C  PR_D  REFE  ANON  LAZF  AHPS  SPMD  FPMD  SH_H  PR_H  SWAP  SWPP  LOCK  THPL  VM                MAP" << endl;
		}

		while((Load = readdir(dp))){//readdir(文件描述符)打开一个目录文件，并不停的读取一个目录项
			pid1 = strtol(Load->d_name, NULL, 10);
            if(Load->d_type == DT_DIR && pid1 == pid){//如果是目录文件且进程号是目标进程号
				char path[50]="/proc/";
				DIR* dir;
				strcat(path,argv[2]);//得到完整的目录表
				dir = opendir(path);
				if(dir == NULL){
					printf("PID is wrong\n");
					closedir(dir);
					closedir(dp);
					exit(-1);
				}
				else{
					ifstream in;
					string line;//一次性读入一行，在从一行内输入
					string temp = argv[2];
					string file = "/proc/" + temp + "/smaps";//找到对应的文件
					in.open(file.c_str());//打开smaps文件，读取记录
                    int num = 1;
                    getline(in,line);
                    vector<string> output1;
                    vector<int> Size;
                    for(int i = 0; i < 30; i++) Size.push_back(0);
					while(!in.eof()){
                        vector<string> output;    
                        if((num % 25) == 0){
                            num = 1;
                            for(int i = 0; i < output1.size(); i++)
                                cout << output1[i]<<"        ";
                            output1.clear(); 
                            cout << endl;
                        }
                        stringstream buf(line);
						string str;
						/*smap文件中 24行对应一页 每24行的第一行包括以下内容：
						地址范围 权限 偏移量 设备 索引节点号 映射文件支持
						其他23行是各种详细内存信息*/
						if(num == 1){//num = 1代表是每一页对应信息的起始位置那一行
                            int cnt = 0;
                            while(!buf.eof()){
                                buf >> str;
                                //cout << cnt << "-" << str << endl;
                                if(cnt == 0){
									string start,end;//地址包含起始地址-结束地址
									int place=str.find('-');//找到'-'，前部分为开始，后部分为结束	
									if(place == 0){
										cout<<"Address is error"<<endl;
										exit(-1);
									}
									else{
										start=str.substr(0,place);
                                        if(mode == 0){
                                            if(start.length() == 16) cout<<start<<"    ";
                                            else cout << "0000" << start << "    ";
										    output.push_back(start);//得到前半部分
                                        }
                                        if(mode == 1 || mode == 2){
                                            cout<<start<<" ";
                                            output.push_back(start);
                                        }
									}		
                                }
                                else if (mode == 0 && cnt == 1)
                                    output1.push_back(str);
                                else if((mode == 1 || mode == 2) && cnt < 5){
                                     cout << str << " ";
                                     if(cnt == 4 && str == "0") cout<<"      ";
                                     output.push_back(str);
                                }
                                   
                                else if(cnt == 5){
                                    if(str[0]=='/'){
                                        string str1;
                                        int place = 1;
                                        while(!buf.eof())
											buf>>str;
										for(int i = str.size() - 1; i > 0; i--)
											if(str[i] == '/'){
												place=i;
												break;
											}
										for(int i = place + 1; i < str.size();i++)
											str1 += str[i];
										str = str1;
                                    }
                                    output1.push_back(str);
                                }
                                cnt ++;
                            }
                        } 
                        else{
                            buf >> str;
                            if(mode == 0 && (str == "Size:" || str == "Rss:" || str == "Private_Dirty:")){
                                string str1 = str;
                                buf >> str;
                                int num1 = strtol(str.c_str(),NULL,10);
                                if(str1 == "Size:"){ 
                                    Size[1] += num1;
                                    str += "K";
                                }
                                if(str1 == "Rss:") 
                                    Size[2] += num1;
                                if(str1 == "Private_Dirty:") 
                                    Size[3] += num1;
                                output.push_back(str);
                                for(int i = 0; i < output.size(); i++){
                                    cout << std::left << setw(5) << output[i] << "      ";
                                }
                            }

                            if(mode == 1 && (str == "Size:" || str == "Rss:" || str == "Pss:" || str == "Pss_Dirty:" || str == "Referenced:" || str == "Anonymous:" 
                            || str == "LazyFree:" || str == "ShmemPmdMapped:" || str == "FilePmdMapped:" || str == "Shared_Hugetlb:" || str == "Private_Hugetlb:"
                            || str == "Swap:" || str == "SwapPss:" || str == "Locked:" || str == "THPeligible:")){
                                buf >> str;
                                string str1 = str;
                                int num1 = strtol(str.c_str(),NULL,10);
                                Size[num] += num1;
                                output.push_back(str);
                                for(int i = 0; i < output.size(); i++){
                                    cout << std::left << setw(5) << output[i] << "  ";
                                }
                            }

                            if(mode == 2 && (str == "Size:" || str == "KernelPageSize:" || str == "MMUPageSize:" || str == "Rss:" || str == "Pss:" 
                            || str == "Pss_Dirty:" || str == "Shared_Clean:" || str == "Shared_Dirty:" || str == "Private_Clean:" || str == "Private_Dirty:" 
                            || str == "Referenced:" || str == "Anonymous:" || str == "LazyFree:" || str == "AnonHugePages:" || str == "ShmemPmdMapped:" 
                            || str == "FilePmdMapped:" || str == "Shared_Hugetlb:" || str == "Private_Hugetlb:" || str == "Swap:" || str == "SwapPss:" 
                            || str == "Locked:" || str == "THPeligible:"|| str == "VmFlags:")){
                                if(str != "VmFlags:"){
                                    buf >> str;
                                    string str1 = str;
                                    int num1 = strtol(str.c_str(),NULL,10);
                                    Size[num] += num1;
                                    output.push_back(str);
                                    for(int i = 0; i < output.size(); i++){
                                        cout << std::left << setw(5) << output[i] << " ";
                                    }
                                }
                                else{
                                    string str1;
									vector<string> v;
									while(!buf.eof()){//权限一列内容较多 需要循环输入
										buf>>str;
										if(v.size()==0){
											v.push_back(str);
                                            str1 = str1 + str;
                                        }
										else{
											bool pd = false;
											for(int i = 0; i < v.size(); i++)
												if(v[i] == str){
													pd = true;
													break;
												}
											if(pd == false)	
												str1 = str1 + " " + str;
										}
									}
									//output.push_back(z);
                                    cout << str1 <<" ";
                                }
                            }
                       
                        }
                        num++;
                        getline(in,line);
                    }
                    for(int i = 0; i < output1.size(); i++)
                            cout << output1[i] << "        ";
                    cout << endl;
                    if(mode == 0){
                        cout << "---------------- ------- ------- ------- " << endl;
                        cout << "Total KB         " << Size[1] << "      " << Size[2] << "     " << Size[3] << endl;
                    }
                    if(mode == 1){
                        cout << "                                         ";
                        cout << Size[2] << "  " << Size[5] << "   " << Size[6] << "   " << Size[7] << "    " << Size[12] << "   " << Size[13] << "    " << Size[14] << "      ";
                        for(int i = 16; i <= 23; i++) cout<< Size[i] << "      "; 
                        cout <<" KB"<<endl;
                    }
                    if(mode == 2){ 
                        cout << "                                         "; 
                        cout<<Size[2] << " " << Size[3] <<"   " << Size[4] << "   " << Size[5] << "  " << Size[6] << "  "; 
                        for(int i = 7; i <= 24; i++) cout << std::left << setw(5) << Size[i] << "  "; 
                        cout <<" KB"<<endl;
                    }
                    in.close();
                    closedir(dir);
				}	
			}	
		}
	}
    else{
        printf("Unable to open file\n");
        return ;
    }
}

void output(){
    printf("\nUsage:\n");
    printf("pmap [options] PID [PID ...]\n");
    printf("\nOptions:\n");
    printf("-x, --extended              show details\n");
    printf("-X                          show even more details\n");
    printf("           WARNING: format changes according to /proc/PID/smaps\n");
    printf("-XX                         show everything the kernel provides\n");
    printf("-c, --read-rc               read the default rc\n");
    printf("-C, --read-rc-from=<file>   read the rc from file\n");
    printf("-n, --create-rc             create new default rc\n");
    printf("-N, --create-rc-to=<file>   create new rc to file\n");
    printf("           NOTE: pid arguments are not allowed with -n, -N\n");
    printf("-d, --device                show the device format\n");
    printf("-q, --quiet                 do not display header and footer\n");
    printf("-p, --show-path             show path in the mapping\n");
    printf("-A, --range=<low>[,<high>]  limit results to the given range\n");
    printf("-h, --help                  显示此帮助然后离开\n");
    printf("-V, --version               显示程序版本然后离开\n");
}

void show(int argc, char **argv, string str){
    dirent* Load=NULL;
	long long pid,pid1;
	pid = atoi(argv[2]);//得到目标进程号
	if(pid <= 0 ){
		cout<<"Pid is illeagal"<<endl;
		exit(-1);
	}
	DIR* dp=opendir("/proc");//打开proc目录 某个进程号x在proc对应/proc/x目录 返回一个dir类型的指针，类似文件描述符
	//proc文件存储的是当前内核运行状态的一系列特殊文件，用户可以通过这些文件查看有关系统硬件及当前正在运行进程的信息
	if(dp!=NULL){
		int size=0;
		while((Load=readdir(dp))){//readdir(文件描述符)打开一个目录文件，并不停的读取一个目录项
			pid1 = strtol(Load->d_name, NULL, 10);
			if(Load->d_type == DT_DIR && pid1 == pid){//如果是目录文件且进程号是目标进程号
				char path[50]="/proc/";
				DIR* dir;
				strcat(path,argv[2]);
				dir = opendir(path);
				if(dir == NULL){
					printf("Pid is wrong\n");
					closedir(dir);
					closedir(dp);
					exit(-1);
				}
				else{
					ifstream in;
					string line;//一次性读入一行，在从一行内输入
					string temp;
					temp=argv[2];
					string file="/proc/" + temp + "/" + str;//找到对应的文件
					in.open(file.c_str());//打开maps文件，读取记录
					getline(in,line);
					while(!in.eof()){
						//对读入的一行进行处理
						stringstream buf(line);
						string str1;
						int i=0;
						vector<string> output;
						while(!buf.eof()){
							//查阅资料，发现
							//address  perms offset dev  inode  pathname
							//map一行包含上述内容，即地址/权限/偏移量/设备 /索引节点编号/路径
							buf>>str1;
							output.push_back(str1);
						}
						for(int i=0;i<output.size();i++)
							if(i==0){
								cout << setfill(' ') << setw(18) << left << output[i];
                            }
							else
								cout << setfill(' ') << setw(10) << left << output[i];
						printf("\n");
						getline(in,line);
					}
			        in.close();
    		        closedir(dir);
    		    	return;
				}	
			}	
		}
	}
    else{
        cout<<"无法打开文件\n";
        return;
    }
}

int main(int argc, char **argv){

    long long pid; // 记录需要查询的进程号
    if(argc < 2){
        printf("This command lacks parameters\n");
        return 0;

    }
    else if (argc == 2){
        if(argv[1][1]=='h'){  //根据pmap -h命令，显示此帮助然后离开
            output();
            return 0;
        }
        if((strcmp(argv[1], "-V") == 0)){
            printf("pmap from procps-ng 3.3.17\n");
            return 0;
        }
        Pmaps(argc,argv);
    }
    else{ //带参数的pmap
        if((strcmp(argv[1], "-x") == 0) || (strcmp(argv[1], "-X") == 0) || (strcmp(argv[1], "-XX") == 0)) {
            Pmapx(argc,argv);
        } 

        else if((strcmp(argv[1], "-d") == 0) || (strcmp(argv[1], "-q") == 0) || (strcmp(argv[1], "-p") == 0) || (strcmp(argv[1], "-c") == 0) ){
            Pmaps(argc,argv);
        }

        else if((strcmp(argv[1], "-m") == 0)){
            string str;
            printf("Please enter the query folder:\n");
            cin>>str;
            show(argc,argv,str);
        }
        else{
            printf("This command is wrong\n");
            output();
        }

    }

    return 0;
}