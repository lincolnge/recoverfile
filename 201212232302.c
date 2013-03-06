#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<stdbool.h>  //定义bool 

#define DEBUG	0

#pragma pack(push,1)
struct BootEntry
{
	unsigned char BS_jmpBoot[3];	/* Assembly instruction to jump to boot code */
	unsigned char BS_OEMName[8];	/* OEM Name in ASCII */
	unsigned short BPB_BytsPerSec; /* Bytes per sector. Allowed values include 512, 1024, 2048, and 4096 */
	unsigned char BPB_SecPerClus; 
	/* Sectors per cluster (data unit). Allowed values are powers of 2, but the cluster size must be 32KB or smaller */
	unsigned short BPB_RsvdSecCnt;	/* Size in sectors of the reserved area */
	unsigned char BPB_NumFATs;	/* Number of FATs */
	unsigned short BPB_RootEntCnt; /* Maximum number of files in the root directory for FAT12 and FAT16. This is 0 for FAT32 */
	unsigned short BPB_TotSec16;	/* 16-bit value of number of sectors in file system */
	unsigned char BPB_Media;	/* Media type */
	unsigned short BPB_FATSz16; /* 16-bit size in sectors of each FAT for FAT12 and FAT16.  For FAT32, this field is 0 */
	unsigned short BPB_SecPerTrk;	/* Sectors per track of storage device */
	unsigned short BPB_NumHeads;	/* Number of heads in storage device */
	unsigned long BPB_HiddSec;	/* Number of sectors before the start of partition */
	unsigned long BPB_TotSec32; /* 32-bit value of number of sectors in file system.  Either this value or the 16-bit value above must be 0 */
	unsigned long BPB_FATSz32;	/* 32-bit size in sectors of one FAT */
	unsigned short BPB_ExtFlags;	/* A flag for FAT */
	unsigned short BPB_FSVer;	/* The major and minor version number */
	unsigned long BPB_RootClus;	/* Cluster where the root directory can be found */
	unsigned short BPB_FSInfo;	/* Sector where FSINFO structure can be found */
	unsigned short BPB_BkBootSec;	/* Sector where backup copy of boot sector is located */
	unsigned char BPB_Reserved[12];	/* Reserved */
	unsigned char BS_DrvNum;	/* BIOS INT13h drive number */
	unsigned char BS_Reserved1;	/* Not used */
	unsigned char BS_BootSig; /* Extended boot signature to identify if the next three values are valid */
	unsigned long BS_VolID;	/* Volume serial number */
	unsigned char BS_VolLab[11]; /* Volume label in ASCII. User defines when creating the file system */
	unsigned char BS_FilSysType[8];	/* File system type label in ASCII */
};
#pragma pack(pop)

#pragma pack(push,1)
struct DirEntry
{
	unsigned char DIR_Name[11]; /* File name */
	unsigned char DIR_Attr; /* File attributes */
	unsigned char DIR_NTRes; /* Reserved */
	unsigned char DIR_CrtTimeTenth;/* Created time (tenths of second) */
	unsigned short DIR_CrtTime; /* Created time (hours, minutes, seconds) */
	unsigned short DIR_CrtDate; /* Created day */
	unsigned short DIR_LstAccDate; /* Accessed day */
	unsigned short DIR_FstClusHI; /* High 2 bytes of the first cluster address */
	unsigned short DIR_WrtTime; /* Written time (hours, minutes, seconds */
	unsigned short DIR_WrtDate; /* Written day */
	unsigned short DIR_FstClusLO; /* Low 2 bytes of the first cluster address */
	unsigned long DIR_FileSize; /* File size in bytes. (0 for directories) */
};
#pragma pack(pop)

/**
 * translate to cluster index into the real offset in the device file
 *
 * @param index: the cluster index
 * @param be: the boot entry of the device file
 *
 * @returns the offset
 */
unsigned long long cluster2offset(unsigned int index, struct BootEntry be);

