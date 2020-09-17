# Heap-File

The purpose of this project is to understand the internal operation of Base Systems
Data regarding the management at block level and the management at
records level. More specifically, in the context of this project I implement a set
functions that manage Heap Files.

The functions I implement are for record management. Their implementation will
done above the block management level, which is given as a library.

Some of the fuctions that I implement:
- HP_ErrorCode HP_Init()

- HP_ErrorCode HP_CreateFile(const char *fileName)

- HP_ErrorCode HP_OpenFile(const char *fileName, int *fileDesc)

- HP_ErrorCode HP_CloseFile(int fileDesc)

- HP_ErrorCode HP_InsertEntry (int fileDesc, Record record)

- HP_ErrorCode HP_PrintAllEntries(int fileDesc, char *fieldName, void *value)

- HP_ErrorCode HP_GetEntry(int fileDesc, int rowId, Record *record)

## Execution:
bash script.sh
