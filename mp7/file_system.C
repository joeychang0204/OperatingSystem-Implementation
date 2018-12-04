/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"


/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    Console::puts("In file system constructor.\n");
    file_num = 0;
    file_list = NULL;
    block_num = 0;
    //the block size is 512
    memset(buffer,0,512);
}

/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/

bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("mounting file system from disk\n");
    disk = _disk;
    file_num = block->size;
    for(int i = 0; i < file_num; i++){
        File* newFile= new File();
        newFile->file_size=block->size;
        newFile->file_id=block->id;
        //update the file list: move the old list to new
        if(file_list == NULL)
            file_list = newFile;
        else{
            File* new_file_list= (File*)new File[file_num + 1];
            for (int i=0;i<file_num;i++)//copy old list
                new_file_list[i]=file_list[i];
            new_file_list[file_num] = *newFile;//set new index to new block number
            file_num += 1;
            delete file_list;
            file_list = new_file_list;
        }
    }
    return true;
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) {
    Console::puts("formatting disk\n");
    
    memset(buffer,0,512);//set the buffer to 0, and write it into disk
    
    //20480 is the disk's block number
    for (int i = 0; i < 20480; i++)
        _disk->write(i,buffer);
    block->available = false; //set block to used
    block->size = 0;
    _disk->write(0,buffer); //initializes master block

	return true;
}

File * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file\n");
    for(int i = 0; i < file_num; i++){
        if (file_list[i].file_id==_file_id)
                return &file_list[i];
    }
    return NULL;
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file\n");
    File* newFile=(File*) new File();
    if (LookupFile(_file_id) != NULL)
        return false;

    newFile->file_id=_file_id;
    newFile->file_size=0;
    newFile->blocks=NULL;
    newFile->inode_block_num=AllocateBlock(0);//get any free block
    disk->read(newFile->inode_block_num,buffer);//load block in buffer
    block->available = false;//mark the block as being used
    block->size=0;//size 0
    block->id=_file_id;
    disk->write(newFile->inode_block_num,buffer);//write file inode to disk
    //update the file list: copy the old one into new
    if(file_list == NULL){
        file_list = newFile;
        file_num += 1;
    }
    else{
        File* new_file_list= (File*)new File[file_num + 1];
        for (int i=0;i<file_num;i++)//copy old list
            new_file_list[i]=file_list[i];
        new_file_list[file_num] = *newFile;//set new index to new block number
        file_num += 1;
        delete file_list;
        file_list = new_file_list;
    }
    return true;
}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("deleting file\n");
    File* new_file_list= (File*)new File[file_num-1];
    bool found=false;
    for (int i = 0;i < file_num; i++){
        if(file_list[i].file_id==_file_id){
            found=true;
            file_list[i].Rewrite();
            DeallocateBlock(file_list[i].inode_block_num);//deletes inode of file
        }
        if(found != true)
            new_file_list[i]=file_list[i];
        else if(i+1 < file_num)
            new_file_list[i]=file_list[i+1];
    }
    if(found)
        file_num -= 1;
    delete file_list; //delete old array
    file_list=new_file_list;//set pointer to new array
    if (file_num==0)
        file_list = NULL;
    return found;
}

unsigned int FileSystem::AllocateBlock(unsigned int _block_num){
        if (_block_num!=0){
            block->available = false;//mark the block as being used
            return _block_num;
        }
        else{
            disk->read(block_num,buffer);
            bool looped = false;
            while (block->available == false){
                if (block_num>(20480-1)){//look back at beginning
                    block_num=0;
                    looped = true;
                    if (looped){
                        Console::puts("ERROR: NO MORE FREE BLOCK");
                        return 0;
                    }
                }
                block_num += 1;
            }
            block->available = false;//sets block header to used
            return block_num;
        }
   }

   /*Allocates a block from free list, for use in file*/
void FileSystem::DeallocateBlock(unsigned int _block_num){
    block->available = true;//sets block header to free
}