/**
 * read clutser data from a device file
 *
 * @param fp: point to a FILE that is readable
 * @param be: the boot entry of the device file
 * @param buf: buffer to hold the read data
 * @param size: size , in bytes, of each element to be read
 * @param clusterindex: cluster index
 *
 * @returns the total number of elements successfully read.
 * If this number differs from the size parameter, either a
 * reading error occurred or the end-of-file was reached while reading.
 */
unsigned int readcluster(FILE * fp, struct BootEntry be, char *buf, unsigned int size, unsigned int clusterindex)
{
	unsigned long long offset = cluster2offset(clusterindex, be);
	fseek(fp, offset, SEEK_SET);
	return fread((void *) buf, 1, size, fp);
	
}

/**
 * write clutser data to a device file
 *
 * @param fp: point to a FILE that is writable
 * @param be: the boot entry of the device file
 * @param buf: buffer to hold the data to be written
 * @param size: size , in bytes, of each element to be written
 * @param clusterindex: cluster index
 *
 * @returns the total number of elements successfully written.
 * If this number differs from the size parameter, a writing error
 * prevented the function from completing.
 */
unsigned int writecluster(FILE * fp, struct BootEntry be, char *buf, unsigned int size, unsigned int clusterindex)
{
	unsigned long long offset = cluster2offset(clusterindex, be);
	fseek(fp, offset, SEEK_SET);
	return fwrite((void *) buf, 1, size, fp);
}

#ifdef DEBUG

// dump implementation
unsigned long long cluster2offset(unsigned int index, struct BootEntry be)
{
	return 0LL;
}

void errorUsage()
{
	printf("Usage: ./recover -d [device filename] [other arguments]\n");
	printf("-i                    Print boot sector information\n");
	printf("-l                    List all the directory entries\n");
	printf("-r filename [-m md5]  File recovery\n");
}

void errorfile()
{
	printf("[filename]: error - fail to recover\n");
}

void error()
{
	printf("# wrong command!\n");
}

int main(int argc,char **argv)
{

	char filename[11];	//有时间再加长文件名 
	FILE *fp;
/*	
//	struct BootEntry be;
	if(argc == 1){
		errorUsage();
	}
	else if((strcmp(argv[1], "-d") == 0)&&argc>=4&&(strcmp(argv[3], "-r") == 0)){
		fp = fopen(argv[2], "r");
		if(fp==NULL) {printf("Error:cannot find this file\n");}
		else{
			filename = argv[4];
			memcpy(filename,argv[4],sizeof(11));
			
		}
	}
	else
		error();
*/
	unsigned char buf[10000];
	unsigned char buf2[100000];
//	struct DirEntry buf2;
	struct BootEntry ptr;
	struct DirEntry ptr2;
	int DataArea = 0;
	int FatArea = 0;
	int directoryArea = 0;
		
	fp = fopen("/dev/ram1", "r");
	if(fp != NULL){
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof(struct BootEntry), 1, fp);
		ptr = *(struct BootEntry *) buf;
		printf("\n\n");
		printf("BPB_BytsPerSec \t%u\n", ptr.BPB_BytsPerSec);
		printf("BPB_BytsPerSec \t%x\n", ptr.BPB_BytsPerSec);
		printf("BPB_SecPerClus \t%u\n", ptr.BPB_SecPerClus);
		printf("BPB_RsvdSecCnt \t%u\n", ptr.BPB_RsvdSecCnt);
		printf("BPB_NumFATs \t%u\n", ptr.BPB_NumFATs);
		printf("BPB_TotSec16 \t%u\n", ptr.BPB_TotSec16);
		printf("BPB_TotSec32 \t%ld\n", ptr.BPB_TotSec32);	//long
		printf("BPB_FATSz32 \t%ld\n", ptr.BPB_FATSz32);		//long
		printf("BPB_RootClus \t%ld\n", ptr.BPB_RootClus);	//long
		
		printf("\n\n");
		DataArea = (ptr.BPB_RsvdSecCnt + ptr.BPB_FATSz32*ptr.BPB_NumFATs)*ptr.BPB_BytsPerSec;
		FatArea = ptr.BPB_RsvdSecCnt*ptr.BPB_BytsPerSec;
		
		printf("%d\n", DataArea);	//1049600
		printf("%d\n", FatArea);	//16384
	
		fseek(fp, DataArea+0, SEEK_SET);	//DataArea = 1049600 所有的 都用DataArea去写 
		fread(buf2, 1, 32, fp);
		ptr2 = *(struct DirEntry *) buf2;
/*		printf("DIR_Name %%s: \t%s\n", ptr2.DIR_Name);
//		printf("DIR_Name %%u: \t%u\n", ptr2.DIR_Name);
		printf("DIR_Attr \t%x\n", ptr2.DIR_Attr);	//这个为0f，就是long file name； 20是文件； 10是目录 
		printf("DIR_FstClusHI \t%u\n", ptr2.DIR_FstClusHI);
		printf("DIR_FstClusLO \t%u\n", ptr2.DIR_FstClusLO);	//用来判断是在哪个cluster里面 
		printf("DIR_FileSize \t%ld\n", ptr2.DIR_FileSize);	//long
		
		printf("\n\n");
*/				
	}else{
		printf("file open error!\n");
	}
	fclose(fp);
