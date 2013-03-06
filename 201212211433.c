#include<stdlib.h>
#include<stdio.h>
#include <string.h>

#define DEBUG	0

#pragma pack(push,1)
struct BootEntry
{
	unsigned char BS_jmpBoot[3];	/* Assembly instruction to jump to boot code */
	unsigned char BS_OEMName[8];	/* OEM Name in ASCII */
	unsigned short BPB_BytsPerSec; /* Bytes per sector. Allowed values include 512, 1024, 2048, and 4096 */
	unsigned char BPB_SecPerClus; /* Sectors per cluster (data unit). Allowed values are powers of 2, but the cluster size must be 32KB or smaller */
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

int main()
{
/*	struct BootEntry be;	// read the boot section. that's your job!

	FILE *fp = fopen("test", "r");
	char in[3];
	readcluster(fp, be, in, 3, 0);
	fclose(fp);
	printf("in = %c%c%c\n", in[0], in[1], in[2]);

	char out[3] = { 'a', 'b', 'c' };
	fp = fopen("test", "w");
	writecluster(fp, be, out, 3, 0);
	fclose(fp);
	fp = fopen("test", "r");
	readcluster(fp, be, in, 3, 0);
	fclose(fp);
	printf("in = %c%c%c\n", in[0], in[1], in[2]);
	return 0;
*/	
/*
	char *filename;
	char in[3];
	char *ptr;
	struct BootEntry be;
	if((strcmp(argv[1], "-d") == 0)&&argc>=4&&(strcmp(argv[3], "-r") == 0)){
		FILE *fp = fopen(argv[2], "rb");
		if(fp==NULL) {printf("Error:cannot find this file\n");}
		else{
			filename = argv[4];
//			memcpy(filename,argv[4],sizeof(8));
		
		}
		
	}
	else
		printf("error!!!\n");
}
*/

	unsigned char buf[10000];
	unsigned char buf2[100000];
//	struct DirEntry buf2;
	struct BootEntry ptr;
	struct DirEntry ptr2;
		
	FILE *fp = fopen("/dev/ram1", "r");
	if(fp != NULL){
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof(struct BootEntry), 1, fp);
		ptr = *(struct BootEntry *) buf;
		printf("\n\n");
		printf("BPB_BytsPerSec \t%u\n", ptr.BPB_BytsPerSec);
		printf("BPB_SecPerClus \t%u\n", ptr.BPB_SecPerClus);
		printf("BPB_RsvdSecCnt \t%u\n", ptr.BPB_RsvdSecCnt);
		printf("BPB_NumFATs \t%u\n", ptr.BPB_NumFATs);
		printf("BPB_TotSec16 \t%u\n", ptr.BPB_TotSec16);
		printf("BPB_TotSec32 \t%ld\n", ptr.BPB_TotSec32);	//long
		printf("BPB_FATSz32 \t%ld\n", ptr.BPB_FATSz32);		//long
		printf("BPB_RootClus \t%ld\n", ptr.BPB_RootClus);	//long
		
		printf("\n\n");
		
/*		//这一part可以用来解决long file name 不过貌似 long file name不需要解决 
		fseek(fp, 1049600+0, SEEK_SET);
		fread(buf2, 1, 32, fp);
		ptr2 = *(struct DirEntry *) buf2;
		printf("DIR_Name %%s: \t%s\n", ptr2.DIR_Name);
//		printf("DIR_Name %%u: \t%u\n", ptr2.DIR_Name);
		printf("DIR_Attr \t%u\n", ptr2.DIR_Attr);	//这个为15，long file name
		printf("DIR_Attr \t%x\n", ptr2.DIR_Attr);	//这个为0f，long file name
		printf("DIR_FstClusHI \t%u\n", ptr2.DIR_FstClusHI);
		printf("DIR_FstClusLO \t%u\n", ptr2.DIR_FstClusLO);
		printf("DIR_FileSize \t%ld\n", ptr2.DIR_FileSize);	//long
		printf("\n\n");
*/		

		fseek(fp, 1049600+0, SEEK_SET);
		fread(buf2, 1, 32, fp);
		ptr2 = *(struct DirEntry *) buf2;
		printf("DIR_Name %%s: \t%s\n", ptr2.DIR_Name);
//		printf("DIR_Name %%u: \t%u\n", ptr2.DIR_Name);
		printf("DIR_Attr \t%x\n", ptr2.DIR_Attr);	//这个为0f，就是long file name； 20是文件； 10是目录 
		printf("DIR_FstClusHI \t%u\n", ptr2.DIR_FstClusHI);
		printf("DIR_FstClusLO \t%u\n", ptr2.DIR_FstClusLO);
		printf("DIR_FileSize \t%ld\n", ptr2.DIR_FileSize);	//long
		
		printf("\n\n");
		
		fseek(fp, 1049600+32, SEEK_SET);
		fread(buf2, 1, 32, fp);
		ptr2 = *(struct DirEntry *) buf2;
		printf("DIR_Name %%s: \t%s\n", ptr2.DIR_Name);
//		printf("DIR_Name %%u: \t%u\n", ptr2.DIR_Name);
		printf("DIR_FstClusHI \t%u\n", ptr2.DIR_FstClusHI);
		printf("DIR_FstClusLO \t%u\n", ptr2.DIR_FstClusLO);
		printf("DIR_FileSize \t%ld\n", ptr2.DIR_FileSize);	//long
		
		printf("\n\n");
		
		fseek(fp, 1049600+64, SEEK_SET);
		fread(buf2, 1, 32, fp);
		ptr2 = *(struct DirEntry *) buf2;
		printf("DIR_Name %%s: \t%s\n", ptr2.DIR_Name);
//		printf("DIR_Name %%u: \t%u\n", ptr2.DIR_Name);
		printf("DIR_FstClusHI \t%u\n", ptr2.DIR_FstClusHI);
		printf("DIR_FstClusLO \t%u\n", ptr2.DIR_FstClusLO);
		printf("DIR_FileSize \t%ld\n", ptr2.DIR_FileSize);	//long
		
		printf("\n\n");
		
/*		//目录下文件输出----- 1048576（10）=100000（16） 
		fseek(fp, 1048576+64+20*512, SEEK_SET);	//20是从上面的 DIR_FstClusLO与 DIR_FstClusHI的和得到的，512是一个蔟大小 
		fread(buf2, 1, 32, fp);
		ptr2 = *(struct DirEntry *) buf2;
		printf("DIR_Name %%s: \t%s\n", ptr2.DIR_Name);
//		printf("DIR_Name %%u: \t%u\n", ptr2.DIR_Name);
		printf("DIR_FstClusHI \t%u\n", ptr2.DIR_FstClusHI);
		printf("DIR_FstClusLO \t%u\n", ptr2.DIR_FstClusLO);
		printf("DIR_FileSize \t%ld\n", ptr2.DIR_FileSize);	//long
*/		

		printf("\n\n");
		
		//文件内容输出 (我还没想好） 
		fseek(fp, 1048576+64+19*512, SEEK_SET);	//19是从上面的 DIR_FstClusLO与 DIR_FstClusHI的和得到的，512是一个蔟大小 
		fread(buf2, 1, 32, fp);
		ptr2 = *(struct DirEntry *) buf2;
//		while(ptr2!=NULL){
//			printf("file print out is : %c\n", ptr2);
//		}
		
		printf("\n\n");
		
	}else{
		printf("file open error!\n");
	}
	fclose(fp);
	
