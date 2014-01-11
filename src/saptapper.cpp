/*
** Simple utility to convert a GBA Rom to a GSF.
** Based on EXE2PSF code, written by Neill Corlett
** Released under the terms of the GNU General Public License
**
** You need zlib to compile this.
** It's available at http://www.gzip.org/zlib/
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "zlib.h"

Byte   compbuf[0x2000000];
Byte uncompbuf[0x2000000];

Byte selectsong[0x1E] = {
	0x00, 0xB5, 0x00, 0x04, 0x07, 0x4A, 0x08, 0x49, 
	0x40, 0x0B, 0x40, 0x18, 0x83, 0x88, 0x59, 0x00, 
	0xC9, 0x18, 0x89, 0x00, 0x89, 0x18, 0x0A, 0x68, 
	0x01, 0x68, 0x10, 0x1C, 0x00, 0xF0,
};

#define INIT_COUNT 2
Byte init[2][INIT_COUNT] = { 
	{0x70, 0xB5},
	{0xF0, 0xB5}
};

Byte soundmain[2] = {
	0x00, 0xB5,
};

Byte vsync[5] = {
	0x4A, 0x03, 0x68, 0x9B, 0x1A,
};

unsigned char sappyblock[248] =
{
    0x00, 0x80, 0x2D, 0xE9, 0x01, 0x00, 0xBD, 0xE8, 0x50, 0x10, 0xA0, 0xE3, 0x00, 0x20, 0x90, 0xE5, 
    0x04, 0x00, 0x80, 0xE2, 0x04, 0x10, 0x41, 0xE2, 0x00, 0x00, 0x51, 0xE3, 0xFA, 0xFF, 0xFF, 0x1A, 
    0x0B, 0x00, 0x00, 0xEA, 0x53, 0x61, 0x70, 0x70, 0x79, 0x20, 0x44, 0x72, 0x69, 0x76, 0x65, 0x72, 
    0x20, 0x52, 0x69, 0x70, 0x70, 0x65, 0x72, 0x20, 0x62, 0x79, 0x20, 0x43, 0x61, 0x69, 0x74, 0x53, 
    0x69, 0x74, 0x68, 0x32, 0x5C, 0x5A, 0x6F, 0x6F, 0x70, 0x64, 0x2C, 0x20, 0x28, 0x63, 0x29, 0x20, 
    0x32, 0x30, 0x30, 0x34, 0x00, 0x40, 0x2D, 0xE9, 0x80, 0x00, 0x9F, 0xE5, 0x21, 0x00, 0x00, 0xEB, 
    0x88, 0x00, 0x9F, 0xE5, 0x00, 0x80, 0x2D, 0xE9, 0x02, 0x00, 0xBD, 0xE8, 0x30, 0x10, 0x81, 0xE2, 
    0x00, 0x10, 0x80, 0xE5, 0x01, 0x03, 0xA0, 0xE3, 0x08, 0x10, 0xA0, 0xE3, 0x04, 0x10, 0x80, 0xE5, 
    0x01, 0x10, 0xA0, 0xE3, 0x00, 0x12, 0x80, 0xE5, 0x08, 0x12, 0x80, 0xE5, 0x60, 0x00, 0x9F, 0xE5, 
    0x40, 0x10, 0x9F, 0xE5, 0x14, 0x00, 0x00, 0xEB, 0x00, 0x00, 0x02, 0xEF, 0xFD, 0xFF, 0xFF, 0xEA, 
    0x00, 0x40, 0x2D, 0xE9, 0x38, 0x00, 0x9F, 0xE5, 0x0E, 0x00, 0x00, 0xEB, 0x28, 0x00, 0x9F, 0xE5, 
    0x0C, 0x00, 0x00, 0xEB, 0x01, 0x03, 0xA0, 0xE3, 0x01, 0x18, 0xA0, 0xE3, 0x01, 0x10, 0x81, 0xE2, 
    0x00, 0x12, 0x80, 0xE5, 0x24, 0x00, 0x9F, 0xE5, 0x04, 0x10, 0x00, 0xE5, 0x01, 0x00, 0xBD, 0xE8, 
    0x10, 0xFF, 0x2F, 0xE1, 0xFF, 0xFF, 0xFF, 0xFF, 0x59, 0x81, 0x03, 0x08, 0x4D, 0x81, 0x03, 0x08, 
    0xD5, 0x80, 0x03, 0x08, 0x89, 0x7A, 0x03, 0x08, 0x10, 0xFF, 0x2F, 0xE1, 0x11, 0xFF, 0x2F, 0xE1, 
    0xFC, 0x7F, 0x00, 0x03, 0x30, 0x00, 0x00, 0x00, 
} ;



uLong minigsfoffset;
uLong minigsfcount;

uLong entrypoint;
uLong load_offset;
uLong rom_size;

uLong sappyoffset;
int manual=0;

FILE *bat=NULL;








unsigned char hexdigit(unsigned char digit)
{
	if(digit>='0'&&digit<='9')
		return digit-'0';
	if(digit>='A'&&digit<='F')
		return digit-'A'+10;
    if(digit>='a'&&digit<='f')
		return digit-'a'+10;
	return 0;
}

unsigned long ahextoi(const char *buffer)
{
	char *buf = (char*) buffer;
	int i=0;
	unsigned long integer=0;
	while(*buf)
	{
		integer<<=4;
		integer+=hexdigit(*buf);
		buf++;
		
	}

	return integer;
}


int isduplicate(int num)
{
	int i,j;
	Byte *rom = uncompbuf+12;
	unsigned char data[8];

	for(i=0;i<8;i++)
	{
		data[i]=rom[sappyoffset+(num*8)+i];
	}

	if(num==0)
		return 0;
	else
	{
		for(i=0;i<num;i++)
		{
			for(j=0;j<8;j++)
			{
				if(data[j]!=rom[sappyoffset+(i*8)+j])
					break;
			}
			if(j==8)
				break;
		}
		if(i!=num)
			return 1;
	}
	return 0;

	
}


int doexe2gsf(unsigned long offset, int size, unsigned short num, const char *to, const char *base) {
  FILE *f;
  uLong ucl;
  uLong cl;
  uLong ccrc;

  unsigned char byte=num&0xFF;
  unsigned short half=num&0xFFFF;



  int r;

  


//  fprintf(stderr, "%s->%s: ", from, to);

  ucl = size;

  
  
  entrypoint = offset & 0xFF000000;
  load_offset = offset;
  rom_size = size;
  memcpy(uncompbuf, &entrypoint, sizeof(entrypoint));
  memcpy(uncompbuf+4, &load_offset, sizeof(load_offset));
  memcpy(uncompbuf+8, &rom_size, sizeof(rom_size));
  if(size==1)
      memcpy(uncompbuf+12,&byte,sizeof(byte));
  else
      memcpy(uncompbuf+12,&half,sizeof(half));
  
//  fprintf(stdout,"uncompressed: %ld bytes\n",ucl);fflush(stdout);

  cl = sizeof(compbuf);
  r=compress2(compbuf,&cl,uncompbuf,ucl+12,9);
  if(r!=Z_OK){fprintf(stderr,"zlib compress2() failed (%d)\n", r);return 1;}

//  fprintf(stdout,"compressed: %ld bytes\n",cl);fflush(stdout);

  f=fopen(to,"wb");if(!f){perror(to);return 1;}
  fputc('P',f);fputc('S',f);fputc('F',f);fputc(0x22,f);
  fputc(0,f);fputc(0,f);fputc(0,f);fputc(0,f);
  fputc(cl  >> 0,f);
  fputc(cl  >> 8,f);
  fputc(cl  >>16,f);
  fputc(cl  >>24,f);
  ccrc=crc32(crc32(0L, Z_NULL, 0), compbuf, cl);
  fputc(ccrc>> 0,f);
  fputc(ccrc>> 8,f);
  fputc(ccrc>>16,f);
  fputc(ccrc>>24,f);
  fwrite(compbuf,1,cl,f);
  //fwrite(base,1,sizeof(base),f);
  fprintf(f,base);

  fclose(f);
  fprintf(stderr,".");
  return 0;
}



#define	GSFLIB_OK		0
#define GSFLIB_NOMAIN	1
#define GSFLIB_NOSELECT 2
#define GSFLIB_NOINIT   3
#define GSFLIB_NOVSYNC  4
#define GSFLIB_NOSPACE  5
#define GSFLIB_ZLIB_ERR 6
#define GSFLIB_INFILE_E 7
#define GSFLIB_ROM_WR   8
#define GSFLIB_OTFILE_E 9
#define GSFLIB_DIR_ERR  10


int dogsflib(const char *from, const char *to) {
  FILE *f;
  uLong ucl;
  uLong cl;
  uLong ccrc;
  int r;
  Byte *rom = uncompbuf+12;
  int i,j,k,rompointer;
  unsigned long fileattr;
  int result;

  char s[1000];

  for(i=0;i<1000;i++)
	  s[i]=0;

  i = (int)(strchr(from, '.') - from); 
  strncpy(s, from, i);

  


  minigsfcount=0;
  fprintf(stderr, "%s: ", to);

  f=fopen(from,"rb");if(!f){perror(from);return GSFLIB_INFILE_E;}
  ucl=(uLong)fread(uncompbuf+12,1,sizeof(uncompbuf)-12,f);
  fclose(f);
  
  entrypoint = load_offset = 0x8000000;
  rom_size = ucl;
  memcpy(uncompbuf, &entrypoint, sizeof(entrypoint));
  memcpy(uncompbuf+4, &load_offset, sizeof(load_offset));
  memcpy(uncompbuf+8, &rom_size, sizeof(rom_size));

  char stroffset[256];

  int diff=0;
  j = 0;
  k=0;
selectcontinue:
  for(i=k;i<rom_size;i++)
  {
	if(rom[i]==selectsong[j])
	{
		for(j=0;j<0x1E;j++)
		{
			if(rom[i+j]!=selectsong[j])
			{
				diff++;
				//j = 0;
				//break;
			}
		}
		
		//if(j!=0x1E)
		//	j=0;
		//else
		if(diff<8)
			break;
		else
		{
			j=0;
			diff=0;
		}
	}
  }
  if(diff<8)
  {
	  sappyoffset = rom[i+40] + (rom[i+41] << 8) + (rom[i+42] << 16) + ((rom[i+43] & 0x01) << 24); 
	  unsigned long sappyoffset2;
	  sappyoffset2=sappyoffset;
	  while((rom[sappyoffset2+3] & 0x0E) == 0x08)
	  {
		  minigsfcount++;
		  sappyoffset2+=8;
	  }
	  if(!minigsfcount)
	  {
		  k=i+1;
		  goto selectcontinue;
	  }
	  sappyblock[0xD8] = (i & 0xFF) + 1;
	  sappyblock[0xD9] = (i >> 8) & 0xFF;
	  sappyblock[0xDA] = (i >> 16) & 0xFF;
  }
  else
  {
	 // fprintf(stderr,"Unable to Locate sappy_selectsongbynum\n");
	  return GSFLIB_NOSELECT;
  }
  j=0;
  rompointer=i-0x20;
  for(i--;i>rompointer;i--)
  {
	if(rom[i]==soundmain[j])
	{
		for(j=0;j<0x2;j++)
		{
			if(rom[i+j]!=soundmain[j])
			{
				j = 0;
				break;
			}
		}
		if(j!=0x2)
			j=0;
		else
			break;
    }
  }
  if(i==rompointer)
  {
//	  fprintf(stderr,"Unable to locate sappy_Soundmain\n");
	  if(manual)
	  {
		fprintf(stderr,"Enter a hex offset, or 0 to cancel - ");
		gets(stroffset);
		i=ahextoi(stroffset);
		if(!i)
			return GSFLIB_NOMAIN;
		manual=2;
	  }
	  else
		  return GSFLIB_NOMAIN;
  }
  	  sappyblock[0xDC] = (i & 0xFF) + 1;
	  sappyblock[0xDD] = (i >> 8) & 0xFF;
	  sappyblock[0xDE] = (i >> 16) & 0xFF;
  j=0;
  rompointer=i-0x100;
  for(i--;i>rompointer;i--)
  {
    for(k=0;k<INIT_COUNT;k++)
	{
		if(rom[i]==init[k][j]) { result=1; break;}
		else result=0;
	}
	if(result)
	{
		for(k=0;k<INIT_COUNT;k++)
		{
			for(j=0;j<0x2;j++)
			{
				if(rom[i+j]!=init[k][j])
				{
					j = 0;
					break;
				}
			}
			if(j==2) break;
		}
		if(j!=0x2)
			j=0;
		else
			break;
	}
  }
  if(i==rompointer)
  {
	//  fprintf(stderr,"Unable to locate sappy_SoundInit\n");
	    if(manual)
	  {
		fprintf(stderr,"Enter a hex offset, or 0 to cancel - ");
		gets(stroffset);
		i=ahextoi(stroffset);
		if(!i)
			return GSFLIB_NOINIT;
		manual=2;
	  }
	  else
		  return GSFLIB_NOINIT;
  }
  	  sappyblock[0xE0] = (i & 0xFF) + 1;
	  sappyblock[0xE1] = (i >> 8) & 0xFF;
	  sappyblock[0xE2] = (i >> 16) & 0xFF;
  j=0;
  //i+=0x1800;
  rompointer=i-0x800;
  for(i--;(i>0 && i>rompointer);i--)
  {
	if(rom[i]==vsync[j])
	{
		for(j=0;j<0x5;j++)
		{
			if(rom[i+j]!=vsync[j])
			{
				j = 0;
				break;
			}
		}
		if(j!=0x5)
			j=0;
		else
			break;
	}
  }

  if(i==rompointer || i==0)
  {
//	  fprintf(stderr,"Unable to locate sappy_VSync\n");
	  if(manual)
	  {
		fprintf(stderr,"Enter a hex offset, or 0 to cancel - ");
		gets(stroffset);
		i=ahextoi(stroffset);
		if(!i)
			return GSFLIB_NOVSYNC;
		manual=2;
	  }
	  else
		  return GSFLIB_NOVSYNC;
  }
  else
  {
	i-=5;
  }
  	  sappyblock[0xE4] = (i & 0xFF) + 1;
	  sappyblock[0xE5] = (i >> 8) & 0xFF;
	  sappyblock[0xE6] = (i >> 16) & 0xFF;

  rompointer=0xFF;
lookforspace:
  for(i=0;i<rom_size;i+=4)
  {
	  if(rom[i]==(unsigned char)rompointer)
	  {
		  for (j=0;j<0x200;j++)
		  {
			  if(rom[i+j]!=(unsigned char)rompointer)
			  {
				  j=0;
				  break;
			  }
		  }
		  if(j!=0x200)
			  j=0;
		  else
			  break;
	  }
  }
  if(j==0x200)
  {
	  minigsfoffset=0x8000000+i+0x1FC;
	  i+=0x108;
	  memcpy(rom+i,sappyblock,sizeof(sappyblock));
	  i>>=2;
	  i-=2;
	  rom[0]=(i&0xFF);
	  rom[1]=(i>>8)&0xFF;
	  rom[2]=(i>>16)&0xFF;
  }
  else
  {
	  if(rompointer==0xFF)
	  {
		  rompointer=0x00;
		  goto lookforspace;
	  }
	  else
	  {
	//	  fprintf(stderr,"Unable to find sufficient unprogrammed space\n");
		  return GSFLIB_NOSPACE;
	  }
  }




	CreateDirectoryA(s,NULL);

	fileattr = GetFileAttributesA(s);
	if ((!(fileattr&FILE_ATTRIBUTE_DIRECTORY))||(fileattr==-1))
	{
		printf("Directory %s could not be created\n",s);
		return -1;
	}

	SetCurrentDirectoryA(s);

  
//  fprintf(stdout,"uncompressed: %ld bytes\n",ucl);fflush(stdout);

  cl = sizeof(compbuf);
  r=compress2(compbuf,&cl,uncompbuf,ucl+12,1);
  if(r!=Z_OK){/*fprintf(stderr,"zlib compress2() failed (%d)\n", r);*/return GSFLIB_ZLIB_ERR;}