/*	
	if(argc==1||argc==0){
		errorUsage();
	}
	else if((strcmp(argv[1], "-d") == 0)&&strcmp(argv[3], "-i") == 0&&argc==4){
		printf("-i \n");
		fp = fopen(argv[2], "r");
		if(fp != NULL){
			struct BootEntry ptr;
			fseek(fp, 0, SEEK_SET);
			fread(buf, sizeof(struct BootEntry), 1, fp);
			ptr = *(struct BootEntry *) buf;
			printf("Number of FATs = %u\n", ptr.BPB_NumFATs);
			printf("Number of bytes per sectoor = %u\n", ptr.BPB_BytsPerSec);
			printf("Number of sectors per cluster = %u\n", ptr.BPB_SecPerClus);
			printf("Number of reserved sectors = %u\n", ptr.BPB_RsvdSecCnt);
				
		}else{
			error();
		}
		fclose(fp);
	}
*/			
	int FirstCluster = 0;
	int SearchNameNum = 0;	//SearchNameNum = 0
	int SearTmp = 0;		//辅助 SearchNameNum 计算 
	int LastSerNum = 0; 
	char FileName[100][11];
	char tmpargv = 0xe5;
	int MuluNameNum = 0;	//MuluNameNum = 0; 目录里循环次数 
	char MuluName[11];		//目录的名字 
