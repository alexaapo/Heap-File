#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "heap_file.h"


#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HP_ERROR;        \
  }                         \
}

HP_ErrorCode HP_Init() {
  //insert code here
  return HP_OK;
}

HP_ErrorCode HP_CreateFile(const char *filename) {
  //Create a file with name filename.
  CALL_BF(BF_CreateFile(filename));
  
  //Open the file with name filename.
  int file_desc;
  CALL_BF(BF_OpenFile(filename,&file_desc));
  
  //I will write to the first block of the file to make it a HEAP_FILE.
  BF_Block* block;
  BF_Block_Init(&block);
  
  CALL_BF(BF_AllocateBlock(file_desc,block)); //Allocate memory for the first block I created.
  
  char* data;
  data=BF_Block_GetData(block); //Take the data of the first block.
  int integer=100;
  memcpy(data,&integer,sizeof(int)); //I choose a random intenger to show that this is a HEAP_FILE.
  BF_Block_SetDirty(block); //Because we want to change the data of the first block.
  if(BF_UnpinBlock(block)); //I unpin the block because I don't want it anymore.
  
  //Close the file.
  BF_CloseFile(file_desc);
  BF_Block_Destroy(&block);

  return HP_OK;
}

HP_ErrorCode HP_OpenFile(const char *fileName, int *fileDesc){
  //Open the file with name filename
  CALL_BF(BF_OpenFile(fileName,fileDesc));
  BF_Block* block;
  BF_Block_Init(&block);
  int integer;
  CALL_BF(BF_GetBlock(*fileDesc,0,block));
  char* data=BF_Block_GetData(block);
  memcpy(&integer,data,sizeof(int));
  if(integer!=100)
   return HP_ERROR;
  CALL_BF(BF_UnpinBlock(block));
  return HP_OK;
}

HP_ErrorCode HP_CloseFile(int fileDesc) {
  //Close the file with a specific identical number.
  CALL_BF(BF_CloseFile(fileDesc));
  return HP_OK;
}

HP_ErrorCode HP_InsertEntry(int fileDesc, Record record) {
  //Get the last block of the file.
  int blocks_num;
  BF_Block* block;
  int records_num;
  int freespace=0;
  int first_case=0;
  char* data;
  BF_Block_Init(&block);
  CALL_BF(BF_GetBlockCounter(fileDesc,&blocks_num));
  if((blocks_num-1)!=0)
  {
    first_case=1;
    CALL_BF(BF_GetBlock(fileDesc,blocks_num-1,block));

    //Take the number of records that the block has.
    data=BF_Block_GetData(block);
    memcpy(&records_num,data,sizeof(int));

    //Check if there is any space for a new record.
    
    freespace=BF_BLOCK_SIZE-sizeof(int)-records_num*(sizeof(Record)); 
    //If there is enough space for a new record put it there.
    BF_Block_SetDirty(block);
    if(freespace>=sizeof(Record))
    {
      memcpy(data+BF_BLOCK_SIZE-freespace,&record,sizeof(Record));
      records_num+=1;
      memcpy(data,&records_num,sizeof(int));
      CALL_BF(BF_UnpinBlock(block));
      return HP_OK;
    }
  }
  if((blocks_num-1)==0 || freespace<sizeof(Record)) //If there isn't any free space or there is only the first block in the HP_FILE.
  {
    records_num=0;
    if(first_case==1) //Unpin the previous block and allocate memory for another block(if I pinne it previously).
    {
      CALL_BF(BF_UnpinBlock(block));
    }
    CALL_BF(BF_AllocateBlock(fileDesc,block));
    data=BF_Block_GetData(block);
    records_num+=1;
    memcpy(data,&records_num,sizeof(int));
    BF_Block_SetDirty(block);
    memcpy(data+sizeof(int),&record,sizeof(Record));
    CALL_BF(BF_UnpinBlock(block));
  }    
  BF_Block_Destroy(&block);
  return HP_OK;
}

HP_ErrorCode HP_PrintAllEntries(int fileDesc, char *attrName, void* value) {
  //Get the number of the available blocks of the HEAP_FILE.
  int blocks_num;
  CALL_BF(BF_GetBlockCounter(fileDesc,&blocks_num));
  //Print the Records of each block.
  BF_Block* block;
  BF_Block_Init(&block);
  int records_num;
  char* data;
  Record record_print;
  for(int i=0;i<blocks_num;i++)
  {
    CALL_BF(BF_GetBlock(fileDesc,i,block));
    data=BF_Block_GetData(block);
    memcpy(&records_num,data,sizeof(int)); //Take the number of records.
    data+=sizeof(int); //Don't take the number of records.
    for(int j=0;j<records_num;j++)
    {
      memcpy(&record_print,data+j*sizeof(Record),sizeof(Record));
      if(value==NULL)
        printf("%d,\"%s\",\"%s\",\"%s\"\n",record_print.id,record_print.name,record_print.surname,record_print.city);
      else
      {
        if(strcmp(attrName,"name")==0)
        {
          if(strcmp(record_print.name,value)==0)
            printf("%d,\"%s\",\"%s\",\"%s\"\n",record_print.id,record_print.name,record_print.surname,record_print.city);
        }
        else if(strcmp(attrName,"surname")==0)
        {
          if(strcmp(value,record_print.surname)==0)
            printf("%d,\"%s\",\"%s\",\"%s\"\n",record_print.id,record_print.name,record_print.surname,record_print.city);
        }
        else if(strcmp(attrName,"city")==0)
        {
          if(strcmp(record_print.city,value)==0)
            printf("%d,\"%s\",\"%s\",\"%s\"\n",record_print.id,record_print.name,record_print.surname,record_print.city);
        }
        else
          HP_ERROR;                                      
      }
    }
    CALL_BF(BF_UnpinBlock(block));
  }
  BF_Block_Destroy(&block);

  return HP_OK;
}

HP_ErrorCode HP_GetEntry(int fileDesc, int rowId, Record *record) {
  //I will find in which block the row is.
  int records_in_block=BF_BLOCK_SIZE/sizeof(Record); //8.
  int block_num=1+((rowId-1)/records_in_block);  //ex: If we want the 8 record give me the 1st block, else if we want the 9 record give me the 2rd block etc. 
  BF_Block* block;
  BF_Block_Init(&block);

  //Get the record that we want from the block.
  int record_num=rowId%records_in_block;

  //printf("%d %d %d\n",records_in_block,block_num,record_num);
  CALL_BF(BF_GetBlock(fileDesc,block_num,block));
  char* data=BF_Block_GetData(block);
  if(record_num!=0)
    memcpy(record,data+sizeof(int)+(record_num-1)*sizeof(Record),sizeof(Record)); 
  else
    memcpy(record,data+sizeof(int)+(records_in_block-1)*sizeof(Record),sizeof(Record));

  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);
 
  return HP_OK;
}