//  fprintf(stdout,"compressed: %ld bytes\n",cl);fflush(stdout);

  f=fopen(to,"wb");if(!f){perror(to);return GSFLIB_OTFILE_E;}

//#define WriteRom
#ifdef WriteRom
  fwrite(rom,1,rom_size,f);
  fclose(f);
 // fprintf(stderr,"Writing normal rom file\n");
  return GSFLIB_ROM_WR;
#endif
  fputc('P',f);fputc('S',f);fputc('F',f);fputc(0x22,f);
  fputc(0,f);fputc(0,f);fputc(0,f);fputc(0,f);
  fputc(cl  >> 0,f);
  fputc(cl  >> 8,f);
  fputc(cl  >>16,f);
  fputc(cl  >>24,f);
  ccrc=crc32(crc32(0L, Z_NULL, 0), compbuf, cl);
  fputc(ccrc>> 0,f);
  fputc(ccrc>> 8,f);
  fputc(ccrc>>16,f);
  fputc(ccrc>>24,f);
  fwrite(compbuf,1,cl,f);
  fclose(f);
  fprintf(stderr,"ok\n");

  if(bat)
  {
	fprintf(bat,"cd \"%s\"\n",s);
	fprintf(bat,"gsfopt -l *.minigsf\n");
//	fprintf(bat,"del \"%s\".gsflib\n",s);
//	fprintf(bat,"ren \"%s\"-0000.gsflib \"%s\".gsflib\n",s,s);
	fprintf(bat,"cd ..\n");
  }

  return GSFLIB_OK;
}




