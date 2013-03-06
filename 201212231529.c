#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<stdbool.h>  //����bool 

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

	char filename[11];	//��ʱ���ټӳ��ļ��� 
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
	
		fseek(fp, DataArea+0, SEEK_SET);	//DataArea = 1049600 ���е� ����DataAreaȥд 
		fread(buf2, 1, 32, fp);
		ptr2 = *(struct DirEntry *) buf2;
		printf("DIR_Name %%s: \t%s\n", ptr2.DIR_Name);
//		printf("DIR_Name %%u: \t%u\n", ptr2.DIR_Name);
		printf("DIR_Attr \t%x\n", ptr2.DIR_Attr);	//���Ϊ0f������long file name�� 20���ļ��� 10��Ŀ¼ 
		printf("DIR_FstClusHI \t%u\n", ptr2.DIR_FstClusHI);
		printf("DIR_FstClusLO \t%u\n", ptr2.DIR_FstClusLO);	//�����ж������ĸ�cluster���� 
		printf("DIR_FileSize \t%ld\n", ptr2.DIR_FileSize);	//long
		
		printf("\n\n");
				
	}else{
		printf("file open error!\n");
	}
	fclose(fp);
	
	if(argc>4){
		//-------------------------ƥ���ļ����ģ�δ��ɣ�-------------------------------- 
		printf("argv[4] is %s \n", argv[4]); 
		printf("argv[4][0] is %c \n", argv[4][0]);
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
	//	printf("%s ",strstr("ABBB","B"));
		int argvii = 0;
		bool ExtenHave = 0;	//bool ExtenHave, �ж���չ����־ ��.�� �Ƿ���� 
		int PositExt = 0;
		for(argvii = 0; argvii<11; argvii++){
			if(argv[4][argvii]=='.'){
				printf("make the position %d \n", argvii+1);
				ExtenHave = 1;
				PositExt = argvii;
			}
		}
		printf("make the position %d \n", argvii+1);
		
		int SearchNameNum = 0;	//SearchNameNum = 0
		int LastSerNum = 0; 
		char FileName[100][11];
		fp = fopen("/dev/ram1", "r");
		if(fp != NULL){
			for(SearchNameNum = 0; SearchNameNum < 20; SearchNameNum++){
				fseek(fp, DataArea+SearchNameNum*32, SEEK_SET);	//DataArea = 1049600 ���е� ����DataAreaȥд 
				fread(buf2, 1, 32, fp);
				ptr2 = *(struct DirEntry *) buf2;
				if(ptr2.DIR_Name[0]!=0xe5 && ptr2.DIR_Name[0]!=0x00){
					if(ptr2.DIR_Attr==0x10){	//�ж��Ƿ�ΪĿ¼ 
						ptr2.DIR_Name[8] = '\0';
						printf("DIR_Name %%s: \t%s\n", ptr2.DIR_Name);
						printf("%d \n", SearchNameNum);
					}
					else{
						printf("DIR_Name %%s: \t%s\n", ptr2.DIR_Name);
						printf("%d \n", SearchNameNum);
						if(ExtenHave){
							if(ptr2.DIR_Name[8]==argv[4][PositExt+1]&&ptr2.DIR_Name[9]==argv[4][PositExt+2]&&ptr2.DIR_Name[10]==argv[4][PositExt+3])
							{
								LastSerNum = SearchNameNum;
							}
						}
						else{
							//���кܶ�&& ����д�� 
							if(ptr2.DIR_Name[1]==argv[4][1]&&ptr2.DIR_Name[2]==argv[4][2]&&ptr2.DIR_Name[3]==argv[4][3]&&ptr2.DIR_Name[4]==argv[4][4])
							{
								LastSerNum = SearchNameNum;
							}
						}
					}
					
				}
			}
			
		}else{
			printf("file open error!\n");
		}
		fclose(fp);
		//------------------------------------------------------------------------------------------ 
		
		int recPos = 0;	//��ͷ��1049600���λ�ã���ʼ����32bytesΪһ����λ��Ȼ��ڼ������ָ����ļ��ǵڼ����� 
		recPos = 7;	//���recPos���ǽӿڣ����Ҫ�ҵ���λ��input�������OK�ˣ� 
		//--------------------------�ָ��ļ�------------------------------------------------------- 
		fp = fopen("/dev/ram1", "rw+");
		if(fp != NULL){
			printf("You know it, haha \n");
	
			printf("\n\n");
			
			//`````````��һpart���������޸��ڴ棬�ָ���������� 
			//recoverǰҪumount�豸��recover��Ҫmount�豸 
			//�ָ�������1049600���� ƥ���ļ�����Ȼ��ѵ�һ���ַ��Ļ���
			//�ڶ�����16384��cluster����дEOF��������ָ����clusterλ��дffff ff0f,
			//clusterλ���� DIR_FstClusLO�� DIR_FstClusHI�õ���ֵ����4byte 
			//���ļ��ָ���ͬ���ĵ��� 
			
	//		buf2[0] = argv[4][0];	// Ҫ�ĵ�ֵΪ ������ļ��� ����ĸ 
	
			unsigned char buff;
			char EOFValue[4];	//����EOF 
			EOFValue[0] = 0xff;
			EOFValue[1] = 0xff;
			EOFValue[3] = 0x0f;
			EOFValue[2] = 0xff;
			unsigned long EOFVal = 0x0fffffff;
			unsigned long FATVal = 0;
			
			fseek(fp, DataArea+recPos*32, SEEK_SET);	//DataArea = 1049600 ���е� ����DataAreaȥд 
			fread(buf2, 1, 32, fp);
			ptr2 = *(struct DirEntry *) buf2;
			ptr2.DIR_Name[0] = argv[4][0];
	//		ptr2.DIR_Name[0] = 0xe5;
			
			*(struct DirEntry *) buf2 = ptr2;
			
	//		memcpy(buff,ptr2.DIR_Name[0],sizeof(ptr2.DIR_Name[0]));
			
			//___________________
			fseek(fp, DataArea+recPos*32, SEEK_SET);	//DataArea = 1049600 ���е� ����DataAreaȥд 
			fwrite(&ptr2.DIR_Name[0], sizeof(ptr2.DIR_Name[0]), 1, fp);
			
			fread(buf2, 1, 32, fp);
			ptr2 = *(struct DirEntry *) buf2;
			printf("DIR_Name %%s: \t%s\n", ptr2.DIR_Name);
			//-------------------
		
			//�ļ��ָ��� ff��16��=255�� ptr2.DIR_FileSize�ļ���С�� DIR_FstClusLO�� DIR_FstClusHI�ļ���ʼλ��
			fseek(fp, DataArea+recPos*32, SEEK_SET);	//DataArea = 1049600 ���е� ����DataAreaȥд 
			fread(buf2, 1, 32, fp);
			ptr2 = *(struct DirEntry *) buf2;
			
			printf("DIR_Name %%s: \t%s\n", ptr2.DIR_Name);
			
			char FATValue[4];
			int i = 0;
			int Num = 0;
			unsigned long FstClus = 0;	//��ʼλ��
			FstClus =  ptr2.DIR_FstClusLO+ptr2.DIR_FstClusHI*65538; 
	//		FstClus = 4;
			FATValue[0] = 0x00;	//��ʼ��FATValue �޸���201212222339 
			FATValue[1] = 0x00;
			FATValue[2] = 0x00;
			FATValue[3] = 0x00;
			
			printf("first Cluster is %ld \n", FstClus);	//�޸���201212222339 
			
			printf("FatArea+FstClus*4 is %x\n", FatArea+FstClus*4);
			Num = ptr2.DIR_FileSize/ptr.BPB_BytsPerSec;	//data�Ĵ�С ����512 ����ѭ������ 
			printf("Num %d \n", Num);
			FATVal =  FstClus + 1;
			for(i = 0; i <= Num-1; i++){	//���Ӧ����Num-1 �޸���201212222339 
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
			fseek(fp, FatArea+4*(FstClus+Num), 0);	//���Ӧ����FstClus+Num �޸���201212222339 
			fwrite(EOFValue, 4, 1, fp);
			printf("what??2\n");
			
		}else{
			printf("file open error!\n");
		}
		fclose(fp);
		//-------------------------------------------------------------------------------------- 
		
		fp = fopen("/dev/ram1", "r");
		if(fp != NULL){
			fseek(fp, 1049600+0, 0);
			
			fread(buf2, 1, 32, fp);
			ptr2 = *(struct DirEntry *) buf2;
			printf("DIR_Name %%s: \t%s\n", ptr2.DIR_Name);
					
		}else{
			printf("file open error!\n");
		}
		
		fclose(fp);
	} 
}	
#endif