	fp = fopen("/dev/ram1", "rw+");
	if(fp != NULL){
		printf("You know it, haha \n");
/*		
		fseek(fp, 1049600+0, SEEK_SET);
		fread(buf2, 1, 32, fp);
		ptr2 = *(struct DirEntry *) buf2;
		printf("DIR_Name %%s: \t%s\n", ptr2.DIR_Name);
//		printf("DIR_Name %%u: \t%u\n", ptr2.DIR_Name);
		printf("DIR_Attr \t%x\n", ptr2.DIR_Attr);	//这个为0f，就是long file name； 20是文件； 10是目录 
		printf("DIR_FstClusHI \t%u\n", ptr2.DIR_FstClusHI);
		printf("DIR_FstClusLO \t%u\n", ptr2.DIR_FstClusLO);
		printf("DIR_FileSize \t%ld\n", ptr2.DIR_FileSize);	//long
		
		printf("\n\n");
		
		fseek(fp, 1049600+32, SEEK_SET);
		fread(buf2, 1, 32, fp);
		ptr2 = *(struct DirEntry *) buf2;		
		ptr2.DIR_Name[0] = 'A';
//		ptr2 = *(struct DirEntry *) buf2;
		fwrite(buf2, 1, 1, fp);

//		ptr2.DIR_Name[0] = 'A';
		printf("DIR_Name %%s: \t%s\n", ptr2.DIR_Name);
//		printf("DIR_Name %%u: \t%u\n", ptr2.DIR_Name);
		printf("DIR_Attr \t%x\n", ptr2.DIR_Attr);	//这个为0f，就是long file name； 20是文件； 10是目录 
		printf("DIR_FstClusHI \t%u\n", ptr2.DIR_FstClusHI);
		printf("DIR_FstClusLO \t%u\n", ptr2.DIR_FstClusLO);
		printf("DIR_FileSize \t%ld\n", ptr2.DIR_FileSize);	//long
*/		
		printf("\n\n");
		
		//`````````这一part可以用来修改内存 
		buf2[0] = 0x41;
//		char *k;
//		k = "AAAA";
		unsigned char buff;
		
		ptr2 = *(struct DirEntry *) buf2;
//		ptr2.DIR_Name[0] = 'D';
		ptr2.DIR_Name[0] = 0xe5;
		
		*(struct DirEntry *) buf2 = ptr2;
		
//		memcpy(buff,ptr2.DIR_Name[0],sizeof(ptr2.DIR_Name[0]));
		
		fseek(fp, 1049600+128, 0);
		fwrite(&ptr2.DIR_Name[0], sizeof(ptr2.DIR_Name[0]), 1, fp);
		
		fread(buf2, 1, 32, fp);
		ptr2 = *(struct DirEntry *) buf2;
		printf("DIR_Name %%s: \t%s\n", ptr2.DIR_Name);
		
	}else{
		printf("file open error!\n");
	}
	fclose(fp);
//	fflush(fp);   //清空输入流
//	fflush(fp); //清空输出流
//	fp = NULL; 
	
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
#endif