//	printf("-l \n\n");
	
	if(argc==1||argc==3){
		errorUsage();
	}
	else if((strcmp(argv[1], "-d") == 0)&&strcmp(argv[3], "-i") == 0&&argc==4){
//		printf("-i \n");
		fp = fopen(argv[2], "r");
		if(fp != NULL){
			struct BootEntry ptr;
			fseek(fp, 0, SEEK_SET);
			fread(buf, sizeof(struct BootEntry), 1, fp);
			ptr = *(struct BootEntry *) buf;
			printf("Number of FATs = %u\n", ptr.BPB_NumFATs);
			printf("Number of bytes per sectoor = %u\n", ptr.BPB_BytsPerSec);
			printf("Number of sectors per cluster = %u\n", ptr.BPB_SecPerClus);
			printf("Number of reserved sectors = %u\n", ptr.BPB_RsvdSecCnt);
				
		}else{
			error();
		}
		fclose(fp);
	}
	else if((strcmp(argv[1], "-d") == 0)&&argc>=4&&(strcmp(argv[3], "-l") == 0)){
		fp = fopen("/dev/ram1", "r");
		if(fp != NULL){
			for(SearchNameNum = 0; SearchNameNum < 20; SearchNameNum++){
				
				fseek(fp, DataArea+SearchNameNum*32, SEEK_SET);	//DataArea = 1049600 所有的 都用DataArea去写 
				fread(buf2, 1, 32, fp);
				ptr2 = *(struct DirEntry *) buf2;
				
				if(ptr2.DIR_Name[0]==0x00)
					break;
			
				FirstCluster = 0;
				if(ptr2.DIR_Name[0]!=0xe5 && ptr2.DIR_Name[0]!=0x00){
					if(ptr2.DIR_Attr==0x10){	//判断是否为目录 
						printf("%d, ", SearchNameNum+SearTmp+1);
						ptr2.DIR_Name[8] = '\0';
						FirstCluster = ptr2.DIR_FstClusLO+ptr2.DIR_FstClusHI*65538;
						memcpy(MuluName, ptr2.DIR_Name, sizeof(ptr2.DIR_Name));
						MuluName[11] = '\0';
						printf("%s/, ", MuluName);
						printf("%ld, ", ptr2.DIR_FileSize);
						printf("%d\n", FirstCluster);
//						printf("ptr2.DIR_FstClusLO \t%u\n", ptr2.DIR_FstClusLO);
//						printf("DIR_FstClusHI \t%u\n", ptr2.DIR_FstClusHI);
						
						//目录是从1048576开始算的，1048576 = 1049600 - 2*512 =  DataArea - 2 * ptr.BPB_BytsPerSec 
						//目录下文件输出----- 1048576（10）=100000（16） 
						directoryArea = DataArea-2*(ptr.BPB_BytsPerSec); 
						for(MuluNameNum = 0; MuluNameNum < 10; MuluNameNum++){
							fseek(fp, directoryArea+FirstCluster*512+MuluNameNum*32, SEEK_SET);	
							//20是从上面的 DIR_FstClusLO与 DIR_FstClusHI的和得到的，512是一个蔟大小， 64是因为文件目录要再加64 
							fread(buf2, 1, 32, fp);
							ptr2 = *(struct DirEntry *) buf2;
							if(ptr2.DIR_Name[0]!=0x00){
								if(ptr2.DIR_Attr==0x10){
									ptr2.DIR_Name[8] = '\0';
								}
								printf("%d, ", SearchNameNum+1+SearTmp+1);
								printf("%s/%s, ", MuluName,ptr2.DIR_Name);
								printf("%ld, ", ptr2.DIR_FileSize);
								printf("%d\n", FirstCluster);
								SearTmp++;
							}
						}
					}
					else{
//						fseek(fp, DataArea+SearchNameNum*32, SEEK_SET);	//DataArea = 1049600 所有的 都用DataArea去写 
//						fread(buf2, 1, 32, fp);
//						ptr2 = *(struct DirEntry *) buf2;
						printf("%d, ", SearchNameNum+1+SearTmp);
						FirstCluster = ptr2.DIR_FstClusLO+ptr2.DIR_FstClusHI*65538;
						printf("%s ", ptr2.DIR_Name);
						printf("%ld ", ptr2.DIR_FileSize);
						printf("%d\n", FirstCluster);
						
					}
					
				}
			}
			
		}else{
			printf("file open error!\n");
		}
		fclose(fp);
	}
	//------------------------------------------------------------------------------------------ 
	
	else if(argc>4&&(strcmp(argv[1], "-d") == 0)&&(strcmp(argv[3], "-r") == 0)){
		//-------------------------匹配文件名的（未完成）-------------------------------- 
		printf("argv[4] is %s \n", argv[4]); 
/*		printf("argv[4][0] is %c \n", argv[4][0]);
		printf("argv[4][1] is %c \n", argv[4][1]);
		printf("argv[4][2] is %c \n", argv[4][2]);
		printf("argv[4][3] is %c \n", argv[4][3]);
		printf("argv[4][4] is %c \n", argv[4][4]);
		printf("argv[4][5] is %c \n", argv[4][5]);
		printf("argv[4][6] is %c \n", argv[4][6]);
		printf("argv[4][7] is %c \n", argv[4][7]);
		printf("argv[4][8] is %c \n", argv[4][8]);
		printf("argv[4][9] is %c \n", argv[4][9]);
		printf("argv[4][10] is %c \n", argv[4][10]);
*/		
	//	printf("%s ",strstr("ABBB","B"));
		int argvii = 0;
		bool ExtenHave = 0;	//bool ExtenHave, 判断扩展名标志 ‘.’ 是否存在 
		int PositExt = 0;
		for(argvii = 0; argvii<11; argvii++){
			if(argv[4][argvii]=='.'){
//				printf("make the position %d \n", argvii+1);
				ExtenHave = 1;
				PositExt = argvii;	// PositExt是 argv[4]点的位置 
			}
		}
//		printf("make the position %d \n", argvii+1);
//		printf("make the ExtenHave %d \n", ExtenHave);
		
		SearchNameNum = 0;	//SearchNameNum = 0
		LastSerNum = 0; 
		FileName[100][11];
		tmpargv = 0xe5;
		tmpargv = argv[4][0];
		
		char tmpFileName[11];	//扩展名要用 
		char tmpArgv[11];		//扩展名要用 
		int ifile = 0;			//用来把空格变成0时 数11位数 
		fp = fopen("/dev/ram1", "r");
		if(fp != NULL){
			for(SearchNameNum = 0; SearchNameNum < 20; SearchNameNum++){
				
//				if(ptr2.DIR_Name[0]==0x00)
//					break;
					
				fseek(fp, DataArea+SearchNameNum*32, SEEK_SET);	//DataArea = 1049600 所有的 都用DataArea去写 
				fread(buf2, 1, 32, fp);
				ptr2 = *(struct DirEntry *) buf2;
				int FilePosi = 0;	//确认输入名字与被删除名字一致 
				if(ptr2.DIR_Name[0]==0xe5){
					if(!ExtenHave){
/*						
						printf("************************\n");
						printf("The file was removed ");
						printf("DIR_Name %%s: \t%s\n", ptr2.DIR_Name);
						printf("%d \n", SearchNameNum);
*/
						memcpy(filename, ptr2.DIR_Name, sizeof(ptr2.DIR_Name));
						filename[11] = '\0';
						ifile = 0;
						for(ifile = 0; ifile < 11; ifile++){
							if(filename[ifile]==0x20)
								filename[ifile] = 0x00;
						}
/*
						printf("file name is %s\n", filename);
						printf("file name is %c\n", filename[0]);
						printf("file name is %x\n", filename[6]);
*/						
						argv[4][0] = 0xe5;
						argv[4][11] = '\0';
//						printf("argv[4] is %s\n", argv[4]);
//						printf("argv[4][6] is %x\n", argv[4][6]);
						FilePosi = strcmp(filename, argv[4]);
//						printf("FilePosi is %d\n", FilePosi);
						if(FilePosi==0){
							LastSerNum = SearchNameNum;
							printf("LastSerNum is %d\n", LastSerNum);
						}
					}
					else{
						printf("************************\n");
//						printf("PositExt is %d\n", PositExt);
						memcpy(filename, ptr2.DIR_Name, sizeof(ptr2.DIR_Name));
						memcpy(tmpArgv, argv[4], sizeof(argv[4]));
						argv[4][11] = '\0'; 
						tmpArgv[11] = '\0';
						tmpArgv[0] = 0xe5;
						tmpArgv[PositExt] = '\0';
						filename[PositExt] = '\0';
//						printf("filename is %s\n", filename);
//						printf("tmpArgv is %s\n", tmpArgv);
						
						filename[11] = '\0';
						for(ifile = 0; ifile < 11; ifile++){
							if(filename[ifile]==0x20)
								filename[ifile] = 0x00;
						}
						FilePosi = strcmp(filename, tmpArgv);
						
						printf("FilePosi is %d\n", FilePosi);
						if(FilePosi==0&&argv[4][PositExt+1]==ptr2.DIR_Name[8]&&argv[4][PositExt+2]==ptr2.DIR_Name[9]&&argv[4][PositExt+3]==ptr2.DIR_Name[10]){
							LastSerNum = SearchNameNum;
							printf("LastSerNum is %d\n", LastSerNum);
						}
					}
				}
				argv[4][0] = tmpargv;
	//			printf("tmpargv is %x\n", tmpargv);
	//			printf("LastSerNum is %d\n", LastSerNum);
				
				if(ptr2.DIR_Name[0]==0x00){
//					printf("Empty part\n ");
				}
				
				FirstCluster = 0;
				if(ptr2.DIR_Name[0]!=0xe5 && ptr2.DIR_Name[0]!=0x00){
					if(ptr2.DIR_Attr==0x10){	//判断是否为目录 
						ptr2.DIR_Name[8] = '\0';
						FirstCluster = ptr2.DIR_FstClusLO+ptr2.DIR_FstClusHI*65538;
//						printf("DIR_Name %%s: \t%s\n", ptr2.DIR_Name);
//						printf("ptr2.DIR_FstClusLO \t%u\n", ptr2.DIR_FstClusLO);
//						printf("DIR_FstClusHI \t%u\n", ptr2.DIR_FstClusHI);
//						printf("First Cluster is %d\n", FirstCluster);
//						printf("size is %ld\n", ptr2.DIR_FileSize);
						
//						printf("%d \n", SearchNameNum);
						
						//目录是从1048576开始算的，1048576 = 1049600 - 2*512 =  DataArea - 2 * ptr.BPB_BytsPerSec 
						//目录下文件输出----- 1048576（10）=100000（16） 
						directoryArea = DataArea-2*(ptr.BPB_BytsPerSec); 
						
						fseek(fp, directoryArea+64+FirstCluster*512, SEEK_SET);	
						//20是从上面的 DIR_FstClusLO与 DIR_FstClusHI的和得到的，512是一个蔟大小， 64是因为文件目录要再加64 
						fread(buf2, 1, 32, fp);
						ptr2 = *(struct DirEntry *) buf2;
						if(ptr2.DIR_Name[0]!=0x00){
//							printf("DIR_Name %%s: \t%s\n", ptr2.DIR_Name);
//							printf("First Cluster is %d\n", FirstCluster);
//							printf("size is %ld\n", ptr2.DIR_FileSize);
//							printf("\n\n");
						}
					}
					else{
//						fseek(fp, DataArea+SearchNameNum*32, SEEK_SET);	//DataArea = 1049600 所有的 都用DataArea去写 
//						fread(buf2, 1, 32, fp);
//						ptr2 = *(struct DirEntry *) buf2;
						FirstCluster = ptr2.DIR_FstClusLO+ptr2.DIR_FstClusHI*65538;
//						printf("DIR_Name %%s: \t%s\n", ptr2.DIR_Name);
//						printf("First Cluster is %d\n", FirstCluster);
//						printf("size is %ld\n", ptr2.DIR_FileSize);
//						printf("%d \n", SearchNameNum);
						
					}
					
				}
			}
			
		}else{
			printf("file open error!\n");
		}
		fclose(fp);
		//------------------------------------------------------------------------------------------ 
		
		int recPos = 0;	//从头（1049600这个位置）开始数，32bytes为一个单位，然后第几个，恢复的文件是第几个！ 
		recPos = LastSerNum;	//这个recPos就是接口，你把要找到的位置input到这里就OK了！ 
		//--------------------------恢复文件------------------------------------------------------- 
		
		if(recPos!=0){
			fp = fopen("/dev/ram1", "rw+");
			if(fp != NULL){
				printf("You know it, haha \n");
		
				printf("\n\n");
				
				//`````````这一part可以用来修改内存，恢复里面的内容 
				//recover前要umount设备，recover后要mount设备 
				//恢复首先在1049600那里 匹配文件名，然后把第一个字符改回来
				//第二，到16384的cluster里面写EOF，就是在指定的cluster位置写ffff ff0f,
				//cluster位置是 DIR_FstClusLO与 DIR_FstClusHI得到的值乘以4byte 
				//大文件恢复是同样的道理 
				
		//		buf2[0] = argv[4][0];	// 要改的值为 输入的文件名 首字母 
		
				unsigned char buff;
				char EOFValue[4];	//用来EOF 
				EOFValue[0] = 0xff;
				EOFValue[1] = 0xff;
				EOFValue[3] = 0x0f;
				EOFValue[2] = 0xff;
				unsigned long EOFVal = 0x0fffffff;
				unsigned long FATVal = 0;
				
				fseek(fp, DataArea+recPos*32, SEEK_SET);	//DataArea = 1049600 所有的 都用DataArea去写 
				fread(buf2, 1, 32, fp);
				ptr2 = *(struct DirEntry *) buf2;
				ptr2.DIR_Name[0] = argv[4][0];
		//		ptr2.DIR_Name[0] = 0xe5;
				
				*(struct DirEntry *) buf2 = ptr2;
				
		//		memcpy(buff,ptr2.DIR_Name[0],sizeof(ptr2.DIR_Name[0]));
				
				//___________________
				fseek(fp, DataArea+recPos*32, SEEK_SET);	//DataArea = 1049600 所有的 都用DataArea去写 
				fwrite(&ptr2.DIR_Name[0], sizeof(ptr2.DIR_Name[0]), 1, fp);
				
				fread(buf2, 1, 32, fp);
				ptr2 = *(struct DirEntry *) buf2;
				printf("The File Name is need recover: \t%s\n", ptr2.DIR_Name);
				//-------------------
			
				//文件恢复： ff（16）=255； ptr2.DIR_FileSize文件大小； DIR_FstClusLO与 DIR_FstClusHI文件起始位置
				fseek(fp, DataArea+recPos*32, SEEK_SET);	//DataArea = 1049600 所有的 都用DataArea去写 
				fread(buf2, 1, 32, fp);
				ptr2 = *(struct DirEntry *) buf2;
				
				printf("Recover and the result of the File Name is \t%s\n", ptr2.DIR_Name);
				
				char FATValue[4];
				int i = 0;
				int Num = 0;
				unsigned long FstClus = 0;	//起始位置
				FstClus =  ptr2.DIR_FstClusLO+ptr2.DIR_FstClusHI*65538; 
		//		FstClus = 4;
				FATValue[0] = 0x00;	//初始化FATValue 修改于201212222339 
				FATValue[1] = 0x00;
				FATValue[2] = 0x00;
				FATValue[3] = 0x00;
				
				printf("first Cluster is %ld \n", FstClus);	//修改于201212222339 
				
				printf("FatArea+FstClus*4 is %x\n", FatArea+FstClus*4);
				Num = ptr2.DIR_FileSize/ptr.BPB_BytsPerSec;	//data的大小 除以512 就是循环次数 
				printf("Num %d \n", Num);
				FATVal =  FstClus + 1;
				for(i = 0; i <= Num-1; i++){	//这个应该是Num-1 修改于201212222339 
					fseek(fp, FatArea+4*(FstClus+i), 0);
					FATValue[0] = FATVal;
					printf("FATVal is %x\n ", FATVal);
					if(FATValue[0] == 0x00)
						FATValue[1] += 1;
					if(FATValue[1] == 0x00 && FATVal>255)	
						FATValue[2] += 1;
					if(FATValue[2] == 0x00 && FATVal>65537)	
						FATValue[3] += 1;
		//			printf("%ld ",FATVal);
					fwrite(FATValue, 4, 1, fp);
		//			printf("%d ",i);
					FATVal++;
				}
				fseek(fp, FatArea+4*(FstClus+Num), 0);	//这个应该是FstClus+Num 修改于201212222339 
				fwrite(EOFValue, 4, 1, fp);
				printf("what??2\n");
				
			}else{
				printf("file open error!\n");
			}
			fclose(fp);
			printf("---------------------------------------\n");
			//-------------------------------------------------------------------------------------- 

		}
	}
	else{
		errorUsage();
	}
}	
#endif