int main(int argc, char **argv) {
  char s[1000];
  char t[1000];
  char gsflibname[1000];
  char minigsfname[1000];
  char nickname[64];
  unsigned long fileattr;
  int i, j, k;
  int errors = 0;
//  int count;
  int dogsf=0;
  FILE *log=NULL;
//  unsigned long offset;
  int size;
  if(argc<2){
    fprintf(stderr,"usage: %s <GBA Files>\n", argv[0]);
    return 1;
  }
//  size=atoi(argv[3]);
//  count=atoi(argv[4]);
//  offset=ahextoi(argv[2]);

  log=fopen("saptapper.txt","w");
  bat=fopen("optimize.bat","w");

 // if(strcmp(argv[1],"-m"))
//	  manual=1;
//  else
	  manual=0;


  

  /*fprintf(stderr,"argc = %d\n",argc);
  for (j=2; j<argc; j++)
	  fprintf(stderr,"argv[%d] = %s\n",j,argv[j]);*/

  for (j=1; j<argc; j++)
  {
    fileattr = GetFileAttributesA(argv[j]);
    if (((fileattr&FILE_ATTRIBUTE_DIRECTORY))||(fileattr==-1))
	{
	//	printf("Directory %s could not be created\n",s);
		continue;
	}
	k = (int)(strchr(argv[j], '.') - argv[j]); 
	memset(gsflibname,0,sizeof(gsflibname));
	strncpy(gsflibname, argv[j], k);
	strcpy(gsflibname+k, ".gsflib");

	//sprintf(s,"%s.gba",argv[j]);
	//sprintf(t,"%s.gsflib",argv[j]);
	dogsf=dogsflib(argv[j],gsflibname);
	if(dogsf)
	{
		fprintf(stderr, "error\n");
		switch(dogsf)
		{
		case GSFLIB_OTFILE_E:
		case GSFLIB_ROM_WR:
		case GSFLIB_ZLIB_ERR:
			SetCurrentDirectoryA("..");
		default:
			break;
		}
		if(log)
		{
			switch (dogsf)
			{
			case GSFLIB_NOMAIN:
				fprintf(log,"sappy_main not found in %s\n",argv[j]);
				break;
			case GSFLIB_NOINIT:
				fprintf(log,"sappy_init not found in %s\n",argv[j]);
				break;
			case GSFLIB_NOVSYNC:
				fprintf(log,"sappy_vsync not found in %s\n",argv[j]);
				break;
			case GSFLIB_NOSPACE:
				fprintf(log,"Insufficient space found in %s\n",argv[j]);
				break;
			case GSFLIB_ZLIB_ERR:
				fprintf(log,"GSFLIB zlib compression error\n");
				break;
			case GSFLIB_OTFILE_E:
				fprintf(log,"Error Writing file %s\n",gsflibname);
				break;
			case GSFLIB_NOSELECT:
			case GSFLIB_INFILE_E:
			default:
				break;
			}
		}

		continue;
	}
	if(minigsfcount>255)
		size=2;
	else
		size=1;

	if(manual==2)
	{
		printf("Enter your name for GSFby purposes.\n");
		printf("The GSFBy tag will look like,\n");
		printf("Saptapper, with help of <your name here>\n");
		gets(nickname);
		if(!strcmp(nickname,"CaitSith2"))
			sprintf(t,"[TAG]_lib=%s\ngsfby=CaitSith2\n", gsflibname);
		else
			sprintf(t,"[TAG]_lib=%s\ngsfby=Saptapper, with help of %s\n", gsflibname,nickname);
		manual=1;
	}
	else
		sprintf(t,"[TAG]_lib=%s\ngsfby=Saptapper\n", gsflibname);
	for(i = 0; i < minigsfcount; i++) {
		memset(minigsfname,0,sizeof(minigsfname));
		k = (int)(strchr(argv[j], '.') - argv[j]); 
		strncpy(minigsfname, argv[j], k);

		sprintf(s,"%s.%.4X.minigsf",minigsfname,i);
		{ char *e = s + strlen(s) - 4;
		if(!strcmp(e, ".gba")) { *e = 0; }
		}


	//  strcat(s, ".gsf");
		if(isduplicate(i)==0)
			errors += doexe2gsf(minigsfoffset,size,i, s, t);
		else
			fprintf(stderr,"|");

	}

	SetCurrentDirectoryA("..");
	fprintf(stderr,"\n");

  }
  fprintf(stderr, "%d error(s)\n", errors);
	if(log)
	{
		fclose(log);
		log=NULL;
	}
	return 0;
  
}
