#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
short count=0;
short num=0;
void write_bootblock(FILE *image, FILE *bbfile, Elf32_Phdr *Phdr);
Elf32_Phdr *read_exec_file(FILE *opfile);
uint8_t count_kernel_sectors(Elf32_Phdr *Phdr);
void extent_opt(Elf32_Phdr *Phdr_bb, Elf32_Phdr *Phdr_k, int kernelsz);

Elf32_Phdr *read_exec_file(FILE *opfile)
{
  void *head;
  head=malloc(52);
  fread(head,52,1,opfile);
  int offset;
  offset=*(int *)(head+28);
  short size;
  size=*(short *)(head+42);
  num=*(short *)(head+44);
  if(count==0){
      count=num-1;
  void *pro_text;
  pro_text=malloc(size);
  fread(pro_text,size,1,opfile);
  Elf32_Phdr *program;
  program=malloc(32);
  program->p_type=*(Elf32_Word*)(pro_text);
  program->p_offset=*(Elf32_Off*)(pro_text+4);
  program->p_vaddr=*(Elf32_Addr*)(pro_text+8);
  program->p_paddr=*(Elf32_Addr*)(pro_text+12);
  program->p_filesz=*(Elf32_Word*)(pro_text+16);
  program->p_memsz=*(Elf32_Word*)(pro_text+20);
  program->p_flags=*(Elf32_Word*)(pro_text+24);
  program->p_align=*(Elf32_Word*)(pro_text+28);
  return program;
  }
  else{
    short i=num-count;
    count--;
    fseek(opfile,size*i+52,SEEK_SET);
    Elf32_Phdr *program;
    void *pro_text;
    pro_text=malloc(size);
    fread(pro_text,size,1,opfile);
    program=malloc(32);
    program->p_type=*(Elf32_Word*)(pro_text);
    program->p_offset=*(Elf32_Off*)(pro_text+4);
    program->p_vaddr=*(Elf32_Addr*)(pro_text+8);
    program->p_paddr=*(Elf32_Addr*)(pro_text+12);
    program->p_filesz=*(Elf32_Word*)(pro_text+16);
    program->p_memsz=*(Elf32_Word*)(pro_text+20);
    program->p_flags=*(Elf32_Word*)(pro_text+24);
    program->p_align=*(Elf32_Word*)(pro_text+28);
    return program;
}
}

uint8_t count_kernel_sectors(Elf32_Phdr *Phdr)
{
  int num;
  num=Phdr->p_memsz;
  num=num/512;
  return num;
}

void write_bootblock(FILE *image, FILE *file, Elf32_Phdr *phdr)
{
  void *block;
  block=malloc(phdr->p_memsz);
  fseek(file,phdr->p_offset,SEEK_SET);
  fread(block,phdr->p_memsz,1,file);
  fseek(image,phdr->p_vaddr-0xa0800000,SEEK_SET);
  fwrite(block,phdr->p_memsz,1,image);
  short *end;
  end=malloc(2);
  *end=0xaa55;
  fseek(image,0x000001fe,SEEK_SET);
  fwrite((void*)end,2,1,image);
}

void write_kernel(FILE *image, FILE *knfile, Elf32_Phdr *Phdr, int kernelsz)
{
  void *block;
  block=malloc(Phdr->p_filesz);
  fseek(knfile,Phdr->p_offset,SEEK_SET);
  fread(block,Phdr->p_filesz,1,knfile);
  fseek(image,Phdr->p_vaddr-0xa0800000,SEEK_SET);
  fwrite(block,Phdr->p_filesz,1,image);
  char *zero;
  zero=malloc(1);
  *zero=0;
  fseek(image,Phdr->p_vaddr+Phdr->p_filesz-0xa0800000+1,SEEK_SET);
  fwrite((void*)zero,1,Phdr->p_memsz-Phdr->p_filesz,image);
}

void record_kernel_sectors(FILE *image, int kernelsz)
{
  int *size,knum;
  knum=kernelsz/512;
  *size=(knum<1)?1:knum;
  fseek(image,0x000001f7,SEEK_SET);
  fwrite((void*)size,4,1,image);
}

void extent_opt(Elf32_Phdr *Phdr_bb, Elf32_Phdr *Phdr_k, int kernelsz)
{
  printf("block type:%x\n",Phdr_bb->p_type);
  printf("block offset:%x\n",Phdr_bb->p_offset);
  printf("block vaddr:%x\n",Phdr_bb->p_vaddr);
  printf("block paddr:%x\n",Phdr_bb->p_paddr);
  printf("block filesz:%x\n",Phdr_bb->p_filesz);
  printf("block memsz:%x\n",Phdr_bb->p_memsz);
  printf("block flags:%x\n",Phdr_bb->p_flags);
  printf("block align:%x\n",Phdr_bb->p_align);
  printf("kernel type:%x\n",Phdr_k->p_type);
  printf("kernel offset:%x\n",Phdr_k->p_offset);
  printf("kernel vaddr:%x\n",Phdr_k->p_vaddr);
  printf("kernel paddr:%x\n",Phdr_k->p_paddr);
  printf("kernel filesz:%x\n",Phdr_k->p_filesz);
  printf("kernel memsz:%x\n",Phdr_k->p_memsz);
  printf("kernel flags:%x\n",Phdr_k->p_flags);
  printf("kernel align:%x\n",Phdr_k->p_align);
}

int main()
{
  FILE *ker,*boot,*ima;
  ker=fopen("main","rb");
  boot=fopen("bootblock","rb");
  ima=fopen("image","wb");
  Elf32_Phdr *Phdr1,*Phdr2;
  Phdr1=read_exec_file(boot);
  write_bootblock(ima,boot,Phdr1);
  while(count!=0){
      Phdr1=read_exec_file(boot);
      write_bootblock(ima,boot,Phdr1);
  }
  count=0;
  Elf32_Phdr *Phdr21;
  Phdr2=read_exec_file(ker);
  int kernelsz=Phdr2->p_memsz;
  write_kernel(ima,ker,Phdr2,kernelsz);
  while(count!=0){
      Phdr2=read_exec_file(ker);
      kernelsz=kernelsz+Phdr2->p_memsz;
      write_kernel(ima,ker,Phdr2,kernelsz);
  }
  record_kernel_sectors(ima,kernelsz);
  extent_opt(Phdr1,Phdr2,kernelsz);
  fclose(ker);
  fclose(boot);
  fclose(ima);
  return 0;
}
